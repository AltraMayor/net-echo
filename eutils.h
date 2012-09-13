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

/* The best appoach would be to use struct __kernel_sockaddr_storage
 * defined in <linux/socket.h>, or struct sockaddr_storage defined in libc.
 * However, while XIA doesn't make into mainline, these structs are only
 * half of the size needed.
 */
struct tmp_sockaddr_storage {
	char memory[256];
};

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

int datagram_socket(int is_xia);

int check_cli_params(int argc, char * const argv[]);

struct sockaddr *__get_addr(int is_xia, char *str1, char *str2, int *plen);

struct sockaddr *get_cli_addr(int is_xia, int argc, char * const argv[],
	int *plen);

struct sockaddr *get_srv_addr(int is_xia, int argc, char * const argv[],
	int *plen);

void datagram_bind(int is_xia, int force, int s, const struct sockaddr *addr,
	int addr_len);

void send_packet(int s, const char *buf, int n, const struct sockaddr *dst,
	socklen_t dst_len);

void recv_write(int s, const struct sockaddr *expected_src,
	socklen_t exp_src_len, FILE *copy, int n_sent);

/* Process File Function. */
typedef void (*pff_recvf_t)(int s, const struct sockaddr *srv,
	socklen_t srv_len, FILE *output, int length);

void process_file(int s, const struct sockaddr *srv, socklen_t srv_len,
	const char *orig_name, int chunk_size, pff_recvf_t f);

#endif /* _ECHO_UTILS_H */
