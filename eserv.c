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
static void echo(int s, int msg_len)
{
	struct sockaddr_in cli;
	unsigned int len = sizeof(cli);
	char *msg = alloca(msg_len);
	int read = recvfrom(s, msg, msg_len, 0, (struct sockaddr *)&cli, &len);
	assert(read >= 0);
	assert(len == sizeof(cli));
	send_packet(s, msg, msg_len, &cli);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	int s;

	if (argc != 2) {
		printf("usage: ./server port\n");
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s >= 0);

	memset(&srv, 0, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

	assert(!bind(s, (const struct sockaddr *)&srv, sizeof(srv)));

	while (1) {
		int len = recvfrom(s, NULL, 0, MSG_PEEK, NULL, NULL);
		assert(len >= 0);
		echo(s, len);
	}
	assert(!close(s));

	return 0;
}
