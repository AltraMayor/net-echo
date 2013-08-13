/*
 * ecli.c
 *
 * Authors:
 *	Cody Doucette		Boston University
 *	Michel Machado		Boston University
 *
 * This program implements an echo client that interacts with eserv.c, an echo
 * server.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "eutils.h"

static void stream_process_text(int s, char *input, int n_read)
{
	assert(write(s, input, n_read) == n_read);
	read_write(s, stdout, n_read);
}

/**
 * datagram_process_text(): Sends and receives a message from the echo server.
 */
static void datagram_process_text(int s, struct sockaddr *srv,
	socklen_t srv_len, char *input, int n_read)
{
	send_packet(s, input, n_read, srv, srv_len);
	recv_write(s, srv, srv_len, stdout, n_read);
}

int main(int argc, char *argv[])
{
	struct sockaddr *cli, *srv;
	int s, is_xia, is_stream, cli_len, srv_len, chunk_size;

	is_xia = check_cli_params(&is_stream, argc, argv);

	s = any_socket(is_xia, is_stream);
	assert(s >= 0);
	cli = get_cli_addr(is_xia, argc, argv, &cli_len);
	assert(cli);
	srv = get_srv_addr(is_xia, argc, argv, &srv_len);
	assert(srv);
	any_bind(is_xia, 0, s, cli, cli_len);

	if (is_stream)
		assert(!connect(s, srv, srv_len));

	chunk_size = is_stream ? 2048 : (is_xia ? 512 : MAX_UDP);
	while (1) {
		char input[512];
		int n_read = read_command(input, sizeof(input));
		if (n_read <= 0)
			break;

		if (is_file(input)) {
			if (is_stream)
				stream_process_file(s, input + 3, chunk_size,
					1, NULL);
			else
				datagram_process_file(s, srv, srv_len,
					input + 3, chunk_size, 1, NULL);
		} else {
			if (is_stream)
				stream_process_text(s, input, n_read);
			else
				datagram_process_text(s, srv, srv_len,
					input, n_read);
		}

		printf("\n\n");
	}

	free(srv);
	free(cli);
	assert(!close(s));
	return 0;
}
