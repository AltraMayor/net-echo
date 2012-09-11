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
/* TODO
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
*/
#include <arpa/inet.h>
#include "eutils.h"

/**
 * process_text(): Sends and receives a message from the echo server.
 */
static void process_text(int s, struct sockaddr *srv, socklen_t srv_len,
	char *input, int read)
{
	send_packet(s, input, read, srv, srv_len);
	recv_write(s, srv, srv_len, stdout, read);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	int s, n_read, is_xia;

	char *input = NULL;
	size_t line_size = 0;

	is_xia = check_cli_params(argc, argv);

	s = datagram_socket(is_xia);
	assert(s >= 0);

	memset(&srv, 0, sizeof(srv));
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
			process_file(s, (struct sockaddr *)&srv, sizeof(srv),
				input + 3, MAX_UDP, recv_write);
		else
			process_text(s, (struct sockaddr *)&srv, sizeof(srv),
				input, n_read - 1);

		puts("\n");
	}

	free(input);
	assert(!close(s));
	return 0;
}
