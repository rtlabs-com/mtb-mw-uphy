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

#ifndef RTE_SNMP_H
#define RTE_SNMP_H

#include "rte_lldp.h"

/* Including termination. Standard says 255 (without termination). */
#define RTE_SNMP_SYSTEM_NAME_MAX_SIZE 256

/**
 * System name (sysName).
 *
 * "An administratively-assigned name for this managed
 * node. By convention, this is the node's fully-qualified
 * domain name. If the name is unknown, the value is
 * the zero-length string."
 * - IETF RFC 3418 (SNMP MIB-II).
 *
 * Is the same as LLDP's "System Name".
 * See PN-Topology Annex A.
 *
 * This is a writable variable. As such it is be in persistent memory.
 * Only writable variables (using SNMP Set) need to be stored
 * in persistent memory.
 * See IEC CDV 61158-5-10 (PN-AL-Services) ch. 7.3.3.3.6.2: "Persistency".
 */
typedef struct rte_snmp_system_name
{
   char string[RTE_SNMP_SYSTEM_NAME_MAX_SIZE]; /* Terminated string */
} rte_snmp_system_name_t;

/**
 * System contact (sysContact).
 *
 * "The textual identification of the contact person
 * for this managed node, together with information
 * on how to contact this person. If no contact information is
 * known, the value is the zero-length string."
 * - IETF RFC 3418 (SNMP MIB-II).
 *
 * The value is supplied by network manager. By default, it is
 * the zero-length string.
 *
 * An extra byte is added as to ensure null-termination.
 *
 * This is a writable variable. As such, it is stored in persistent memory.
 * Only writable variables (using SNMP Set) need to be stored
 * in persistent memory.
 * See IEC CDV 61158-5-10 (PN-AL-Services) ch. 7.3.3.3.6.2: "Persistency".
 */
typedef struct rte_snmp_system_contact
{
   char string[255 + 1]; /* Terminated */
} rte_snmp_system_contact_t;

/**
 * System description (sysDescr).
 *
 * "A textual description of the entity. This value
 * should include the full name and version
 * identification of the system's hardware type,
 * software operating-system, and networking
 * software. It is mandatory that this only contain
 * printable ASCII characters."
 * - IETF RFC 3418 (SNMP MIB-II) ch. 6 "Definitions".
 *
 * Note that MIB-II's sysDescr should have the same value as LLDP's
 * lldpLocSysDesc and (preferably) as Profinet's SystemIdentification.
 * The Chassis ID may also be encoded as SystemIdentification.
 * See IEEE 802.1AB-2005 (LLDPv1) ch. 12.2 "LLDP MIB module" and
 * IEC CDV 61158-6-10 (PN-AL-Protocol) ch. 4.16.4 "IETF RFC 1213-MIB" and
 * ch. 4.11.3.18.1 "Coding of the field LLDP_ChassisID".
 *
 * Note:
 * An extra byte is included as to ensure null-termination.
 */
typedef struct rte_snmp_system_description
{
   char string[RTE_LLDP_CHASSIS_ID_MAX_SIZE]; /** Terminated string */
} rte_snmp_system_description_t;

/**
 * System location (sysLocation).
 *
 * "The physical location of this node (e.g.,
 * 'telephone closet, 3rd floor'). If the location is unknown,
 *  the value is the zero-length string."
 * - IETF RFC 3418 (SNMP MIB-II).
 *
 * The value is supplied by network manager. By default, it is a string with
 * 22 space (' ') characters.
 *
 * The first 22 bytes should have the same value as "IM_Tag_Location" in I&M1.
 * See PN-Topology ch. 11.5.2: "Consistency".
 *
 * This is a writable variable. As such, it is stored in persistent memory.
 * Only writable variables (using SNMP Set) need to be stored
 * in persistent memory.
 * See IEC CDV 61158-5-10 (PN-AL-Services) ch. 7.3.3.3.6.2: "Persistency".
 */
typedef struct rte_snmp_system_location
{
   char string[255 + 1]; /* Terminated string */
} rte_snmp_system_location_t;

/**
 * Encoded management address.
 *
 * Contains similar information as pf_lldp_management_address_t, but the
 * fields have been encoded so they may be immediately placed in SNMP response.
 */
typedef struct rte_snmp_management_address
{
   /** First byte is size of actual address */
   uint8_t value[1 + 31];

   /** 1 for IPv4 */
   uint8_t subtype;

   /** 5 for IPv4 */
   size_t len;
} rte_snmp_management_address_t;

/**
 * Encoded link status.
 *
 * Contains the same information as pf_lldp_link_status_t, but the fields
 * have been encoded so they may be immediately placed in SNMP response.
 */
typedef struct rte_snmp_link_status
{
   /** 1 if true, 2 if false */
   int32_t auto_neg_supported;

   /** 1 if true, 2 if false */
   int32_t auto_neg_enabled;

   /** OCTET STRING encoding of BITS */
   uint8_t auto_neg_advertised_cap[2];

   int32_t oper_mau_type;
} rte_snmp_link_status_t;

/**
 * Measured signal delays in nanoseconds
 *
 * If a signal delay was not measured, its value is zero.
 *
 * See IEC CDV 61158-6-10 (PN-AL-Protocol) Annex U: "LLDP EXT MIB", fields
 * lldpXPnoLocLPDValue / lldpXPnoRemLPDValue,
 * lldpXPnoLocPortTxDValue / lldpXPnoRemPortTxDValue,
 * lldpXPnoLocPortRxDValue / lldpXPnoRemPortRxDValue.
 *
 * See also pf_lldp_signal_delay_t.
 */
typedef struct rte_snmp_signal_delay
{
   uint32_t port_tx_delay_ns;
   uint32_t port_rx_delay_ns;
   uint32_t line_propagation_delay_ns;
} rte_snmp_signal_delay_t;

/** SNMP configuration */
typedef struct rte_snmp_cfg
{
   void * user_data;
   void (*get_system_description) (
      rte_snmp_system_description_t * description,
      void * user_data);
   void (*get_system_contact) (
      rte_snmp_system_contact_t * contact,
      void * user_data);
   int (*set_system_contact) (
      rte_snmp_system_contact_t const * contact,
      void * user_data);
   void (*get_system_name) (rte_snmp_system_name_t * name, void * user_data);
   int (*set_system_name) (rte_snmp_system_name_t const * name, void * user_data);
   void (*get_system_location) (
      rte_snmp_system_location_t * location,
      void * user_data);
   int (*set_system_location) (
      rte_snmp_system_location_t * location,
      void * user_data);
   /* LLDP */
   size_t port_iterator_size;
   void (*init_port_iterator) (void * iterator, void * user_data);
   int (*get_next_port) (void * iterator);
   void (*get_port_list) (rte_lldp_port_list_t * list, void * user_data);
   int (*get_peer_timestamp) (
      int loc_port_num,
      uint32_t * timestamp_10ms,
      void * user_data);
   void (*get_chassis_id) (rte_lldp_chassis_id_t * chassis_id, void * user_data);
   int (*get_peer_chassis_id) (
      int loc_port_num,
      rte_lldp_chassis_id_t * chassis_id,
      void * user_data);
   void (*get_port_id) (
      int loc_port_num,
      rte_lldp_port_id_t * port_id,
      void * user_data);
   int (*get_peer_port_id) (
      int loc_port_num,
      rte_lldp_port_id_t * port_id,
      void * user_data);
   void (*get_port_description) (
      int loc_port_num,
      rte_lldp_port_description_t * port_description,
      void * user_data);
   int (*get_peer_port_description) (
      int loc_port_num,
      rte_lldp_port_description_t * port_description,
      void * user_data);
   void (*get_management_address) (
      rte_snmp_management_address_t * man_address,
      void * user_data);
   int (*get_peer_management_address) (
      int loc_port_num,
      rte_snmp_management_address_t * man_address,
      void * user_data);
   void (*get_management_port_index) (
      rte_lldp_interface_number_t * port_index,
      void * user_data);
   int (*get_peer_management_port_index) (
      int loc_port_num,
      rte_lldp_interface_number_t * port_index,
      void * user_data);
   void (*get_station_name) (
      rte_lldp_station_name_t * station_name,
      void * user_data);
   int (*get_peer_station_name) (
      int loc_port_num,
      rte_lldp_station_name_t * station_name,
      void * user_data);
   void (*get_signal_delays) (
      int loc_port_num,
      rte_snmp_signal_delay_t * delays,
      void * user_data);
   int (*get_peer_signal_delays) (
      int loc_port_num,
      rte_snmp_signal_delay_t * delays,
      void * user_data);
   void (*get_link_status) (
      int loc_port_num,
      rte_snmp_link_status_t * link_status,
      void * user_data);
   int (*get_peer_link_status) (
      int loc_port_num,
      rte_snmp_link_status_t * link_status,
      void * user_data);
} rte_snmp_cfg_t;

/* check that all fields in rte_snmp_cfg_t have been set */
#define RTE_SNMP_CHECK_CFG(cfg) \
    ((cfg) != NULL && \
    (cfg)->get_system_description != NULL && \
    (cfg)->get_system_contact != NULL && \
    (cfg)->set_system_contact != NULL && \
    (cfg)->get_system_name != NULL && \
    (cfg)->set_system_name != NULL && \
    (cfg)->get_system_location != NULL && \
    (cfg)->set_system_location != NULL && \
    (cfg)->port_iterator_size != 0 && \
    (cfg)->init_port_iterator != NULL && \
    (cfg)->get_next_port != NULL && \
    (cfg)->get_port_list != NULL && \
    (cfg)->get_peer_timestamp != NULL && \
    (cfg)->get_chassis_id != NULL && \
    (cfg)->get_peer_chassis_id != NULL && \
    (cfg)->get_port_id != NULL && \
    (cfg)->get_peer_port_id != NULL && \
    (cfg)->get_port_description != NULL && \
    (cfg)->get_peer_port_description != NULL && \
    (cfg)->get_management_address != NULL && \
    (cfg)->get_peer_management_address != NULL && \
    (cfg)->get_management_port_index != NULL && \
    (cfg)->get_peer_management_port_index != NULL && \
    (cfg)->get_station_name != NULL && \
    (cfg)->get_peer_station_name != NULL && \
    (cfg)->get_signal_delays != NULL && \
    (cfg)->get_peer_signal_delays != NULL && \
    (cfg)->get_link_status != NULL && \
    (cfg)->get_peer_link_status != NULL)

/* RTE SNMP callback wrappers */

#define rte_snmp_get_system_description(cfg, desc)                             \
   (cfg)->get_system_description (desc, (cfg)->user_data)

#define rte_snmp_get_system_contact(cfg, contact)                              \
   (cfg)->get_system_contact (contact, (cfg)->user_data)

#define rte_snmp_set_system_contact(cfg, contact)                              \
   (cfg)->set_system_contact (contact, (cfg)->user_data)

#define rte_snmp_get_system_name(cfg, name)                                    \
   (cfg)->get_system_name (name, (cfg)->user_data)

#define rte_snmp_set_system_name(cfg, name)                                    \
   (cfg)->set_system_name (name, (cfg)->user_data)

#define rte_snmp_get_system_location(cfg, location)                            \
   (cfg)->get_system_location (location, (cfg)->user_data)

#define rte_snmp_set_system_location(cfg, location)                            \
   (cfg)->set_system_location (location, (cfg)->user_data)

#define rte_snmp_init_port_iterator(cfg, iterator)                             \
   (cfg)->init_port_iterator (iterator, (cfg)->user_data)

#define rte_snmp_get_next_port(cfg, iterator) (cfg)->get_next_port (iterator)

#define rte_snmp_get_port_list(cfg, list)                                      \
   (cfg)->get_port_list (list, (cfg)->user_data)

#define rte_snmp_get_peer_timestamp(cfg, loc_port_num, timestamp)              \
   (cfg)->get_peer_timestamp (loc_port_num, timestamp, (cfg)->user_data)

#define rte_snmp_get_chassis_id(cfg, chassis_id)                               \
   (cfg)->get_chassis_id (chassis_id, (cfg)->user_data)

#define rte_snmp_get_peer_chassis_id(cfg, loc_port_num, chassis_id)            \
   (cfg)->get_peer_chassis_id (loc_port_num, chassis_id, (cfg)->user_data)

#define rte_snmp_get_port_id(cfg, loc_port_num, port_id)                       \
   (cfg)->get_port_id (loc_port_num, port_id, (cfg)->user_data)

#define rte_snmp_get_peer_port_id(cfg, loc_port_num, port_id)                  \
   (cfg)->get_peer_port_id (loc_port_num, port_id, (cfg)->user_data)

#define rte_snmp_get_port_description(cfg, loc_port_num, port_desc)            \
   (cfg)->get_port_description (loc_port_num, port_desc, (cfg)->user_data)

#define rte_snmp_get_peer_port_description(cfg, loc_port_num, port_desc)       \
   (cfg)->get_peer_port_description (loc_port_num, port_desc, (cfg)->user_data)

#define rte_snmp_get_management_address(cfg, man_address)                      \
   (cfg)->get_management_address (man_address, (cfg)->user_data)

#define rte_snmp_get_peer_management_address(cfg, loc_port_num, man_address)   \
   (cfg)->get_peer_management_address (loc_port_num, man_address, (cfg)->user_data)

#define rte_snmp_get_management_port_index(cfg, port_index)                    \
   (cfg)->get_management_port_index (port_index, (cfg)->user_data)

#define rte_snmp_get_peer_management_port_index(cfg, loc_port_num, port_index) \
   (cfg)->get_peer_management_port_index (                                     \
      loc_port_num,                                                            \
      port_index,                                                              \
      (cfg)->user_data)

#define rte_snmp_get_station_name(cfg, station_name)                           \
   (cfg)->get_station_name (station_name, (cfg)->user_data)

#define rte_snmp_get_peer_station_name(cfg, loc_port_num, station_name)        \
   (cfg)->get_peer_station_name (loc_port_num, station_name, (cfg)->user_data)

#define rte_snmp_get_signal_delays(cfg, loc_port_num, delays)                  \
   (cfg)->get_signal_delays (loc_port_num, delays, (cfg)->user_data)

#define rte_snmp_get_peer_signal_delays(cfg, loc_port_num, delays)             \
   (cfg)->get_peer_signal_delays (loc_port_num, delays, (cfg)->user_data)

#define rte_snmp_get_link_status(cfg, loc_port_num, link_status)               \
   (cfg)->get_link_status (loc_port_num, link_status, (cfg)->user_data)

#define rte_snmp_get_peer_link_status(cfg, loc_port_num, link_status)          \
   (cfg)->get_peer_link_status (loc_port_num, link_status, (cfg)->user_data)

/**
 * Configure SNMP server.
 *
 * This function configures a platform-specific SNMP server as to
 * enable a connected SNMP client to read variables from the p-net stack,
 * as well as to write some variables to it.
 *
 * @param snmp_cfg         In:    SNMP configuration
 * @return  0 if the operation succeeded.
 *         -1 if an error occurred.
 */

/* fixme -- rename all pnal references to rte */
int pnal_snmp_init (rte_snmp_cfg_t * snmp_cfg);

#endif /* RTE_SNMP_H */
