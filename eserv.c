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
#include "eutils.h"

/**
 * echo(): Receive a message of msg_len size, allocate memory using alloca(),
 * and echo the message back to the source.
 */
static void echo(int s, int msg_len, const struct sockaddr_in *srv)
{
	char *msg = alloca(msg_len);
	recv_packet(s, msg, msg_len, srv);
	send_packet(s, msg, msg_len, srv);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	struct sockaddr_in cli;
	char buf[0];
	int s, len;

	if (argc != 2) {
		printf("usage: ./server port\n");
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s >= 0);

	memset((char *)&srv, 0, sizeof(struct sockaddr_in));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

	assert(!bind(s, (struct sockaddr *)&srv, sizeof(struct sockaddr_in)));

	while (1) {
		len = recvfrom(s, buf, MAX_UDP, MSG_PEEK, NULL, 0);
		assert(len >= 0);
		echo(s, len, &cli);
	}
	assert(!close(s));

	return 0;
}
