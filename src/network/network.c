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
#include "filesys.h"

/* Standard C header files */
#include <inttypes.h>
#include <stdlib.h>

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

/*******************************************************************************
 * Macros
 ********************************************************************************/

/* Maximum number of connection retries to the ethernet network */
#define MAX_ETH_RETRY_COUNT (3u)

/* static void read_mac_from_file (cy_ecm_mac_t * mac_address); */

const char * mac_file = STORAGE_ROOT "mac";

static cy_ecm_t ecm_handle = NULL;

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
         printf ("Successfully connected to Ethernet.\n");
         printf (
            "IP Address Assigned: %d.%d.%d.%d\n",
            (uint8)ip_addr.ip.v4,
            (uint8) (ip_addr.ip.v4 >> 8),
            (uint8) (ip_addr.ip.v4 >> 16),
            (uint8) (ip_addr.ip.v4 >> 24));

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
  int f = fs_open (mac_file, O_RDONLY);

  if (f < 0)
  {
     return;
  }
  n_bytes = fs_read (f, mac_address, 6);
  if (n_bytes != 6)
  {
     memset (mac_address, 0, 6);
  }
  fs_close (f);
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

int _cmd_mac (int argc, char * argv[])
{
   cy_ecm_mac_t mac_address;
   int f;
   int n_bytes;

   if (argc == 1)
   {
      f = fs_open (mac_file, O_RDONLY);
      if (f < 0)
      {
         printf ("No MAC address file. Default MAC address (00:03:19:45:00:00) "
                 "is used\n");
         return 0;
      }

      n_bytes = fs_read (f, mac_address, sizeof (mac_address));
      fs_close (f);

      if (n_bytes != sizeof (mac_address))
      {
         printf (
            "Unexpected data in MAC address file. File will be deleted.\n");
         fs_unlink (mac_file);
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

      f = fs_open (mac_file, O_WRONLY | O_CREAT);
      if (f < 0)
      {
         printf ("Error: Could not open file %s\n", mac_file);
         return 0;
      }

      n_bytes = fs_write (f, mac_address, sizeof (mac_address));
      fs_close (f);

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
