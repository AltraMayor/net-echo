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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "eutils.h"

/**
 * process_file(): Sets up the input and output files and uses a loop to
 * process the desired file.
 */
static void process_file(int s, const struct sockaddr_in *srv, char *orig_name)
{
	FILE *orig, *copy;
	int n_read, n_rcvd;

	char *line = NULL;
	size_t line_len = 0;

	int name_len = strlen(orig_name) + strlen(FILE_APPEND) + 1;
	char *copy_name = malloc(name_len);
	setup_output_file(orig_name, copy_name, name_len);

	if ((orig = fopen(orig_name, "rb")) == NULL)
		perrorq("input fopen");

	if ((copy = fopen(copy_name, "wb")) == NULL)
		perrorq("output fopen");

	if ((n_read = getdelim(&line, &line_len, EOF, orig)) > 0)
		__process_file(s, srv, line, n_read, copy, n_read);

	if (fclose(copy) != 0)
		perrorq("output fclose");

	if (fclose(orig) != 0)
		perrorq("input fclose");

	free(line);
	free(copy_name);
}

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

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		perrorq("socket");

	memset((char *)&srv, 0, sizeof(struct sockaddr_in));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(atoi(argv[2]));

	if (inet_aton(argv[1], &srv.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	while (1) {
		if ((n_read = getline(&input, &line_size, stdin)) < 0)
			perrorq("getline");

		if (n_read == 1)		/* Empty message. */
			continue;

		strtok(input, "\n");

		if (is_file(input))
			process_file(s, &srv, input + 3);
		else
			process_text(s, &srv, input, n_read - 1);

		puts("\n");
	}

	if (close(s) == -1)
		perrorq("close");

	free(input);

	return 0;
}
