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
 * check_validity(): Check the validity of the program; ensure that the
 * correct number of arguments have been given and that the CORK_SIZE is
 * not greater than the maximum size of a UDP packet.
 */
static inline void check_validity(int argc)
{
	if (argc != 3) {
		printf("usage: ./corkclient ip port\n");
		exit(1);
	}

	if (CORK_SIZE > MAX_UDP) {
		printf("cork size > maximum UDP packet size\n");
		exit(1);
	}
}

/**
 * empty_cork(): Empties a corked socket by uncorking, receiving the resulting
 * packet, and writing it to a file before corking the socket again.
 */
static void empty_cork(int s, const struct sockaddr_in *srv)
{
	uncork(s);
	recv_write(s, stdout, bytes_corked, srv);
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

	if (n > bytes_avail) {
		send_packet(s, in, bytes_avail, srv);
		bytes_corked += bytes_avail;
		empty_cork(s, srv);
		n -= bytes_avail;
		process_text(s, srv, in + bytes_avail, n);
	} else {
		send_packet(s, in, n, srv);
		bytes_corked += n;

		if (bytes_corked == CORK_SIZE)
			empty_cork(s, srv);
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in srv;
	int s, n_read;

	char *input = NULL;
	size_t line_size = 0;

	check_validity(argc);

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
			if (bytes_corked)
				empty_cork(s, &srv);
			uncork(s);
			process_file(s, &srv, input + 3, CORK_SIZE);
			cork(s);
		} else {
			process_text(s, &srv, input, n_read - 1);
		}

		puts("\n");
	}

	printf("While-loop finished\n");

	free(input);
	assert(!close(s));
	return 0;
}
