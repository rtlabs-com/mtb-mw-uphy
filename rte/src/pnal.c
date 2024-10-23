/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#include "pnal.h"

#include "pnet_options.h"
#include "osal.h"
#include "osal_log.h"
#include "rte_fs.h"

#if LWIP_IPV6
#error "no ipv6 supported"
#endif

/* modus toolbox remaps htons etc transparantly */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include <lwip/apps/snmp.h>
#include <lwip/netif.h>

#include <lwip/snmp.h>
#include <lwip/sys.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO

#ifndef PF_PNAL_LOG
#define PF_PNAL_LOG (LOG_STATE_ON)
#endif

int pnal_set_ip_suite (
   const char * interface_name,
   const pnal_ipaddr_t * p_ipaddr,
   const pnal_ipaddr_t * p_netmask,
   const pnal_ipaddr_t * p_gw,
   const char * hostname,
   bool permanent)
{
   ip_addr_t ip_addr;
   ip_addr_t ip_mask;
   ip_addr_t ip_gw;

   ip_addr.addr = htonl (*p_ipaddr);
   ip_mask.addr = htonl (*p_netmask);
   ip_gw.addr = htonl (*p_gw);
   netif_set_addr (netif_default, &ip_addr, &ip_mask, &ip_gw);

   return 0;
}

int pnal_get_macaddress (const char * interface_name, pnal_ethaddr_t * mac_addr)
{
   memcpy (mac_addr, netif_default->hwaddr, sizeof (pnal_ethaddr_t));
   return 0;
}

pnal_ipaddr_t pnal_get_ip_address (const char * interface_name)
{
   CC_ASSERT (netif_default);
   return htonl (netif_default->ip_addr.addr);
}

pnal_ipaddr_t pnal_get_netmask (const char * interface_name)
{
   CC_ASSERT (netif_default);
   return htonl (netif_default->netmask.addr);
}

pnal_ipaddr_t pnal_get_gateway (const char * interface_name)
{
   CC_ASSERT (netif_default);
   return htonl (netif_default->gw.addr);
}

int pnal_get_hostname (char * hostname)
{
   CC_ASSERT (netif_default);
   strncpy (hostname, netif_default->hostname, PNAL_HOSTNAME_MAX_SIZE);
   hostname[PNAL_HOSTNAME_MAX_SIZE - 1] = '\0';
   return 0;
}

int pnal_get_ip_suite (
   const char * interface_name,
   pnal_ipaddr_t * p_ipaddr,
   pnal_ipaddr_t * p_netmask,
   pnal_ipaddr_t * p_gw,
   char * hostname)
{
   int ret = -1;

   *p_ipaddr = pnal_get_ip_address (interface_name);
   *p_netmask = pnal_get_netmask (interface_name);
   *p_gw = pnal_get_gateway (interface_name);
   ret = pnal_get_hostname (hostname);

   return ret;
}

#ifndef MIB2_STATS
#error "no stats"
#endif

int pnal_get_port_statistics (
   const char * interface_name,
   pnal_port_stats_t * port_stats)
{
#ifdef MIB2_STATS
   port_stats->if_in_octets = netif_default->mib2_counters.ifinoctets;
   port_stats->if_in_errors = netif_default->mib2_counters.ifinerrors;
   port_stats->if_in_discards = netif_default->mib2_counters.ifindiscards;
   port_stats->if_out_octets = netif_default->mib2_counters.ifoutoctets;
   port_stats->if_out_errors = netif_default->mib2_counters.ifouterrors;
   port_stats->if_out_discards = netif_default->mib2_counters.ifoutdiscards;
#endif
   return 0;
}

int pnal_get_interface_index (const char * interface_name)
{
   return 0;
}

int pnal_eth_get_status (const char * interface_name, pnal_eth_status_t * status)
{
   /* fixme -- query ethernet driver on status */
   status->is_autonegotiation_supported = false;
   status->is_autonegotiation_enabled = false;
   status->autonegotiation_advertised_capabilities = 0;

   status->operational_mau_type = PNAL_ETH_MAU_COPPER_100BaseTX_FULL_DUPLEX;
   status->running = true;

   return 0;
}

int pnal_save_file (
   const char * fullpath,
   const void * object_1,
   size_t size_1,
   const void * object_2,
   size_t size_2)
{
   int ret = 0;
   RTE_FILE * file;

   file = rte_fs_fopen (fullpath, "w");
   if (file == NULL)
   {
      LOG_ERROR (
         PF_PNAL_LOG,
         "PNAL(%d): Failed to open file %s: %s\n",
         __LINE__,
         fullpath,
         rte_fs_error (NULL));
      return -1;
   }

   if (size_1 > 0)
   {
      if (rte_fs_fwrite (object_1, size_1, 1, file) != 1)
      {
         ret = -1;

         LOG_ERROR (
            PF_PNAL_LOG,
            "PNAL(%d): Failed to write to file %s: %s\n",
            __LINE__,
            fullpath,
            rte_fs_error (file));
      }
   }

   if (size_2 > 0 && ret == 0)
   {
      if (rte_fs_fwrite (object_2, size_2, 1, file) != 1)
      {
         ret = -1;

         LOG_ERROR (
            PF_PNAL_LOG,
            "PNAL(%d): Failed to write to file %s: %s\n",
            __LINE__,
            fullpath,
            rte_fs_error (file));
      }
   }

   if (rte_fs_fclose (file) != 0)
   {
      ret = -1;

      LOG_ERROR (
         PF_PNAL_LOG,
         "PNAL(%d): Failed to close file %s: %s\n",
         __LINE__,
         fullpath,
         rte_fs_error (NULL));
   }

   return ret;
}

void pnal_clear_file (const char * fullpath)
{
   LOG_DEBUG (PF_PNAL_LOG, "PNAL(%d): Clearing file %s\n", __LINE__, fullpath);
   if (rte_fs_remove (fullpath) < 0)
   {
      LOG_ERROR (
         PF_PNAL_LOG,
         "PNAL(%d): Failed to clear file %s, : %s\n",
         __LINE__,
         fullpath,
         rte_fs_error (NULL));
   }
}

int pnal_load_file (
   const char * fullpath,
   void * object_1,
   size_t size_1,
   void * object_2,
   size_t size_2)
{
   int ret = 0;
   RTE_FILE * file;

   file = rte_fs_fopen (fullpath, "r");
   if (file == NULL)
   {
      LOG_ERROR (
         PF_PNAL_LOG,
         "PNAL(%d): Failed to open file %s: %s\n",
         __LINE__,
         fullpath,
         rte_fs_error (NULL));
      return -1;
   }

   if (size_1 > 0)
   {
      if (rte_fs_fread (object_1, size_1, 1, file) != 1)
      {
         ret = -1;

         LOG_ERROR (
            PF_PNAL_LOG,
            "PNAL(%d): Failed to read from file %s: %s\n",
            __LINE__,
            fullpath,
            rte_fs_error (file));
      }
   }

   if (size_2 > 0 && ret == 0)
   {
      if (rte_fs_fread (object_2, size_2, 1, file) != 1)
      {
         ret = -1;

         LOG_ERROR (
            PF_PNAL_LOG,
            "PNAL(%d): Failed to read from file %s: %s\n",
            __LINE__,
            fullpath,
            rte_fs_error (file));
      }
   }

   if (rte_fs_fclose (file) != 0)
   {
      ret = -1;

      LOG_ERROR (
         PF_PNAL_LOG,
         "PNAL(%d): Failed to close file %s: %s\n",
         __LINE__,
         fullpath,
         rte_fs_error (NULL));
   }

   return ret;
}

uint32_t pnal_get_system_uptime_10ms (void)
{
   uint32_t uptime = 0;

   MIB2_COPY_SYSUPTIME_TO (&uptime);
   return uptime;
}

pnal_buf_t * pnal_buf_alloc (uint16_t length)
{
   return (pnal_buf_t *)pbuf_alloc (PBUF_RAW, length, PBUF_POOL);
}

void pnal_buf_free (pnal_buf_t * p)
{
   CC_ASSERT (pbuf_free ((struct pbuf *)p) == 1);
}

uint8_t pnal_buf_header (pnal_buf_t * p, int16_t header_size_increment)
{
   return pbuf_header ((struct pbuf *)p, header_size_increment);
}
