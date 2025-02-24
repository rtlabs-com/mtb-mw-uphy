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

#include "osal_log.h"
#include "rte_config.h"
#include "rte_snmp.h"
#include "pnal_snmp.h"

#include "rowindex.h"

#include <string.h>

#undef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO

static void rowindex_construct_for_local_port (
   struct snmp_obj_id * row_index,
   int port)
{
   row_index->id[0] = port; /* lldpLocPortNum */
   row_index->len = 1;
}

static void rowindex_construct_for_local_interface (
   struct snmp_obj_id * row_index)
{
   rte_snmp_management_address_t address;
   size_t i;

   rte_snmp_get_management_address (pnal_snmp.snmp_cfg, &address);

   row_index->id[0] = address.subtype; /* lldpLocManAddrSubtype */
   for (i = 0; i < address.len; i++)
   {
      row_index->id[1 + i] = address.value[i]; /* lldpLocManAddr */
   }
   row_index->len = 1 + address.len;
}

static int rowindex_construct_for_remote_device (
   struct snmp_obj_id * row_index,
   int port)
{
   uint32_t timestamp;
   int error;

   error = rte_snmp_get_peer_timestamp (pnal_snmp.snmp_cfg, port, &timestamp);

   if (error)
   {
      return error;
   }

   row_index->id[0] = timestamp; /* lldpRemTimeMark */
   row_index->id[1] = port;      /* lldpRemLocalPortNum */
   row_index->id[2] = port;      /* lldpRemIndex */
   row_index->len = 3;

   return error;
}

static int rowindex_construct_for_remote_interface (
   struct snmp_obj_id * row_index,
   int port)
{
   uint32_t timestamp;
   rte_snmp_management_address_t address;
   int error;
   size_t i;

   error = rte_snmp_get_peer_timestamp (pnal_snmp.snmp_cfg, port, &timestamp);

   if (error)
   {
      return error;
   }
   error = rte_snmp_get_peer_management_address (pnal_snmp.snmp_cfg, port, &address);

   if (error)
   {
      return error;
   }

   row_index->id[0] = timestamp;       /* lldpRemTimeMark */
   row_index->id[1] = port;            /* lldpRemLocalPortNum */
   row_index->id[2] = port;            /* lldpRemIndex */
   row_index->id[3] = address.subtype; /* lldpRemManAddrSubtype */
   for (i = 0; i < address.len; i++)
   {
      row_index->id[4 + i] = address.value[i]; /* lldpRemManAddr */
   }
   row_index->len = 4 + address.len;

   return error;
}

int rowindex_match_with_local_port (const u32_t * row_oid, u8_t row_oid_len)
{
   void * port_iterator;
   int port;

   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);

   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);

   while (port != 0)
   {
      struct snmp_obj_id port_oid;

      rowindex_construct_for_local_port (&port_oid, port);
      if (snmp_oid_equal (row_oid, row_oid_len, port_oid.id, port_oid.len))
      {
         return port;
      }

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   return 0;
}

int rowindex_update_with_next_local_port (struct snmp_obj_id * row_oid)
{
   struct snmp_next_oid_state state;
   u32_t next_oid[SNMP_MAX_OBJ_ID_LEN];
   void* port_iterator;
   int port;

   snmp_next_oid_init (
      &state,
      row_oid->id,
      row_oid->len,
      next_oid,
      SNMP_MAX_OBJ_ID_LEN);
   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);

   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);

   while (port != 0)
   {
      struct snmp_obj_id port_oid;

      rowindex_construct_for_local_port (&port_oid, port);
      snmp_next_oid_check (&state, port_oid.id, port_oid.len, (void *)port);

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   if (state.status == SNMP_NEXT_OID_STATUS_SUCCESS)
   {
      snmp_oid_assign (row_oid, state.next_oid, state.next_oid_len);
      port = (int)state.reference;
   }
   else
   {
      port = 0;
   }

   return port;
}

int rowindex_match_with_local_interface (const u32_t * row_oid, u8_t row_oid_len)
{
   struct snmp_obj_id interface_oid;
   int interface;

   rowindex_construct_for_local_interface (&interface_oid);
   if (snmp_oid_equal (row_oid, row_oid_len, interface_oid.id, interface_oid.len))
   {
      interface = 1;
   }
   else
   {
      interface = 0;
   }

   return interface;
}

snmp_err_t rowindex_update_with_next_local_interface (
   struct snmp_obj_id * row_oid)
{
   struct snmp_next_oid_state state;
   u32_t next_oid[SNMP_MAX_OBJ_ID_LEN];
   struct snmp_obj_id interface_oid;
   int interface;

   snmp_next_oid_init (
      &state,
      row_oid->id,
      row_oid->len,
      next_oid,
      SNMP_MAX_OBJ_ID_LEN);
   rowindex_construct_for_local_interface (&interface_oid);
   snmp_next_oid_check (&state, interface_oid.id, interface_oid.len, NULL);

   if (state.status == SNMP_NEXT_OID_STATUS_SUCCESS)
   {
      snmp_oid_assign (row_oid, state.next_oid, state.next_oid_len);
      interface = 1;
   }
   else
   {
      interface = 0;
   }

   return interface;
}

int rowindex_match_with_remote_device (const u32_t * row_oid, u8_t row_oid_len)
{
   void* port_iterator;
   int port;

   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);
   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   while (port != 0)
   {
      struct snmp_obj_id port_oid;
      int error;

      error = rowindex_construct_for_remote_device (&port_oid, port);
      if (
         !error &&
         snmp_oid_equal (row_oid, row_oid_len, port_oid.id, port_oid.len))
      {
         return port;
      }

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   return 0;
}

int rowindex_update_with_next_remote_device (struct snmp_obj_id * row_oid)
{
   struct snmp_next_oid_state state;
   u32_t next_oid[SNMP_MAX_OBJ_ID_LEN];
   void* port_iterator;
   int port;

   snmp_next_oid_init (
      &state,
      row_oid->id,
      row_oid->len,
      next_oid,
      SNMP_MAX_OBJ_ID_LEN);
   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);
   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   while (port != 0)
   {
      int error;
      struct snmp_obj_id port_oid;

      error = rowindex_construct_for_remote_device (&port_oid, port);
      if (!error)
      {
         snmp_next_oid_check (&state, port_oid.id, port_oid.len, (void *)port);
      }

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   if (state.status == SNMP_NEXT_OID_STATUS_SUCCESS)
   {
      snmp_oid_assign (row_oid, state.next_oid, state.next_oid_len);
      port = (int)state.reference;
   }
   else
   {
      port = 0;
   }

   return port;
}

int rowindex_match_with_remote_interface (
   const u32_t * row_oid,
   u8_t row_oid_len)
{
   void * port_iterator;
   int port;

   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);
   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   while (port != 0)
   {
      int error;
      struct snmp_obj_id port_oid;

      error = rowindex_construct_for_remote_interface (&port_oid, port);
      if (
         !error &&
         snmp_oid_equal (row_oid, row_oid_len, port_oid.id, port_oid.len))
      {
         return port;
      }

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   return 0;
}

int rowindex_update_with_next_remote_interface (struct snmp_obj_id * row_oid)
{
   struct snmp_next_oid_state state;
   u32_t next_oid[SNMP_MAX_OBJ_ID_LEN];
   void * port_iterator;
   int port;

   snmp_next_oid_init (
      &state,
      row_oid->id,
      row_oid->len,
      next_oid,
      SNMP_MAX_OBJ_ID_LEN);
   rte_snmp_init_port_iterator (pnal_snmp.snmp_cfg, &port_iterator);
   port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   while (port != 0)
   {
      int error;
      struct snmp_obj_id port_oid;

      error = rowindex_construct_for_remote_interface (&port_oid, port);
      if (!error)
      {
         snmp_next_oid_check (&state, port_oid.id, port_oid.len, (void *)port);
      }

      port = rte_snmp_get_next_port (pnal_snmp.snmp_cfg, &port_iterator);
   }

   if (state.status == SNMP_NEXT_OID_STATUS_SUCCESS)
   {
      snmp_oid_assign (row_oid, state.next_oid, state.next_oid_len);
      port = (int)state.reference;
   }
   else
   {
      port = 0;
   }

   return port;
}
