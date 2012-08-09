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

#include "eutils.h"

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	struct sockaddr_in cli;

	unsigned int len = sizeof(struct sockaddr_in);
	int s;
	int read;
	char buf[MAX_UDP];

	if (argc != 2) {
		printf("usage: ./server port\n");
		exit(1);
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		perrorq("socket");

	memset((char *)&srv, 0, len);
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[1]));
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, (struct sockaddr *)&srv, len) == -1)
		perrorq("bind");

	while (1) {

		unsigned int len = sizeof(struct sockaddr_in);

		read = recvfrom(s, buf, MAX_UDP, MSG_PEEK, 
						(struct sockaddr *)&cli, &len);
		printf("Bytes peeked: %d\n", read);

		read = recv_packet(s, buf, MAX_UDP, &cli);

		printf("Bytes received: %d\n", read);

		send_packet(s, buf, read, &cli);
	}

	if (close(s) == -1)
		print_error("close");

	return 0;
}
