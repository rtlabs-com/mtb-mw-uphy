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

#include "rte_network.h"

/* modus toolbox remaps htons etc transparantly */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include "lwip/netif.h"
#include <string.h>
#include "osal.h"

int rte_get_netif_info (const char * iface, struct rte_netif_info * info)
{
   struct netif * netif = netif_find (iface);

   if ((netif == NULL) && (netif_default))
   {
      /* use default interface */
      netif = netif_default;
   }
   else
   {
      printf ("rte_get_netif_info failed\n");
      return -1;
   }

   info->ip_addr = netif->ip_addr.addr;
   info->netmask = netif->netmask.addr;
   info->gateway = netif->gw.addr;
   info->name_server = netif->gw.addr; // Assuming name server is the same as
                                       // gateway
   memcpy (info->hwaddr, netif->hwaddr, sizeof (netif->hwaddr));

   return 0;
}

int rte_wait_for_ip (const char * iface)
{
   struct netif * netif = netif_find (iface);

   if ((netif == NULL) && (netif_default))
   {
      /* use default interface */
      netif = netif_default;
   }
   else
   {
      printf ("rte_wait_for_ip failed\n");
      return -1;
   }

   while ((netif_is_link_up (netif) && netif->ip_addr.addr != 0) == false)
   {
      os_usleep (10 * 1000);
   }

   return 0;
}

int rte_get_hostname (const char * iface, char * hostname, size_t hostname_len)
{
   struct netif * netif = netif_find (iface);

   if ((netif == NULL) && (netif_default))
   {
      /* use default interface */
      netif = netif_default;
   }
   else
   {
      printf ("rte_get_hostname failed\n");
      return -1;
   }

   if (netif->hostname)
   {
      strncpy (hostname, netif->hostname, hostname_len);
      hostname[hostname_len - 1] = '\0'; // Ensure null-termination
   }
   else
   {
      memset (hostname, 0, hostname_len);
   }

   return 0;
}

uint32_t rte_ipaddr_addr (const char * cp)
{
   return ipaddr_addr (cp);
}

int rte_ip4addr_aton (const char * cp, rte_ip4_addr_t * addr)
{
   return ip4addr_aton (cp, (ip4_addr_t *)addr);
}