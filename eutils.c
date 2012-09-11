/*
 * eutils.c
 *
 * Cody Doucette
 * Boston University
 *
 * This file contains utility definitions for the echo clients and server.
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/socket.h>
#include "eutils.h"

#define FILE_APPENDIX "_echo"

/**
 * check_cli_params(): Ensure that the correct number of arguments have been
 * given.
 */
int check_cli_params(int argc, char * const argv[])
{
	int is_xia;

	if (argc > 1) {
		if (!strcmp(argv[1], "xip"))
			is_xia = 1;
		else if (!strcmp(argv[1], "ip"))
			is_xia = 0;
		else
			goto failure;
	}

	if ((is_xia && argc == 3) || (!is_xia && argc == 4))
		return is_xia;

failure:
	printf("usage:\t%s 'ip' srvip_addr port\n", argv[0]);
	printf(      "\t%s 'xia' filename\n", argv[0]);
	exit(1);
}

int datagram_socket(int is_xia)
{
	if (is_xia)
		/* TODO */
		return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

/**
 * send_packet(): Send a packet via the given socket.
 */
void send_packet(int s, const char *buf, int n, const struct sockaddr *dst,
	socklen_t dst_len)
{
	assert(sendto(s, buf, n, 0, dst, dst_len) >= 0);
}

/**
 * recv_write(): Receive a packet via the given socket and write to the
 * given file.
 */
void recv_write(int s, const struct sockaddr *expected_src,
	socklen_t exp_src_len, FILE *copy, int n_sent)
{
	char *out = alloca(n_sent);
	struct __kernel_sockaddr_storage src;
	unsigned int len = sizeof(src);
	int n_read = recvfrom(s, out, n_sent, 0, (struct sockaddr *)&src, &len);
	assert(n_read >= 0);

	/* Make sure that we're reading from the server. */
	assert(len == exp_src_len);
	assert(!memcmp(&src, expected_src, len));

	assert(fwrite(out, sizeof(char), n_read, copy) >= 0);
}

/**
 * fopen_copy(): Create and open a file to for writing output.
 */
static FILE *fopen_copy(const char *orig_name, const char *mode)
{
	int name_len = strlen(orig_name) + strlen(FILE_APPENDIX) + 1;
	char *copy_name = alloca(name_len);

	assert(snprintf(copy_name, name_len, "%s%s", orig_name, FILE_APPENDIX)
		== name_len - 1);

	return fopen(copy_name, mode);
}

/**
 * process_file(): Set up the output file, read in the file into a char buffer,
 * and call __process_file() to do a sequence of sends and receives.
 */
void process_file(int s, const struct sockaddr *srv, socklen_t srv_len,
	const char *orig_name, int chunk_size, pff_recvf_t f)
{
	FILE *orig, *copy;
	char *buf;

	orig = fopen(orig_name, "rb");
	assert(orig);
	copy = fopen_copy(orig_name, "wb");
	assert(copy);

	buf = alloca(chunk_size);
	do {
		size_t bytes_read = fread(buf, 1, chunk_size, orig);
		assert(!ferror(orig));
		if (bytes_read > 0) {
			send_packet(s, buf, bytes_read, srv, srv_len);
			f(s, srv, srv_len, copy, bytes_read);
		}
	} while (!feof(orig));

	assert(!fclose(copy));
	assert(!fclose(orig));
}
