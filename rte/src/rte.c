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

#include "pnal.h"

#if LWIP_IPV6
#error "no ipv6 supported"
#endif

/* modus toolbox remaps htons etc transparantly */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include <lwip/netif.h>
#include <lwip/sys.h>

void * pnal_buf_payload (pnal_buf_t const * buf)
{
   return ((struct pbuf *)buf)->payload;
}

uint16_t * pnal_buf_len_ref (pnal_buf_t * buf)
{
   return &((struct pbuf *)buf)->len;
}
