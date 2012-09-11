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
#include <netinet/in.h>
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

int main(int argc, char *argv[])
{
	/* TODO add support to XIA! */
	struct sockaddr_in srv;
	int s, is_xia = 0;

	if (argc != 2) {
		printf("usage: ./server port\n");
		exit(1);
	}

	s = datagram_socket(is_xia);
	assert(s >= 0);

	memset(&srv, 0, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);
	datagram_bind(0, 1, s, (const struct sockaddr *)&srv, sizeof(srv));

	while (1) {
		int len = recvfrom(s, NULL, 0, MSG_PEEK|MSG_TRUNC, NULL, NULL);
		assert(len >= 0);
		echo(s, len);
	}
	assert(!close(s));

	return 0;
}
