/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2020 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#include "pnal.h"
#include "pnal_snmp.h"

#include "mib/mib2_system.h"
#include "mib/lldp-mib.h"
#include "mib/lldp-ext-pno-mib.h"
#include "mib/lldp-ext-dot3-mib.h"

#include <lwip/apps/snmp.h>
#include <lwip/apps/snmp_mib2.h>
#include <lwip/apps/snmp_threadsync.h>

#include <lwip/netif.h>
#include <lwip/snmp.h>

/* Global variable for SNMP server state */
pnal_snmp_t pnal_snmp;

/* Configure standard SNMP variables (MIB-II)
 *
 * NOTE: This uses a patched version of lwip where callback functions are
 * called when the MIB-II system variables are read or written.
 */
static void pnal_snmp_configure_mib2 (void)
{
   static const struct snmp_obj_id enterprise_oid = {
      .len = SNMP_DEVICE_ENTERPRISE_OID_LEN,
      .id = {1, 3, 6, 1, 4, 1, 24686}, /* Profinet */
   };

   snmp_threadsync_init (&snmp_mib2_lwip_locks, snmp_mib2_lwip_synchronizer);
   snmp_set_device_enterprise_oid (&enterprise_oid);
   snmp_mib2_system_set_callbacks (
      mib2_system_get_value,
      mib2_system_test_set_value,
      mib2_system_set_value);
}

int pnal_snmp_init (rte_snmp_cfg_t * snmp_cfg)
{
   static const struct snmp_mib * mibs[] = {
      &mib2,
      &lldpmib,
      &lldpxpnomib,
      &lldpxdot3mib,
   };

   /* Ensure config is complete */
   if (RTE_SNMP_CHECK_CFG (snmp_cfg) == 0)
   {
      return -1;
   }

   /* Store reference to Profinet device instance */
   pnal_snmp.snmp_cfg = snmp_cfg;

   /* Workaround for ethernet driver not setting the link_type */
   netif_default->link_type = snmp_ifType_ethernet_csmacd;

   pnal_snmp_configure_mib2();
   snmp_set_mibs (mibs, LWIP_ARRAYSIZE (mibs));

   /* Start the SNMP server task */
   snmp_init();

   /* Success */
   return 0;
}
