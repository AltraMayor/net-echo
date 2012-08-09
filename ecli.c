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

#include "eutils.h"

/**
 * process_file(): Sets up the input and output files and uses a loop to
 * process the desired file.
 */
static void process_file(int s, struct sockaddr_in *srv, char *in)
{
	FILE *orig, *copy;
	int n_read, n_rcvd;

	int name_size = strlen(in) + strlen(FILE_APPEND) + 1;
	char *filename = malloc(name_size);
	char *line = NULL;
	size_t line_size = 0;

	setup_output_file(in, filename, name_size);

	if ((orig = fopen(in, "rb")) == NULL)
		perrorq("input fopen");

	if ((copy = fopen(filename, "wb")) == NULL)
		perrorq("output fopen");

	while ((n_read = getdelim(&line, &line_size, EOF, orig)) > 0)
		__process_file(s, srv, line, n_read, copy, MAX_UDP);

	if (fclose(copy) != 0)
		perrorq("output fclose");

	if (fclose(orig) != 0)
		perrorq("input fclose");

	free(line);
	free(filename);
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
	int s;
	int n_read;

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

		/* Empty message. */
		if (n_read == 1)
			continue;

		strtok(input, "\n");

		/* Quit client. */
		if (strcmp(input, "-q") == 0)
			break;

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
