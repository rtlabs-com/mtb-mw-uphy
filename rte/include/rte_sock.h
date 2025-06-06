/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2025 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#ifndef RTE_SOCK_H
#define RTE_SOCK_H

#include <stdint.h>
#include <stddef.h>
#include "rte_types.h"

#if RTE_INTERFACE_BUILD == 1
#include "rte_sock_int.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * NOTE:
 * RTE constants typically matches that of LWIP, for other stacks a conversion
 * of parameters and types needs to be done in c wrapper implementation
 *
 ******************************************************************************/

#define RTE_SHUT_RD   0
#define RTE_SHUT_WR   1
#define RTE_SHUT_RDWR 2

/* Socket types */
#define RTE_SOCK_STREAM 1
#define RTE_SOCK_DGRAM  2
#define RTE_SOCK_RAW    3

/* Address families */
#define RTE_AF_INET 2

/* Socket option levels */
#define RTE_SOL_SOCKET 0xFFF

/* Socket options */
#define RTE_SO_REUSEADDR 0x0004

/* Protocols */
#define RTE_IPPROTO_IP  0
#define RTE_IPPROTO_TCP 6
#define RTE_IPPROTO_UDP 17

/* Multicast options */
#define RTE_IP_MULTICAST_TTL 5
#define RTE_IP_MULTICAST_IF  6

/** 255.255.255.255 */
#define RTE_IPADDR_NONE ((uint32_t)0xffffffffUL)

/** 127.0.0.1 */
#define RTE_IPADDR_LOOPBACK ((uint32_t)0x7f000001UL)

/** 0.0.0.0 */
#define RTE_IPADDR_ANY ((uint32_t)0x00000000UL)

/** 255.255.255.255 */
#define RTE_IPADDR_BROADCAST ((uint32_t)0xffffffffUL)

#define RTE_SO_BROADCAST 0x0020

/* nonblocking I/O */
#define RTE_O_NONBLOCK 1

/* maximum length of a string representation of an IPv4 address */
#define RTE_INET_ADDRSTRLEN 16

/* commands for fnctl */
#define RTE_F_GETFL 3
#define RTE_F_SETFL 4
/*
 * Options for level IPPROTO_IP
 */
#define RTE_IP_TOS 1

#define RTE_TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define RTE_TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define RTE_TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define RTE_TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define RTE_TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */

/*
 * Additional options, not kept in so_options.
 */
#define RTE_SO_DEBUG        0x0001 /* Unimplemented: turn on debugging info recording */
#define RTE_SO_ACCEPTCONN   0x0002 /* socket has had listen() */
#define RTE_SO_DONTROUTE    0x0010 /* Unimplemented: just use interface addresses */
#define RTE_SO_USELOOPBACK  0x0040 /* Unimplemented: bypass hardware when possible */
#define RTE_SO_LINGER       0x0080 /* linger on close if data present */
#define RTE_SO_DONTLINGER   ((int)(~RTE_SO_LINGER))
#define RTE_SO_OOBINLINE    0x0100 /* Unimplemented: leave received OOB data in line */
#define RTE_SO_REUSEPORT    0x0200 /* Unimplemented: allow local address & port reuse */
#define RTE_SO_SNDBUF       0x1001 /* Unimplemented: send buffer size */
#define RTE_SO_RCVBUF       0x1002 /* receive buffer size */
#define RTE_SO_SNDLOWAT     0x1003 /* Unimplemented: send low-water mark */
#define RTE_SO_RCVLOWAT     0x1004 /* Unimplemented: receive low-water mark */
#define RTE_SO_SNDTIMEO     0x1005 /* send timeout */
#define RTE_SO_RCVTIMEO     0x1006 /* receive timeout */
#define RTE_SO_ERROR        0x1007 /* get error status and clear */
#define RTE_SO_TYPE         0x1008 /* get socket type */
#define RTE_SO_CONTIMEO     0x1009 /* Unimplemented: connect timeout */
#define RTE_SO_NO_CHECK     0x100a /* don't create UDP checksum */
#define RTE_SO_BINDTODEVICE 0x100b /* bind to device */

/*
 * Option flags per-socket
 */
#define RTE_SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define RTE_SO_KEEPALIVE   0x0008 /* keep connections alive */
#define RTE_SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */

/* Address structure */
struct rte_sockaddr
{
   uint8_t sa_len;
   uint8_t sa_family;
   char sa_data[14];
};

typedef uint8_t rte_sa_family_t;

/* data type used for representing port numbers in network programming
   (the definition matches that of netinet) */
typedef uint16_t rte_in_port_t;

/* Define rte_in_addr to match lwip variant */
struct rte_in_addr
{
   uint32_t s_addr;
};

/* IPv4-specific address structure */
struct rte_sockaddr_in
{
   uint8_t sin_len;
   rte_sa_family_t sin_family;
   rte_in_port_t sin_port;
   struct rte_in_addr sin_addr;
#define SIN_ZERO_LEN 8
   char sin_zero[SIN_ZERO_LEN];
};

/* size of a socket address structure */
typedef uint32_t rte_socklen_t;

/* need to allocate fd_set dynamically as we don't have the
   system definition of fd_set in terms of max nbr descriptors */

rte_fd_set_t * rte_fd_set_alloc (void);
void rte_fd_set_free (rte_fd_set_t * fdset);

void rte_fd_set_add (int fd, rte_fd_set_t * fdset);
int rte_fd_is_set (int fd, rte_fd_set_t * fdset);
void rte_fd_clear (int fd, rte_fd_set_t * fdset);
void rte_fd_zero (rte_fd_set_t * fdset);
void * rte_fd_get_raw (rte_fd_set_t * fdset);
void rte_fd_copy (rte_fd_set_t * dest, rte_fd_set_t * src);

int rte_select (
   int nfds,
   rte_fd_set_t * read_fds,
   rte_fd_set_t * write_fds,
   rte_fd_set_t * except_fds,
   rte_timeval_t * timeout);

/* Function prototypes */
int rte_socket (int domain, int type, int protocol);
int rte_bind (int sockfd, const struct rte_sockaddr * addr, rte_socklen_t addrlen);
int rte_listen (int sockfd, int backlog);
int rte_accept (int sockfd, struct rte_sockaddr * addr, rte_socklen_t * addrlen);
int rte_connect (int sockfd, const struct rte_sockaddr * addr, rte_socklen_t addrlen);
int rte_send (int sockfd, const void * buf, size_t len, int flags);
int rte_sendto (
   int sockfd,
   const void * buf,
   size_t len,
   int flags,
   const struct rte_sockaddr * dest_addr,
   rte_socklen_t addrlen);
int rte_recv (int sockfd, void * buf, size_t len, int flags);
int rte_recvfrom (
   int sockfd,
   void * buf,
   size_t len,
   int flags,
   struct rte_sockaddr * src_addr,
   rte_socklen_t * addrlen);
int rte_close (int sockfd);
int rte_shutdown (int sockfd, int how);

int rte_getpeername (int sockfd, struct rte_sockaddr * addr, rte_socklen_t * addrlen);
char * rte_inet_ntoa (const struct rte_in_addr * addr);
int rte_setsockopt (
   int sockfd,
   int level,
   int optname,
   const void * optval,
   rte_socklen_t optlen);
int rte_fcntl (int sockfd, int cmd, int val);
uint32_t rte_inet_addr (const char * cp);

void rte_sock_validate (void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_SOCK_H */
