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

#include <lwip/netif.h>
#include <lwip/apps/snmp_core.h>
#include <lwip/lwip_hooks.h>
#include <lwip/tcpip.h>

#include "pnal.h"
#include "osal_log.h"

#define XMC72_EVK_ETHERNET_WORKAROUND

#ifdef XMC72_EVK_ETHERNET_WORKAROUND
#include <lwip/snmp.h>
#endif

#define MAX_NUMBER_OF_IF 1

#if !LWIP_TCPIP_CORE_LOCKING
#error LWIP_TCPIP_CORE_LOCKING must be enabled
#endif

#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO

#ifndef PF_PNAL_LOG
#define PF_PNAL_LOG (LOG_STATE_ON)
#endif

struct pnal_eth_handle
{
   struct netif * netif;
   pnal_eth_callback_t * eth_rx_callback;
   void * arg;
};

static pnal_eth_handle_t interface[MAX_NUMBER_OF_IF];
static int nic_index = 0;

/**
 * Find PNAL network interface handle
 *
 * @param netif            In:    lwip network interface.
 * @return PNAL network interface handle corresponding to \a netif,
 *         NULL otherwise.
 */
static pnal_eth_handle_t * pnal_eth_find_handle (struct netif * netif)
{
   pnal_eth_handle_t * handle;
   int i;

   for (i = 0; i < MAX_NUMBER_OF_IF; i++)
   {
      handle = &interface[i];
      if (handle->netif == netif)
      {
         return handle;
      }
   }

   return NULL;
}

/**
 * Allocate PNAL network interface handle
 *
 * Handles are allocated from a static array and need never be freed.
 *
 * @return PNAL network interface handle if available,
 *         NULL if too many handles were allocated.
 */
static pnal_eth_handle_t * pnal_eth_allocate_handle (void)
{
   pnal_eth_handle_t * handle;

   if (nic_index < MAX_NUMBER_OF_IF)
   {
      handle = &interface[nic_index];
      nic_index++;
      return handle;
   }
   else
   {
      return NULL;
   }
}


/**
 * Process received Ethernet frame
 *
 * Called from lwip when an Ethernet frame is received with an EtherType
 * lwip is not aware of (e.g. Profinet and LLDP).
 *
 * @param p_buf            InOut: Packet buffer containing Ethernet frame.
 * @param netif            InOut: Network interface receiving the frame.
 * @return ERR_OK if frame was processed and freed,
 *         ERR_IF if it was ignored.
 */
static err_t pnal_eth_sys_recv (struct pbuf * p_buf, struct netif * netif)
{
   int processed;
   pnal_eth_handle_t * handle;

   handle = pnal_eth_find_handle (netif);
   if (handle == NULL)
   {
      /* p-net not started yet, let lwIP handle frame */
      return ERR_IF;
   }

#ifdef XMC72_EVK_ETHERNET_WORKAROUND
   p_buf->tot_len -= 4;
   p_buf->len -= 4;

   /* Workaround for missing statistics implementation in infineon driver. */
   MIB2_STATS_NETIF_INC (netif, ifinoctets);
#endif

   processed = handle->eth_rx_callback (handle, handle->arg, (pnal_buf_t *)p_buf);
   if (processed)
   {
      /* Frame handled and freed */
      return ERR_OK;
   }
   else
   {
      /* Frame not handled */
      return ERR_IF;
   }
}

pnal_eth_handle_t * pnal_eth_init (
   const char * if_name,
   pnal_ethertype_t receive_type,
   pnal_eth_callback_t * callback,
   void * arg)
{
   pnal_eth_handle_t * handle;
   struct netif * netif;

   (void)receive_type; /* Ignore, for now all frames will be received. */

   netif = netif_find (if_name);
   if (netif == NULL)
   {
      os_log (LOG_LEVEL_ERROR, "Network interface \"%s\" not found!\n", if_name);
      return NULL;
   }

   handle = pnal_eth_allocate_handle();
   if (handle == NULL)
   {
      os_log (LOG_LEVEL_ERROR, "Too many network interfaces\n");
      return NULL;
   }

   lwip_set_hook_for_unknown_eth_protocol (netif, pnal_eth_sys_recv);

   handle->arg = arg;
   handle->eth_rx_callback = callback;
   handle->netif = netif;

   return handle;
}

/* tbd - embed timestamp to monitor elapsed time to low level driver */
int pnal_eth_send (pnal_eth_handle_t * handle, pnal_buf_t * buf)
{
   struct pbuf * p_buf = (struct pbuf *)buf;
   int ret = -1;

   CC_ASSERT (handle->netif->linkoutput != NULL);

   /* TODO: Determine if buf could ever be NULL here */
   if (p_buf != NULL)
   {
#ifdef XMC72_EVK_ETHERNET_WORKAROUND
      /* Workaround for missing statistics implementation in infineon driver. */
      MIB2_STATS_NETIF_INC (handle->netif, ifoutoctets);
#endif

      /* TODO: remove tot_len from os_buff */
      p_buf->tot_len = p_buf->len;

      LOCK_TCPIP_CORE();
      handle->netif->linkoutput (handle->netif, p_buf);
      UNLOCK_TCPIP_CORE();
      ret = p_buf->len;
   }
   return ret;
}
