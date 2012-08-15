/*
 * eutils.c
 *
 * Cody Doucette
 * Boston University
 *
 * This file contains utility definitions for the echo clients and server.
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "eutils.h"

#define FILE_APPEND "_echo"

/**
 * check_validity(): Check the validity of the program; ensure that the
 * correct number of arguments have been given.
 */
void check_cli_params(int argc, char * const argv[])
{
	if (argc != 3) {
		printf("usage: %s ip port\n", argv[0]);
		exit(1);
	}
}

/**
 * send_packet(): Send a packet via the given socket.
 */
void send_packet(int s, const char *buf, int n, const struct sockaddr_in *srv)
{
	assert(sendto(s, buf, n, 0, (struct sockaddr *)srv,
		sizeof(struct sockaddr_in)) >= 0);
}

/**
 * recv_packet(): Receive a packet via the given socket.
 */
int recv_packet(int s, char *buf, int n, const struct sockaddr_in *srv)
{
	int read;
	unsigned int len = sizeof(struct sockaddr_in);

	read = recvfrom(s, buf, n, 0, (struct sockaddr *)srv, &len);
	assert(read >= 0);
	return read;
}

/**
 * recv_write(): Receive a packet via the given socket and write to the
 * given file.
 */
void recv_write(int s, FILE *copy, int n, const struct sockaddr_in *srv)
{
        int n_read;
        char *out = alloca(n);
        unsigned int len = sizeof(struct sockaddr_in);

        n_read = recvfrom(s, out, n, 0, (struct sockaddr *)srv, &len);
	assert(n_read >= 0);

        assert(fwrite(out, sizeof(char), n_read, copy) >= 0);
}

static FILE *fopen_copy(const char *orig_name, const char *mode)
{
	int name_len = strlen(orig_name) + strlen(FILE_APPEND) + 1;
	char *copy_name = alloca(name_len);

	assert(snprintf(copy_name, name_len, "%s%s", orig_name, FILE_APPEND)
		== name_len - 1);

	return fopen(copy_name, mode);
}

/**
 * __process_file(): Is called after each EOF is found in a file to write
 * the output to a given file.
 */
static void __process_file(int s, const struct sockaddr_in *srv,
	const char *line, int n_read, FILE *copy, int max)
{
        int bytes_sent = 0;

        while (n_read > 0) {
		int bytes_to_send = n_read > max ? max : n_read;

                send_packet(s, line + bytes_sent, bytes_to_send, srv);
                recv_write(s, copy, bytes_to_send, srv);

                n_read -= bytes_to_send;
                bytes_sent += bytes_to_send;
        }
}

/**
 * process_file(): Sets up the input and output files and uses a loop to
 * process the desired file.
 */
void process_file(int s, const struct sockaddr_in *srv, const char *orig_name,
	int chunk_size)
{
	FILE *orig, *copy;
	int n_read;

	char *line = NULL;
	size_t line_len = 0;

	orig = fopen(orig_name, "rb");
	assert(orig);

	copy = fopen_copy(orig_name, "wb");
	assert(copy);

	if ((n_read = getdelim(&line, &line_len, EOF, orig)) > 0)
		__process_file(s, srv, line, n_read, copy, chunk_size);

	free(line);
	assert(!fclose(copy));
	assert(!fclose(orig));
}
