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

#ifndef RTE_NETWORK_H
#define RTE_NETWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct rte_netif_info
{
   uint32_t ip_addr;
   uint32_t netmask;
   uint32_t gateway;
   uint32_t name_server;
   uint8_t hwaddr[6];
};

struct rte_ip4_addr
{
   uint32_t addr;
};

typedef struct rte_ip4_addr rte_ip4_addr_t;

int rte_get_netif_info (const char * iface, struct rte_netif_info * info);
int rte_get_hostname (const char * iface, char * hostname, size_t hostname_len);
int rte_wait_for_ip (const char * iface);

uint16_t rte_htons (uint16_t n);
#define rte_ntohs rte_htons
uint32_t rte_htonl (uint32_t n);
#define rte_ntohl rte_htonl

/* To ease transition to RTE, the following definitions are made.
   They will be removed at some point!
   */
#ifndef htons
#define htons(x) rte_htons (x)
#endif

#ifndef ntohs
#define ntohs(x) rte_ntohs (x)
#endif

#ifndef htonl
#define htonl(x) rte_htonl (x)
#endif

#ifndef ntohl
#define ntohl(x) rte_ntohl (x)
#endif

uint32_t rte_ipaddr_addr (const char * cp);
int rte_ip4addr_aton (const char * cp, rte_ip4_addr_t * addr);

#endif /* RTE_NETWORK_H */
