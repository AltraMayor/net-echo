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

#include <stdio.h>
#include <sys/socket.h>

/* 0xffff - sizeof(Maximum UDP header)
 *	- sizeof(IP header without options) - sizeof(Ethernet header)
 *
 * The second part of the sum shouldn't be necessary, however,
 * while communicating through loopback device,
 * not removing Ethernet header confuses Wireshark, and
 * not removing IP header confuses the kernel.
 */
#define MAX_UDP (0xffff - 8 - 20 - 14)

static inline int is_file(const char *x)
{
	return (*x == '-') && (*(x + 1) == 'f');
}

void check_cli_params(int argc, char * const argv[]);

void send_packet(int s, const char *buf, int n, const struct sockaddr *dst);

void recv_write(int s, const struct sockaddr *expected_src, FILE *copy,
	int n_sent);

/* Process File Function. */
typedef void (*pff_recvf_t)(int, const struct sockaddr *, FILE *, int);

void process_file(int s, const struct sockaddr *srv, const char *orig_name,
	int chunk_size, pff_recvf_t f);

#endif /* _ECHO_UTILS_H */
