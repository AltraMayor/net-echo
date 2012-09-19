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
	struct sockaddr *cli, *srv;
	int s, n_read, is_xia, cli_len, srv_len, chunk_size;

	char *input = NULL;
	size_t line_size = 0;

	is_xia = check_cli_params(argc, argv);

	s = datagram_socket(is_xia);
	assert(s >= 0);
	cli = get_cli_addr(is_xia, argc, argv, &cli_len);
	assert(cli);
	srv = get_srv_addr(is_xia, argc, argv, &srv_len);
	assert(srv);
	datagram_bind(is_xia, 0, s, cli, cli_len);

	chunk_size = is_xia ? 512 : MAX_UDP;
	while (1) {
		n_read = getline(&input, &line_size, stdin);
		assert(n_read >= 0);

		if (n_read == 1)		/* Empty message. */
			continue;

		strtok(input, "\n");

		if (is_file(input))
			process_file(s, srv, srv_len, input + 3,
				chunk_size, 1, NULL);
		else
			process_text(s, srv, srv_len, input, n_read - 1);

		printf("\n\n");
	}

	free(input);
	free(srv);
	free(cli);
	assert(!close(s));
	return 0;
}
