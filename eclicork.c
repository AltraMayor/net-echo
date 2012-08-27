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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include "eutils.h"

#define CORK_SIZE 64

static int bytes_corked; /* Number of corked bytes in current packet. */

/**
 * cork(): Cork socket to CORK_SIZE bytes.
 */
static inline void cork(int s)
{
	int one = 1;
	assert(!setsockopt(s, IPPROTO_UDP, UDP_CORK, &one, sizeof(int)));
}

/**
 * uncork(): Uncork socket.
 */
static inline void uncork(int s)
{
	int zero = 0;
	assert(!setsockopt(s, IPPROTO_UDP, UDP_CORK, &zero, sizeof(int)));
}

/**
 * empty_cork(): Add the number of recently corked bytes, if any, and empty a
 * corked socket.
 */
static void empty_cork(int s, const struct sockaddr_in *srv, FILE *f,
	int n_sent)
{
	bytes_corked += n_sent;
	uncork(s);
	recv_write(s, srv, f, bytes_corked);
	cork(s);
	bytes_corked = 0;
}

/**
 * process_text(): Depending on whether a packet will exceed CORK_SIZE bytes,
 * this function either splits up the message into separate packets or fits the
 * message into the current packet.
 */
static void process_text(int s, const struct sockaddr_in *srv, char *in, int n)
{
	int bytes_avail = CORK_SIZE - bytes_corked;
	int bytes_this_time = n > bytes_avail ? bytes_avail : n;

	send_packet(s, in, bytes_this_time, srv);
	bytes_corked += bytes_this_time;

	if (bytes_corked == CORK_SIZE)
		empty_cork(s, srv, stdout, 0);

	n -= bytes_this_time;
	assert(n >= 0);
	if (n)
		process_text(s, srv, in + bytes_this_time, n);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	int s, n_read;

	char *input = NULL;
	size_t line_size = 0;

	check_cli_params(argc, argv);

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

		cork(s);

		if (is_file(input)) {
			struct fc_info fci;
			fci.recv_fn = empty_cork;

			if (bytes_corked)
				empty_cork(s, &srv, stdout, 0);

			process_file(s, &srv, input + 3, CORK_SIZE, &fci);
		} else {
			process_text(s, &srv, input, n_read - 1);
		}

		puts("\n");
	}

	free(input);
	assert(!close(s));
	return 0;
}
