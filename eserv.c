/*
 * eserv.c
 *
 * Cody Doucette
 * Boston University
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
#include "eutils.h"

/**
 * echo(): Receive a message of msg_len size, allocate memory using alloca(),
 * and echo the message back to the source.
 */
static void echo(int s, int msg_len)
{
	struct tmp_sockaddr_storage cli_stack;
	struct sockaddr *cli = (struct sockaddr *)&cli_stack;
	unsigned int len = sizeof(cli);
	char *msg = alloca(msg_len);
	int read = recvfrom(s, msg, msg_len, 0, cli, &len);
	assert(read >= 0);
	send_packet(s, msg, msg_len, cli, len);
}

static int check_srv_params(int argc, char * const argv[])
{
	if (argc != 3)
		goto failure;

	if (!strcmp(argv[1], "xip"))
		return 1;

	if (!strcmp(argv[1], "ip"))
		return 0;

failure:
	printf("usage:\t%s 'ip' port\n", argv[0]);
	printf(      "\t%s 'xip' srv_addr_file\n", argv[0]);
	exit(1);
}

int main(int argc, char *argv[])
{
	struct sockaddr *srv;
	int s, is_xia, srv_len;

	is_xia = check_srv_params(argc, argv);

	s = datagram_socket(is_xia);
	if (s < 0) {
		int orig_errno = errno;
		fprintf(stderr, "Cannot create %s socket: %s\n",
			is_xia ? "xia" : "ip", strerror(orig_errno));
		return 1;
	}

	srv = is_xia ?
		__get_addr(is_xia, argv[2], NULL, &srv_len) :
		__get_addr(is_xia, NULL, argv[2], &srv_len) ;
	datagram_bind(0, 1, s, srv, srv_len);

	while (1) {
		int len = recvfrom(s, NULL, 0, MSG_PEEK|MSG_TRUNC, NULL, NULL);
		assert(len >= 0);
		echo(s, len);
	}

	free(srv);
	assert(!close(s));
	return 0;
}
