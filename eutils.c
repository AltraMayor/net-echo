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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "xia_all.h"
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

	/* Don't simplify this test, argc may change for each case. */
	if ((is_xia && argc == 4) || (!is_xia && argc == 4))
		return is_xia;

failure:
	printf("usage:\t%s 'ip' srvip_addr port\n", argv[0]);
	printf(      "\t%s 'xip' cli_addr_file srv_addr_file\n", argv[0]);
	exit(1);
}

int datagram_socket(int is_xia)
{
	if (is_xia)
		return socket(AF_XIA,  SOCK_DGRAM, XIDTYPE_XDP);
	else
		return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

static void set_sockaddr_in(struct sockaddr_in *in, char *str_addr, int port)
{
	in->sin_family = AF_INET;
	in->sin_port = htons(port);
	if (str_addr)
		assert(inet_aton(str_addr, &in->sin_addr));
	else
		in->sin_addr.s_addr = htonl(INADDR_ANY);
}

/* XXX This function was copied from xiaconf/xip/xiphid.c.
 * It should go to a library!
 */
static int parse_and_validate_addr(char *str, struct xia_addr *addr)
{
	int invalid_flag;
	int rc;

	rc = xia_pton(str, INT_MAX, addr, 0, &invalid_flag);
	if (rc < 0) {
		fprintf(stderr, "Syntax error: invalid address: [[%s]]\n", str);
		return rc;
	}
	rc = xia_test_addr(addr);
	if (rc < 0) {
		char buf[XIA_MAX_STRADDR_SIZE];
		assert(xia_ntop(addr, buf, XIA_MAX_STRADDR_SIZE, 1) >= 0);
		fprintf(stderr, "Invalid address (%i): [[%s]] "
			"as seen by xia_xidtop: [[%s]]\n", -rc, str, buf);
		return rc;
	}
	if (invalid_flag) {
		fprintf(stderr, "Although valid, address has invalid flag: "
			"[[%s]]\n", str);
		return -1;
	}
	return 0;
}

static int set_sockaddr_xia(struct sockaddr_xia *xia, const char *filename)
{
#define BUFSIZE (4 * 1024)
	static int ppal_map_loaded = 0;
	FILE *f;
	char buf[BUFSIZE];
	int len;
	
	if (!ppal_map_loaded) {
		ppal_map_loaded = 1;
		assert(!init_ppal_map());
	}

	/* Read address. */
	f = fopen(filename, "r");
	if (!f) {
		perror(__func__);
		return errno;
	}
	len = fread(buf, 1, BUFSIZE, f);
	assert(len < BUFSIZE);
	fclose(f);
	buf[len] = '\0';

	xia->sxia_family = AF_XIA;
	return parse_and_validate_addr(buf, &xia->sxia_addr);
}

struct sockaddr *__get_addr(int is_xia, char *str1, char *str2, int *plen)
{
	struct tmp_sockaddr_storage *skaddr;

	skaddr = malloc(sizeof(*skaddr));
	assert(skaddr);
	memset(skaddr, 0, sizeof(*skaddr));

	if (is_xia) {
		struct sockaddr_xia *xia = (struct sockaddr_xia *)skaddr;
		/* XXX It should be a BUILD_BUG_ON(). */
		assert(sizeof(*skaddr) >= sizeof(*xia));
		assert(!set_sockaddr_xia(xia, str1));
		*plen = sizeof(*xia);
	} else {
		struct sockaddr_in *in = (struct sockaddr_in *)skaddr;
		/* XXX It should be a BUILD_BUG_ON(). */
		assert(sizeof(*skaddr) >= sizeof(*in));
		set_sockaddr_in(in, str1, atoi(str2));
		*plen = sizeof(*in);
	}

	return (struct sockaddr *)skaddr;
}

struct sockaddr *get_cli_addr(int is_xia, int argc, char * const argv[],
	int *plen)
{
	if (is_xia)
		return __get_addr(is_xia, argv[2], NULL, plen);
	else
		return __get_addr(is_xia, NULL, "0", plen);
}

struct sockaddr *get_srv_addr(int is_xia, int argc, char * const argv[],
	int *plen)
{
	if (is_xia)
		return __get_addr(is_xia, argv[3], NULL, plen);
	else
		return __get_addr(is_xia, argv[2], argv[3], plen);
}

void datagram_bind(int is_xia, int force, int s, const struct sockaddr *addr,
	int addr_len)
{
	if (is_xia || force) {
		/* XIA requires explicit binding, whereas TCP/IP doesn't,
		 * so only bind if @force is true.
		 */
		assert(!bind(s, addr, addr_len));
	}
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
	struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
	fd_set readfds;
	struct tmp_sockaddr_storage src;
	char *out;
	unsigned int len;
	int rc, n_read;

	/* Is there anything to read? */
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	rc = select(s + 1, &readfds, NULL, NULL, &timeout);
	assert(rc >= 0);
	if (!rc) {
		/* A packet was dropped. */
		fprintf(stderr, ".");
		return;
	}

	/* Read. */
	out = alloca(n_sent);
	len = sizeof(src);
	n_read = recvfrom(s, out, n_sent, 0, (struct sockaddr *)&src, &len);
	assert(n_read >= 0);

	/* Make sure that we're reading from the server. */
	assert(len == exp_src_len);
	assert(!memcmp(&src, expected_src, len));

	/* Write. */
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
	const char *orig_name, int chunk_size, int times, pff_mark_t f)
{
	FILE *orig, *copy;
	char *buf;
	int count, bytes_sent;

	orig = fopen(orig_name, "rb");
	assert(orig);
	copy = fopen_copy(orig_name, "wb");
	assert(copy);

	buf = alloca(chunk_size);
	count = bytes_sent = 0;
	do {
		size_t bytes_read = fread(buf, 1, chunk_size, orig);
		assert(!ferror(orig));
		if (bytes_read > 0) {
			send_packet(s, buf, bytes_read, srv, srv_len);
			count++;
			bytes_sent += bytes_read;
		}
		if (count == times) {
			if (f)
				f(s);
			recv_write(s, srv, srv_len, copy, bytes_sent);
			count = bytes_sent = 0;
		}
	} while (!feof(orig));

	if (count) {
		if (f)
			f(s);
		recv_write(s, srv, srv_len, copy, bytes_sent);
	}

	assert(!fclose(copy));
	assert(!fclose(orig));
}
