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

#ifndef PNAL_SYS_H
#define PNAL_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

/* modus toolbox remaps htons etc transparantly */
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

#include <lwip/def.h> /* For htonl etc */
#include <lwip/pbuf.h>

#define PNAL_BUF_MAX_SIZE PBUF_POOL_BUFSIZE

/* Re-use lwIP pbuf for rt-kernel */
typedef struct pbuf pnal_buf_t;

#ifdef __cplusplus
}
#endif

#endif /* PNAL_SYS_H */
