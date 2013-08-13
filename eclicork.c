/*
 * eclicork.c
 *
 * Cody Doucette
 * Boston University
 *
 * This program implements an echo client that uses a corked socket to interact
 * with eserv.c, an echo server.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include "eutils.h"

#define CORK_SIZE 64
#define CORK_TIMES (512/CORK_SIZE)

static int bytes_corked; /* Number of corked bytes in current packet. */
int is_xia;

/**
 * cork(): Cork socket to CORK_SIZE bytes.
 */
static inline void cork(int s)
{
	int rc, one = 1;
	rc = is_xia ?
		setsockopt(s, get_xdp_type(), XDP_CORK, &one, sizeof(int)):
		setsockopt(s, IPPROTO_UDP, UDP_CORK, &one, sizeof(int));
	assert(!rc);
}

/**
 * uncork(): Uncork socket.
 */
static inline void uncork(int s)
{
	int rc, zero = 0;
	rc = is_xia ?
		setsockopt(s, get_xdp_type(), XDP_CORK, &zero, sizeof(int)):
		setsockopt(s, IPPROTO_UDP, UDP_CORK, &zero, sizeof(int));
	assert(!rc);
}

/**
 * empty_cork(): Add the number of recently corked bytes, if any, and empty a
 * corked socket.
 */
static void empty_cork(int s, const struct sockaddr *srv, socklen_t srv_len,
	FILE *f, int n_sent)
{
	bytes_corked += n_sent;
	uncork(s);
	recv_write(s, srv, srv_len, f, bytes_corked);
	fprintf(f, "\n");
	cork(s);
	bytes_corked = 0;
}

static void mark(int s)
{
	uncork(s);
	cork(s);
}

/**
 * process_text(): Depending on whether a packet will exceed CORK_SIZE bytes,
 * this function either splits up the message into separate packets or fits the
 * message into the current packet.
 */
static void process_text(int s, const struct sockaddr *srv, socklen_t srv_len,
	char *in, int n)
{
	int bytes_avail = CORK_SIZE - bytes_corked;
	int bytes_this_time = n > bytes_avail ? bytes_avail : n;

	send_packet(s, in, bytes_this_time, srv, srv_len);
	bytes_corked += bytes_this_time;

	assert(bytes_corked <= CORK_SIZE);
	if (bytes_corked == CORK_SIZE)
		empty_cork(s, srv, srv_len, stdout, 0);

	n -= bytes_this_time;
	assert(n >= 0);
	if (n)
		process_text(s, srv, srv_len, in + bytes_this_time, n);
}

int main(int argc, char *argv[])
{
	struct sockaddr *cli, *srv;
	int s, cli_len, srv_len;

	is_xia = check_cli_params(argc, argv);

	s = any_socket(is_xia, 0);
	assert(s >= 0);
	cli = get_cli_addr(is_xia, argc, argv, &cli_len);
	assert(cli);
	srv = get_srv_addr(is_xia, argc, argv, &srv_len);
	assert(srv);
	any_bind(is_xia, 0, s, cli, cli_len);

	cork(s);
	while (1) {
		char input[512];
		int n_read = read_command(input, sizeof(input));
		if (n_read <= 0)
			break;

		if (is_file(input)) {
			if (bytes_corked)
				empty_cork(s, srv, srv_len, stdout, 0);
			process_file(s, srv, srv_len, input + 3,
				CORK_SIZE, CORK_TIMES, mark);
			printf("\n");
		} else {
			process_text(s, srv, srv_len, input, n_read);
		}

		printf("\n");
	}

	free(srv);
	free(cli);
	assert(!close(s));
	return 0;
}
