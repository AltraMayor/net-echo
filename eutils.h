/*
 * eutils.h
 *
 * Cody Doucette
 * Boston University
 *
 * A header file for the utilities that support the echo clients and server.
 *
 */

#ifndef _ECHO_UTILS_H
#define _ECHO_UTILS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* 0xffff - sizeof(Maximum UDP header) */
#define MAX_UDP (0xffff - 8)
#define FILE_APPEND "_echo"

static inline int is_file(const char *x)
{
	return (*x == '-') && (*(x + 1) == 'f');
}

void perrorq(char *s);

void setup_output_file(char *orig, char *copy, int buflen);

void send_packet(int s, char *buf, int nbytes, const struct sockaddr_in *srv);

int recv_packet(int s, char *buf, int nbytes, const struct sockaddr_in *srv);

void recv_write(int s, FILE *copy, int n, const struct sockaddr_in *srv);

void __process_file(int s, const struct sockaddr_in *srv, char *line,
				    int n_read, FILE *copy, int max);
#endif /* _ECHO_UTILS_H */
