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
#include "eutils.h"

#define FILE_APPENDIX "_echo"

/**
 * check_cli_params(): Ensure that the correct number of arguments have been
 * given.
 */
void check_cli_params(int argc, char * const argv[])
{
	if (argc != 3) {
		printf("usage: %s ip port\n", argv[0]);
		exit(1);
	}
}

/**
 * send_packet(): Send a packet via the given socket.
 */
void send_packet(int s, const char *buf, int n, const struct sockaddr_in *dst)
{
	assert(sendto(s, buf, n, 0, (const struct sockaddr *)dst,
		sizeof(*dst)) >= 0);
}

/**
 * recv_write(): Receive a packet via the given socket and write to the
 * given file.
 */
void recv_write(int s, const struct sockaddr_in *expected_src, FILE *copy,
	int n_sent)
{
	char *out = alloca(n_sent);
	struct sockaddr_in src;
	unsigned int len = sizeof(src);
	int n_read = recvfrom(s, out, n_sent, 0, (struct sockaddr *)&src, &len);	assert(n_read >= 0);

	/* Make sure that we're reading from the server. */
	assert(len == sizeof(src));
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
 * __process_file(): Is called after a EOF is found in a file to write
 * the output to a given file.
 */
static void __process_file(int s, const struct sockaddr_in *srv,
	struct fc_info *fci, int max)
{
	int bytes_sent = 0;

	while (fci->nbytes > 0) {
		int bytes_to_send = fci->nbytes > max ? max : fci->nbytes;

		send_packet(s, fci->text + bytes_sent, bytes_to_send, srv);
		fci->recv_fn(s, srv, fci->copy, bytes_to_send);

		fci->nbytes -= bytes_to_send;
		bytes_sent += bytes_to_send;
	}
}

/**
 * process_file(): Set up the output file, read in the file into a char buffer,
 * and call __process_file() to do a sequence of sends and receives.
 */
void process_file(int s, const struct sockaddr_in *srv, const char *orig_name,
	int chunk_size, struct fc_info *fci)
{
	size_t text_len = 0;

	FILE *orig = fopen(orig_name, "rb");
	assert(orig);

	fci->copy = fopen_copy(orig_name, "wb");
	assert(fci->copy);

	fci->nbytes = getdelim(&fci->text, &text_len, EOF, orig);
	assert(fci->nbytes > 0);
	__process_file(s, srv, fci, chunk_size);

	free(fci->text);
	assert(!fclose(fci->copy));
	assert(!fclose(orig));
}
