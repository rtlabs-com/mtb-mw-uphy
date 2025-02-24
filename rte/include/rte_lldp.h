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

#ifndef RTE_LLDP_H
#define RTE_LLDP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Including termination. Standard says 240 (without termination). */
#define RTE_LLDP_CHASSIS_ID_MAX_SIZE 241

/* Including termination. Standard says 14 (without termination). */
#define RTE_LLDP_PORT_NAME_MAX_SIZE 15

/* Including termination. Standard says 255 (without termination). */
#define RTE_LLDP_PORT_DESCRIPTION_MAX_SIZE 256

/* Including termination. Standard says 240 (without termination). */
#define RTE_LLDP_STATION_NAME_MAX_SIZE 241

#define RTE_LLDP_PORT_ID_MAX_SIZE                                              \
   (RTE_LLDP_STATION_NAME_MAX_SIZE + RTE_LLDP_PORT_NAME_MAX_SIZE)

/**
 * Port list
 *
 * "Each octet within this value specifies a set of eight ports,
 * with the first octet specifying ports 1 through 8, the second
 * octet specifying ports 9 through 16, etc. Within each octet,
 * the most significant bit represents the lowest numbered port,
 * and the least significant bit represents the highest numbered
 * port. Thus, each port of the system is represented by a
 * single bit within the value of this object. If that bit has
 * a value of '1' then that port is included in the set of ports;
 * the port is not included if its bit has a value of '0'."
 * - IEEE 802.1AB (LLDP) ch. 12.2 "LLDP MIB module" (lldpPortList).
 *
 * Se also section about lldpConfigManAddrTable in PN-Topology.
 */
typedef struct rte_lldp_port_list_t
{
   uint8_t ports[2];
} rte_lldp_port_list_t;

/**
 * Chassis ID
 *
 * See IEEE 802.1AB-2005 (LLDPv1) ch. 9.5.2.3 "chassis ID".
 */
typedef struct rte_lldp_chassis_id
{
   /** Terminated string
       Typically this field is a string (PF_LLDP_SUBTYPE_LOCALLY_ASSIGNED)
       If the subtype indicates a MAC address, the content is the
       raw MAC address bytes (and trailing null termination).
       Similarly for other subtypes */
   char string[RTE_LLDP_CHASSIS_ID_MAX_SIZE];

   /** PF_LLDP_SUBTYPE_xxx */
   uint8_t subtype;

   bool is_valid;

   size_t len;
} rte_lldp_chassis_id_t;

/**
 * Port ID
 *
 * See IEEE 802.1AB-2005 (LLDPv1) ch. 9.5.3 "Port ID TLV".
 */
typedef struct rte_lldp_port_id
{
   /** Terminated string
       Typically this field is a string (PF_LLDP_SUBTYPE_LOCALLY_ASSIGNED)
       If the subtype indicates a MAC address, the content is the
       raw MAC address bytes (and trailing null termination).
       Similarly for other subtypes */
   char string[RTE_LLDP_PORT_ID_MAX_SIZE];

   /** PF_LLDP_SUBTYPE_xxx */
   uint8_t subtype;

   bool is_valid;

   size_t len;
} rte_lldp_port_id_t;

/**
 * Port description
 *
 * "The port description field shall contain an alphanumeric string
 * that indicates the port's description. If RFC 2863
 * is implemented, the ifDescr object should be used for this field."
 * - IEEE 802.1AB (LLDP) ch. 9.5.5.2 "port description".
 */
typedef struct rte_lldp_port_description
{
   char string[RTE_LLDP_PORT_DESCRIPTION_MAX_SIZE]; /* Terminated string */
   bool is_valid;
   size_t len;
} rte_lldp_port_description_t;

/**
 * Interface number
 *
 * "The interface number field shall contain the assigned number within
 * the system that identifies the specific interface associated with this
 * management address. If the value of the interface subtype is unknown,
 * this field shall be set to zero."
 * - IEEE 802.1AB-2005 (LLDPv1) ch. 9.5.9 "Management Address TLV".
 *
 * Also see PN-Topology ch. 6.5.1 "Mapping of Ports and PROFINET Interfaces
 * in LLDP MIB and MIB-II".
 */
typedef struct rte_lldp_interface_number
{
   int32_t value;
   uint8_t subtype; /* 1 = unknown, 2 = ifIndex, 3 = systemPortNumber */
} rte_lldp_interface_number_t;

/**
 * Station name
 *
 * This is the name of an interface. It is called NameOfStation in
 * the Profinet specification. It is usually a string, but may also
 * be a MAC address.
 *
 * See IEC CDV 61158-6-10 (PN-AL-Protocol) ch. 4.3.1.4.16.
 */
typedef struct rte_lldp_station_name
{
   char string[RTE_LLDP_STATION_NAME_MAX_SIZE]; /** Terminated string */
   size_t len;
} rte_lldp_station_name_t;

#endif /* RTE_LLDP_H */
