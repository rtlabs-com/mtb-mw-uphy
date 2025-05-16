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

/* define lwip structs first */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include <assert.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <string.h>

#include "rte_sock.h"

struct rte_fd_set
{
   fd_set set;
};

int rte_socket (int domain, int type, int protocol)
{
   return lwip_socket (domain, type, protocol);
}

int rte_bind (int sockfd, const struct rte_sockaddr * addr, rte_socklen_t addrlen)
{
   return lwip_bind (sockfd, (const struct sockaddr *)addr, addrlen);
}

int rte_listen (int sockfd, int backlog)
{
   return lwip_listen (sockfd, backlog);
}

int rte_accept (int sockfd, struct rte_sockaddr * addr, rte_socklen_t * addrlen)
{
   return lwip_accept (sockfd, (struct sockaddr *)addr, addrlen);
}

int rte_connect (int sockfd, const struct rte_sockaddr * addr, rte_socklen_t addrlen)
{
   return lwip_connect (sockfd, (const struct sockaddr *)addr, addrlen);
}

int rte_send (int sockfd, const void * buf, size_t len, int flags)
{
   return lwip_send (sockfd, buf, len, flags);
}
int rte_sendto (
   int sockfd,
   const void * buf,
   size_t len,
   int flags,
   const struct rte_sockaddr * dest_addr,
   rte_socklen_t addrlen)
{
   return lwip_sendto (
      sockfd,
      buf,
      len,
      flags,
      (const struct sockaddr *)dest_addr,
      addrlen);
}

int rte_recv (int sockfd, void * buf, size_t len, int flags)
{
   return lwip_recv (sockfd, buf, len, flags);
}

int rte_close (int sockfd)
{
   return lwip_close (sockfd);
}

int rte_shutdown (int sockfd, int how)
{
   return lwip_shutdown (sockfd, how);
}

int rte_select (
   int nfds,
   rte_fd_set_t * read_fds,
   rte_fd_set_t * write_fds,
   rte_fd_set_t * except_fds,
   rte_timeval_t * timeout)
{
   struct timeval * tv_ptr = NULL;
   struct timeval tv;

   if (timeout)
   {
      tv.tv_sec = timeout->tv_sec;
      tv.tv_usec = timeout->tv_usec;
      tv_ptr = &tv;
   }

   return select (
      nfds,
      (fd_set *)&read_fds->set,
      (fd_set *)&write_fds->set,
      (fd_set *)&except_fds->set,
      tv_ptr);
}

rte_fd_set_t * rte_fd_set_alloc (void)
{
   rte_fd_set_t * fdset = malloc (sizeof (rte_fd_set_t));
   FD_CLR (0, &fdset->set);
   return (rte_fd_set_t *)fdset;
}

void rte_fd_set_free (rte_fd_set_t * set)
{
   free (set);
}

void rte_fd_set_add (int fd, rte_fd_set_t * fdset)
{
   if (fdset)
      FD_SET (fd, &fdset->set);
}

int rte_fd_is_set (int fd, rte_fd_set_t * fdset)
{
   return (fdset) ? FD_ISSET (fd, &fdset->set) : 0;
}

void rte_fd_clear (int fd, rte_fd_set_t * fdset)
{
   if (fdset)
      FD_CLR (fd, &fdset->set);
}

void rte_fd_zero (rte_fd_set_t * fdset)
{
   if (fdset)
      FD_ZERO (&fdset->set);
}

void * rte_fd_get_raw (rte_fd_set_t * fdset)
{
   return fdset ? (void *)&fdset->set : NULL;
}

void rte_fd_copy (rte_fd_set_t * dest, rte_fd_set_t * src)
{
   if (dest && src)
   {
      memcpy (&dest->set, &src->set, sizeof (fd_set));
   }
}

int rte_getpeername (int sockfd, struct rte_sockaddr * addr, rte_socklen_t * addrlen)
{
   return lwip_getpeername (sockfd, (struct sockaddr *)addr, addrlen);
}

uint16_t rte_htons (uint16_t n)
{
   return lwip_htons (n);
}

uint16_t rte_ntohs (uint16_t n)
{
   return lwip_ntohs (n);
}

uint32_t rte_htonl (uint32_t n)
{
   return lwip_htonl (n);
}

uint32_t rte_ntohl (uint32_t n)
{
   return lwip_ntohl (n);
}

int rte_setsockopt (
   int sockfd,
   int level,
   int optname,
   const void * optval,
   rte_socklen_t optlen)
{
   return lwip_setsockopt (sockfd, level, optname, optval, optlen);
}

int rte_fcntl (int s, int cmd, int val)
{
   return lwip_fcntl (s, cmd, val);
}

int rte_recvfrom (
   int sockfd,
   void * buf,
   size_t len,
   int flags,
   struct rte_sockaddr * src_addr,
   rte_socklen_t * addrlen)
{
   return lwip_recvfrom (
      sockfd,
      buf,
      len,
      flags,
      (struct sockaddr *)src_addr,
      addrlen);
}

char * rte_inet_ntoa (const struct rte_in_addr * addr)
{
   return inet_ntoa (addr);
}

uint32_t rte_inet_addr(const char *cp)
{
  ip4_addr_t val;

  if (ip4addr_aton(cp, &val)) {
    return ip4_addr_get_u32(&val);
  }
  return (IPADDR_NONE);
}

/* Sanity check parameter and types */

/* This wrapper file maps directly onto lwip stack
   and uses parameter values transparantly. To ensure
   there is no mismatch we do compile time checks on all
   constants and types */

#define STATIC_ASSERT_DEFINES_EQUAL(def1, def2)                                \
   static_assert (def1 == def2, "Define mismatch: " #def1 " != " #def2);

#define STATIC_ASSERT_STRUCT_SIZE_MATCH(type1, type2)                          \
   static_assert (                                                             \
      sizeof (type1) == sizeof (type2),                                        \
      "Size mismatch: " #type1 " vs " #type2);

#define STATIC_ASSERT_MEMBER_SIZE_EQUAL(type1, type2, member)                  \
   static_assert (                                                             \
      sizeof (((type1 *)0)->member) == sizeof (((type2 *)0)->member),          \
      "Size mismatch for member '" #member "' between " #type1                 \
      " and " #type2);

/* all constants currently used in this socket interface */
STATIC_ASSERT_DEFINES_EQUAL (MSG_NOSIGNAL, RTE_MSG_NOSIGNAL);

STATIC_ASSERT_DEFINES_EQUAL (SHUT_RD, RTE_SHUT_RD);
STATIC_ASSERT_DEFINES_EQUAL (SHUT_WR, RTE_SHUT_WR);
STATIC_ASSERT_DEFINES_EQUAL (SHUT_RDWR, RTE_SHUT_RDWR);

STATIC_ASSERT_DEFINES_EQUAL (F_GETFL, RTE_F_GETFL);
STATIC_ASSERT_DEFINES_EQUAL (F_SETFL, RTE_F_SETFL);

STATIC_ASSERT_DEFINES_EQUAL (IP_TOS, RTE_IP_TOS);

STATIC_ASSERT_DEFINES_EQUAL (AF_INET, RTE_AF_INET);
STATIC_ASSERT_DEFINES_EQUAL (IPADDR_NONE, RTE_IPADDR_NONE);
STATIC_ASSERT_DEFINES_EQUAL (IPADDR_LOOPBACK, RTE_IPADDR_LOOPBACK);
STATIC_ASSERT_DEFINES_EQUAL (IPADDR_ANY, RTE_IPADDR_ANY);
STATIC_ASSERT_DEFINES_EQUAL (IPADDR_BROADCAST, RTE_IPADDR_BROADCAST);

STATIC_ASSERT_DEFINES_EQUAL (IPPROTO_IP, RTE_IPPROTO_IP);
STATIC_ASSERT_DEFINES_EQUAL (IPPROTO_TCP, RTE_IPPROTO_TCP);
STATIC_ASSERT_DEFINES_EQUAL (IPPROTO_UDP, RTE_IPPROTO_UDP);

STATIC_ASSERT_DEFINES_EQUAL (IP_MULTICAST_TTL, RTE_IP_MULTICAST_TTL);
STATIC_ASSERT_DEFINES_EQUAL (IP_MULTICAST_IF, RTE_IP_MULTICAST_IF);

STATIC_ASSERT_DEFINES_EQUAL (SOCK_STREAM, RTE_SOCK_STREAM);
STATIC_ASSERT_DEFINES_EQUAL (SOCK_DGRAM, RTE_SOCK_DGRAM);
STATIC_ASSERT_DEFINES_EQUAL (SOCK_RAW, RTE_SOCK_RAW);

STATIC_ASSERT_DEFINES_EQUAL (INET_ADDRSTRLEN, RTE_INET_ADDRSTRLEN);
STATIC_ASSERT_DEFINES_EQUAL (SOL_SOCKET, RTE_SOL_SOCKET);
STATIC_ASSERT_DEFINES_EQUAL (SO_REUSEADDR, RTE_SO_REUSEADDR);
STATIC_ASSERT_DEFINES_EQUAL (SO_BROADCAST, RTE_SO_BROADCAST);
STATIC_ASSERT_DEFINES_EQUAL (O_NONBLOCK, RTE_O_NONBLOCK);

/* sockaddr */
STATIC_ASSERT_STRUCT_SIZE_MATCH (struct rte_sockaddr, struct sockaddr);
STATIC_ASSERT_MEMBER_SIZE_EQUAL (struct rte_sockaddr, struct sockaddr, sa_family);
STATIC_ASSERT_MEMBER_SIZE_EQUAL (struct rte_sockaddr, struct sockaddr, sa_data);

/* sockaddr_in */
STATIC_ASSERT_STRUCT_SIZE_MATCH (struct rte_sockaddr_in, struct sockaddr_in);
STATIC_ASSERT_MEMBER_SIZE_EQUAL (
   struct rte_sockaddr_in,
   struct sockaddr_in,
   sin_port);
STATIC_ASSERT_MEMBER_SIZE_EQUAL (
   struct rte_sockaddr_in,
   struct sockaddr_in,
   sin_family);
STATIC_ASSERT_MEMBER_SIZE_EQUAL (
   struct rte_sockaddr_in,
   struct sockaddr_in,
   sin_addr);

/* in_addr */
STATIC_ASSERT_STRUCT_SIZE_MATCH (struct rte_in_addr, struct in_addr);

/* socklen_t */
STATIC_ASSERT_STRUCT_SIZE_MATCH (rte_socklen_t, socklen_t);

/* in_port_t */
STATIC_ASSERT_STRUCT_SIZE_MATCH (rte_in_port_t, in_port_t);

/* sa_family_t */
STATIC_ASSERT_STRUCT_SIZE_MATCH (rte_sa_family_t, sa_family_t);

STATIC_ASSERT_MEMBER_SIZE_EQUAL (struct rte_in_addr, struct in_addr, s_addr);
