/*
 * eutils.c
 *
 * Cody Doucette
 * Boston University
 *
 * This file contains utility definitions for the echo clients and server.
 *
 */

#include "eutils.h"

/**
 * perrorq(): Print the given error and quit.
 */
void perrorq(char *s)
{
	perror(s);
	exit(1);
}

/**
 * setup_output_file(): Copy the name of the input file and append "_echo" to
 * it to get the name of the output file.
 */
void setup_output_file(char *orig, char *copy, int buflen)
{
	if (snprintf(copy, buflen, "%s%s", orig, FILE_APPEND) != buflen -1)
		perrorq("setup_output_file buffer error");
}

/**
 * send_packet(): Send a packet via the given socket.
 */
void send_packet(int s, char *buf, int nbytes, const struct sockaddr_in *srv)
{
	if (sendto(s, buf, nbytes, 0, (struct sockaddr *)srv,
			sizeof(struct sockaddr_in)) == -1)
		perrorq("sendto");
}

/**
 * recv_packet(): Receive a packet via the given socket.
 */
int recv_packet(int s, char *buf, int nbytes, const struct sockaddr_in *srv)
{
	int read;
	unsigned int len = sizeof(struct sockaddr_in);

	if ((read = recvfrom(s, buf, nbytes, 0,
			(struct sockaddr *)srv, &len)) == -1)
		perrorq("recvfrom");

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

        if (n_read == -1)
                perrorq("recvfrom");

        if (fwrite(out, sizeof(char), n_read, copy) < 0)
                perrorq("fwrite");
}

/**
 * __process_file(): Is called after each EOF is found in a file to write
 * the output to a given file. With a corked socket, max := CORK_SIZE, else
 * max := UDP_MAX.
 */
void __process_file(int s, const struct sockaddr_in *srv, char *line,
				     int n_read, FILE *copy, int max)
{
        int bytes_to_send = 0;
        int bytes_sent = 0;

        while (n_read > 0) {
                if (n_read > max)
                        bytes_to_send = max;
                else
                        bytes_to_send = n_read;

                send_packet(s, line + bytes_sent, bytes_to_send, srv);
                recv_write(s, copy, bytes_to_send, srv);

                n_read -= bytes_to_send;
                bytes_sent += bytes_to_send;
        }
}
