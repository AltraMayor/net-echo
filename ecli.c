/*
 * ecli.c
 *
 * Cody Doucette
 * Boston University
 *
 * This program implements an echo client that interacts with eserv.c, an echo
 * server.
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "eutils.h"

/**
 * process_text(): Sends and receives a message from the echo server.
 */
static void process_text(int s, struct sockaddr_in *srv, char *input, int read)
{
	send_packet(s, input, read, srv);
	recv_write(s, stdout, read, srv);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	int s, n_read;

	char *input = NULL;
	size_t line_size = 0;

	if (argc != 3) {
		printf("usage: ./client ip port\n");
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s >= 0);

	memset((char *)&srv, 0, sizeof(struct sockaddr_in));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[2]));
	assert(inet_aton(argv[1], &srv.sin_addr));

	while (1) {
		n_read = getline(&input, &line_size, stdin);
		assert(n_read >= 0);

		if (n_read == 1)		/* Empty message. */
			continue;

		strtok(input, "\n");

		if (is_file(input))
			process_file(s, &srv, input + 3, MAX_UDP);
		else
			process_text(s, &srv, input, n_read - 1);

		puts("\n");
	}

	assert(!close(s));
	free(input);
	return 0;
}
