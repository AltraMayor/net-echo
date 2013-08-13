/*
 * eserv.c
 *
 * Authors:
 *	Cody Doucette		Boston University
 *	Michel Machado		Boston University
 *
 * This program implements an echo server that interacts with echo clients,
 * such as eclicork.c and ecli.c.
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "eutils.h"

static void stream_loop(int sock)
{
	int conn;

	assert(!listen(sock, 5));

	while ((conn = accept(sock, NULL, NULL)) >= 0) {
		printf("---- Connect()\n");
		copy_data(conn, conn);
		printf("---- Close()\n");
		close(conn);
	}

	assert(conn >= 0);
}

/**
 * echo(): Receive a message of msg_len size, allocate memory using alloca(),
 * and echo the message back to the source.
 */
static void echo(int s, int msg_len)
{
	struct tmp_sockaddr_storage cli_stack;
	struct sockaddr *cli = (struct sockaddr *)&cli_stack;
	unsigned int len = sizeof(cli_stack);
	char *msg = alloca(msg_len);
	int read = recvfrom(s, msg, msg_len, 0, cli, &len);
	assert(read == msg_len);
	send_packet(s, msg, msg_len, cli, len);
}

static void datagram_loop(int sock)
{
	while (1) {
		int len = recvfrom(sock, NULL, 0, MSG_PEEK|MSG_TRUNC,
			NULL, NULL);
		assert(len >= 0);
		echo(sock, len);
	}
}

static int check_srv_params(int *pis_stream, int argc, char * const argv[])
{
	if (argc != 4)
		goto failure;

	if (!strcmp(argv[1], "datagram"))
		*pis_stream = 0;
	else if (!strcmp(argv[1], "stream"))
		*pis_stream = 1;
	else
		goto failure;

	if (!strcmp(argv[2], "xip"))
		return 1;

	if (!strcmp(argv[2], "ip"))
		return 0;

failure:
	printf("usage:\t%s <'datagram' | 'stream'> 'ip' port\n", argv[0]);
	printf(      "\t%s <'datagram' | 'stream'> 'xip' srv_addr_file\n",
		argv[0]);
	exit(1);
}

int main(int argc, char *argv[])
{
	struct sockaddr *srv;
	int s, is_xia, is_stream, srv_len;

	is_xia = check_srv_params(&is_stream, argc, argv);

	s = any_socket(is_xia, is_stream);
	if (s < 0) {
		int orig_errno = errno;
		fprintf(stderr, "Cannot create %s %s socket: %s\n",
			is_xia ? "xia" : "ip",
			is_stream ? "stream" : "datagram",
			strerror(orig_errno));
		return 1;
	}

	srv = is_xia ?
		__get_addr(is_xia, argv[3], NULL, &srv_len) :
		__get_addr(is_xia, NULL, argv[3], &srv_len) ;
	any_bind(is_xia, 1, s, srv, srv_len);

	if (is_stream)
		stream_loop(s);
	else
		datagram_loop(s);

	free(srv);
	assert(!close(s));
	return 0;
}
