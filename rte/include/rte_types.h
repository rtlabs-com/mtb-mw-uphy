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

#ifndef RTE_TYPES_H
#define RTE_TYPES_H

#include <stdint.h>

/* a structure used in the select() system call for monitoring multiple file
   descriptors defined here as opaque type to be cast to the correct type when
   implemented onto target system typically fd_set */
typedef struct rte_fd_set rte_fd_set_t;

/* data type used to represent file mode permissions */
typedef uint32_t rte_mode_t;

/* internal type used to represent timeouts */
typedef struct rte_timeval
{
   int32_t tv_sec;
   int32_t tv_usec;
} rte_timeval_t;

/* prevents the SIGPIPE signal from being sent to the process if the other end
 * of a socket is closed */
#define RTE_MSG_NOSIGNAL 0x20

#endif
