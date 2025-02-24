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

/**
 * @file
 * @brief SNMP server for rt-kernel
 *
 * Uses (a patched version of) the SNMP server implementation supplied by lwip.
 * Access to Profinet stack is done through the platform-independent internal
 * API, pf_snmp.h. Supported MIBs:
 * - MIB-II,
 * - LLDP-MIB,
 * - LLDP-EXT-DOT3-MIB,
 * - LLDP-EXT-PNO-MIB.
 *
 * Note that the rt-kernel tree needs to be patched to support SNMP etc.
 * See supplied patch file.
 */

#ifndef PNAL_SNMP_H
#define PNAL_SNMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rte_snmp.h"
#include "rte_network.h"

/* modus toolbox remaps htons etc transparantly */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include <lwip/apps/snmp.h>

/**
 * Response buffer
 *
 * Note that this is a union, not a struct.
 */
typedef union pnal_snmp_response
{
   uint32_t u32;
   int32_t s32;
   uint8_t buffer[SNMP_MAX_VALUE_SIZE];
} pnal_snmp_response_t;

/**
 * SNMP server state
 */
typedef struct pnal_snmp
{
   /* The SNMP configuration.
    *
    * Used for accessing data in the stack, such as LLDP variables.
    */
   rte_snmp_cfg_t * snmp_cfg;

   pnal_snmp_response_t response;
} pnal_snmp_t;

/** Global variable containing SNMP server state */
extern pnal_snmp_t pnal_snmp;

#ifdef __cplusplus
}
#endif

#endif /* PNAL_SNMP_H */
