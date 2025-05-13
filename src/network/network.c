/******************************************************************************
 * File Name:   network.c
 *
 * Description: Ethernet initialization.
 *              Based on udp_server example.
 *
 ********************************************************************************
 * Copyright 2022, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/* Header file includes */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>

/* Ethernet connection manager header files */
#include "cy_ecm.h"
#include "cy_ecm_error.h"

#include "network.h"
#include "shell.h"
#include "rte_fs.h"

/* Standard C header files */
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <lwip/netif.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/dhcp.h"

#include "shell.h"

#include "cy_eth_phy_driver.h"

/* Ethernet interface ID */
#ifdef XMC7100D_F176K4160
#define INTERFACE_ID CY_ECM_INTERFACE_ETH0
#else
#define INTERFACE_ID CY_ECM_INTERFACE_ETH1
#endif

cy_ecm_phy_callbacks_t phy_callbacks = {
   .phy_init = cy_eth_phy_init,
   .phy_configure = cy_eth_phy_configure,
   .phy_enable_ext_reg = cy_eth_phy_enable_ext_reg,
   .phy_discover = cy_eth_phy_discover,
   .phy_get_auto_neg_status = cy_eth_phy_get_auto_neg_status,
   .phy_get_link_partner_cap = cy_eth_phy_get_link_partner_cap,
   .phy_get_linkspeed = cy_eth_phy_get_linkspeed,
   .phy_get_linkstatus = cy_eth_phy_get_linkstatus,
   .phy_reset = cy_eth_phy_reset};

/* variable used to maintain hostname in lwip */
static char net_hostname[64];

/*******************************************************************************
 * Macros
 ********************************************************************************/

/*
 * In ECM v2.1.0 the option of configuring static IP address in runtime was
 * removed Furthermore, when supplying a static ip address to the
 * cy_ecm_connect() function this is overridden by settings configured in device
 * configurator tool
 *
 * Out of the box, u-phy needs to configure either DHCP or static ip depending
 * on which protocol is active. Hence we have added a workaround which
 * re-configures the network via LWIP once ECM connection is setup. This will be
 * removed once the ECM issues have been addressed.
 */

#define ECM2_1_WORKAROUND

/* Maximum number of connection retries to the ethernet network */
#define MAX_ETH_RETRY_COUNT (3u)

/* static void read_mac_from_file (cy_ecm_mac_t * mac_address); */

const char * mac_file = STORAGE_ROOT "mac";

static cy_ecm_t ecm_handle = NULL;

static void dhcp_set (struct netif * netif, bool enable)
{
   if (enable)
   {
      dhcp_start (netif);
   }
   else
   {
      dhcp_stop (netif);
      dhcp_cleanup (netif);
      netif_set_down (netif);
      netif_set_up (netif);
   }
}

int dhcp_is_enabled (struct netif * netif)
{
   struct dhcp * dhcp_data = netif_dhcp_data (netif);
   return (dhcp_data != NULL) ? 1 : 0;
}

#ifdef ECM2_1_WORKAROUND

void ecm_workaround (ip_config_t ip_config)
{
   /* re-apply network settings depending on active fieldbus protocol */
   if (ip_config == IP_CONFIG_STATIC)
   {
      cy_ecm_ip_setting_t static_ip_addr;
      static_ip_addr.ip_address.version = CY_ECM_IP_VER_V4;
      static_ip_addr.ip_address.ip.v4 = APP_STATIC_IP_ADDR;
      static_ip_addr.gateway.version = CY_ECM_IP_VER_V4;
      static_ip_addr.gateway.ip.v4 = APP_STATIC_GATEWAY;
      static_ip_addr.netmask.version = CY_ECM_IP_VER_V4;
      static_ip_addr.netmask.ip.v4 = APP_NETMASK;

      dhcp_set (netif_default, false);

      netif_set_addr (
         netif_default,
         (const ip4_addr_t *)&static_ip_addr.ip_address.ip.v4,
         (const ip4_addr_t *)&static_ip_addr.netmask.ip.v4,
         (const ip4_addr_t *)&static_ip_addr.gateway.ip.v4);
   }
   else
   {
      /* enable dhcp */
      dhcp_set (netif_default, true);
   }
}

#endif

/*
 * multiple events from lwip stack with same info
 * maintain copy and only print when modified
 */
static cy_ecm_event_data_t prev_evt_data = {0};

static void ethernet_event_callback (
   cy_ecm_event_t event,
   cy_ecm_event_data_t * event_data)
{
   switch (event)
   {
   case CY_ECM_EVENT_CONNECTED:
      printf ("Ethernet connected.\n");
      break;
   case CY_ECM_EVENT_DISCONNECTED:
      printf ("Ethernet disconnected.\n");
      break;
   case CY_ECM_EVENT_IP_CHANGED:
   {
      cy_ecm_event_data_t * prev_evt = &prev_evt_data;

      if (memcmp (prev_evt, event_data, sizeof (cy_ecm_event_data_t)) == 0)
         return;

      memcpy (prev_evt, event_data, sizeof (cy_ecm_event_data_t));

      printf (
         "IP address changed : %s\n",
         ipaddr_ntoa ((const ip_addr_t *)&event_data->ip_addr.ip.v4));
   }
   break;
   default:
      break;
   }
}

cy_rslt_t connect_to_ethernet (ip_config_t ip_config)
{
   cy_rslt_t result = CY_RSLT_SUCCESS;
   uint8_t retry_count = 0;

   /* Variables used by Ethernet connection manager.*/
   cy_ecm_ip_address_t ip_addr;

   /* Initialize ethernet connection manager. */
   result = cy_ecm_init();
   if (result != CY_RSLT_SUCCESS)
   {
      printf (
         "Ethernet connection manager initialization failed! Error code: "
         "0x%08" PRIx32 "\n",
         (uint32_t)result);
      CY_ASSERT (0);
   }
   else
   {
      printf ("Ethernet connection manager initialized.\n");
   }

   printf ("IP: %s\n", (ip_config == IP_CONFIG_STATIC) ? "Static" : "Dynamic");

   /* Initialize the Ethernet Interface and PHY driver */
   result = cy_ecm_ethif_init (INTERFACE_ID, &phy_callbacks, &ecm_handle);
   if (result != CY_RSLT_SUCCESS)
   {
      printf (
         "Ethernet interface initialization failed! Error code: 0x%08" PRIx32
         "\n",
         (uint32_t)result);

      CY_ASSERT (0);
   }

   result = cy_ecm_register_event_callback (ecm_handle, ethernet_event_callback);

   /* Establish a connection to the ethernet network */
   while (1)
   {
      if (ip_config == IP_CONFIG_STATIC)
      {
         cy_ecm_ip_setting_t static_ip_addr;

         static_ip_addr.ip_address.version = CY_ECM_IP_VER_V4;
         static_ip_addr.ip_address.ip.v4 = APP_STATIC_IP_ADDR;
         static_ip_addr.gateway.version = CY_ECM_IP_VER_V4;
         static_ip_addr.gateway.ip.v4 = APP_STATIC_GATEWAY;
         static_ip_addr.netmask.version = CY_ECM_IP_VER_V4;
         static_ip_addr.netmask.ip.v4 = APP_NETMASK;

         result = cy_ecm_connect (ecm_handle, &static_ip_addr, &ip_addr);
      }
      else
      {
         result = cy_ecm_connect (ecm_handle, NULL, &ip_addr);
      }

      if (result != CY_RSLT_SUCCESS)
      {
         retry_count++;
         if (retry_count >= MAX_ETH_RETRY_COUNT)
         {
            printf ("Exceeded max ethernet connection attempts\n");
            return result;
         }
         printf ("Connection to ethernet network failed. Retrying...\n");
         continue;
      }
      else
      {
#ifdef ECM2_1_WORKAROUND
         ecm_workaround (ip_config);
#endif
         printf ("Successfully connected to Ethernet.\n");

         // cy_ecm_set_promiscuous_mode(ecm_handle, true);
         cy_ecm_broadcast_disable (ecm_handle, false);
         break;
      }
   }

   return result;
}

/**
 * Read the MAC address from a file.
 * If the file does not exist or is not valid, the MAC address is set to 0.
 *
 * @param mac_address MAC address buffer to write to
 */
/*
static void read_mac_from_file (cy_ecm_mac_t * mac_address)
{
  int n_bytes;
  RTE_FILE * f = rte_fs_fopen (mac_file, "r");

  if (f < 0)
  {
     return;
  }
  n_bytes = rte_fs_fread (mac_address, 1, 6, f);
  if (n_bytes != 6)
  {
     memset (mac_address, 0, 6);
  }
  rte_fs_fclose (f);
}
*/

/**
 * Convert a string to a MAC address.
 * Avoids using sscanf.
 *
 * @param str MAC address string in format XX:XX:XX:XX:XX:XX
 * @param mac MAC address buffer to write to
 * @return 0 on success, -1 on error
 */
int str2mac (const char * str, cy_ecm_mac_t mac)
{
   if (
      strlen (str) != 17 || str[2] != ':' || str[5] != ':' || str[8] != ':' ||
      str[11] != ':' || str[14] != ':')
   {
      return -1;
   }

   for (int i = 0; i < 6; i++)
   {
      char byte_str[3] = {str[i * 3], str[i * 3 + 1], '\0'};
      char * endptr;
      mac[i] = (uint8_t)strtol (byte_str, &endptr, 16);
      if (*endptr != '\0')
      {
         return -1;
      }
   }

   return 0;
}

/*
 * disabled until infineon adds back method of updating mac address in runtime
 * currently mac address is hardcoded via device configurator and cannot be changed
 * without modifying ethernet-connection-manager component
 */
#if 0
int _cmd_mac (int argc, char * argv[])
{
   cy_ecm_mac_t mac_address;
   RTE_FILE * f;
   int n_bytes;

   if (argc == 1)
   {
      f = rte_fs_fopen (mac_file, "r");
      if (f < 0)
      {
         printf ("No MAC address file. Default MAC address (00:03:19:45:00:00) "
                 "is used\n");
         return 0;
      }

      n_bytes = rte_fs_fread (mac_address, 1, sizeof (mac_address), f);
      rte_fs_fclose (f);

      if (n_bytes != sizeof (mac_address))
      {
         printf (
            "Unexpected data in MAC address file. File will be deleted.\n");
         rte_fs_remove (mac_file);
         return 0;
      }

      printf (
         "MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         mac_address[0],
         mac_address[1],
         mac_address[2],
         mac_address[3],
         mac_address[4],
         mac_address[5]);
   }
   else if (argc == 2)
   {
      if (str2mac (argv[1], mac_address) != 0)
      {
         printf ("Error: Invalid MAC address format\n");
         return -1;
      }

      f = rte_fs_fopen (mac_file, "w");
      if (f < 0)
      {
         printf ("Error: Could not open file %s\n", mac_file);
         return 0;
      }

      n_bytes = rte_fs_fwrite (mac_address, 1, sizeof (mac_address), f);
      rte_fs_fclose (f);

      if (n_bytes != sizeof (mac_address))
      {
         printf ("Error: Could not write MAC address to file\n");
         return 0;
      }

      printf ("Ok, MAC address written to file\n");
   }
   else
   {
      printf ("Usage: mac [<MAC address>]\n");
      return -1;
   }

   return 0;
}

const shell_cmd_t cmd_mac = {
   .cmd = _cmd_mac,
   .name = "mac",
   .help_short = "read/write MAC address",
   .help_long =
      "Read or write the MAC address to file.\n"
      "Usage: mac [<MAC address>]\n"
      "If no MAC address is given, the current MAC address is shown.\n"
      "The MAC address should be in the format XX:XX:XX:XX:XX:XX."};

SHELL_CMD (cmd_mac);
#endif

static void show_lwip_netconfig (struct netif * netif)
{
   ip_addr_t ip_addr = netif->ip_addr;
   ip_addr_t netmask = netif->netmask;
   ip_addr_t gw = netif->gw;

   if (netif == NULL)
   {
      printf ("network interface not initialized\n");
      return;
   }

   /* assume only one adapter (0) */
   printf ("\n[%s0] : \n", netif->name);

   printf (
      "  mac address : %02x:%02x:%02x:%02x:%02x:%02x\n",
      netif->hwaddr[0],
      netif->hwaddr[1],
      netif->hwaddr[2],
      netif->hwaddr[3],
      netif->hwaddr[4],
      netif->hwaddr[5]);

   printf ("  ipaddress   : %s\n", ipaddr_ntoa (&ip_addr));
   printf ("  netmask     : %s\n", ipaddr_ntoa (&netmask));
   printf ("  gateway     : %s\n", ipaddr_ntoa (&gw));

   if (netif->hostname)
      printf ("  hostname    : %s\n", netif->hostname);
   else
      printf ("  hostname    : not set\n");

   printf (
      "  dhcp        : %s\n",
      dhcp_is_enabled (netif) ? "enabled" : "disabled");
}

int netcfg_set (
   struct netif * netif,
   const char * ip_str,
   const char * netmask_str,
   const char * gw_str,
   const char * hostname)
{
   ip4_addr_t ipaddr;
   ip4_addr_t netmask;
   ip4_addr_t gw;

   if (ip_str)
   {
      if (!ip4addr_aton (ip_str, &ipaddr))
         printf ("Invalid IP address format: %s\n", ip_str);
      else
         netif_set_ipaddr (netif, &ipaddr);
      return -1;
   }

   if (netmask_str)
   {
      if (!ip4addr_aton (netmask_str, &netmask))
         printf ("Invalid netmask format: %s\n", netmask_str);
      else
         netif_set_netmask (netif, &netmask);
      return -1;
   }

   if (gw_str)
   {
      if (!ip4addr_aton (gw_str, &gw))
         printf ("Invalid gateway address format: %s\n", gw_str);
      else
         netif_set_gw (netif, &gw);
      return -1;
   }

   if (hostname && netif)
   {
      /* need static area for name */
      strcpy (net_hostname, hostname);
      netif->hostname = net_hostname;
   }

   return 0;
}

int _netcfg_cmd (int argc, char * argv[])
{
   int i;
   const char * hostname = NULL;
   const char * local_ip = NULL;
   const char * netmask = NULL;
   const char * gw = NULL;
   const char * dhcp = NULL;

   if (netif_default == NULL)
   {
      printf ("network interface not initialized, please insert network cable\n");
      return 0;
   }

   if (argc < 2)
   {
      show_lwip_netconfig (netif_default);
      return 0;
   }

   for (i = 1; i < argc; i++)
   {
      if (i + 1 >= argc)
      {
         printf ("missing value for option: %s\n", argv[i]);
         return -1;
      }

      if (strcmp (argv[i], "hostname") == 0)
      {
         hostname = argv[++i];
      }
      else if (strcmp (argv[i], "ip") == 0)
      {
         local_ip = argv[++i];
      }
      else if (strcmp (argv[i], "mask") == 0)
      {
         netmask = argv[++i];
      }
      else if (strcmp (argv[i], "gw") == 0)
      {
         gw = argv[++i];
      }
      else if (strcmp (argv[i], "dhcp") == 0)
      {
         dhcp = argv[++i];
      }
      else
      {
         printf ("netcfg : unknown option: %s\n", argv[i]);
         return -1;
      }
   }

   if (dhcp)
   {
      if (strncmp (dhcp, "on", 2) == 0)
      {
         dhcp_set (netif_default, true);
      }
      else
      {
         dhcp_set (netif_default, false);
      }
   }
   else
   {
      netcfg_set (netif_default, local_ip, netmask, gw, hostname);
   }

   return 0;
}

const shell_cmd_t netcfg_cmd = {
   .cmd = _netcfg_cmd,
   .name = "netcfg",
   .help_short = "configure network parameters",
   .help_long = "\nnetcfg\n"
                "                           -- show current config\n"
                "   ip <addr>               -- set local ip addr\n"
                "   mask <mask>             -- set netmask\n"
                "   gw <addr>               -- set gateway addr\n"
                "   hostname <name>         -- set local hostname\n"
                "   dhcp <on/off>           -- enable / disable dhcp\n"
                "\n"
                "Example : \n"
                "   netcfg ip 10.10.0.25 mask 255.255.255.0 gw 10.10.0.1\n"};

SHELL_CMD (netcfg_cmd);
