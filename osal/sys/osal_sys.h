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

#ifndef OSAL_SYS_H
#define OSAL_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>
#include <queue.h>

#define OS_MAIN extern "C" int _main

#define OS_THREAD
#define OS_MUTEX
#define OS_SEM
#define OS_EVENT
#define OS_MBOX
#define OS_TIMER

typedef struct SemaphoreHandle_t os_mutex_t;
typedef struct TaskHandle_t os_thread_t;
typedef struct SemaphoreHandle_t os_sem_t;
typedef struct EventGroupHandle_t os_event_t;
typedef struct QueueHandle_t os_mbox_t;
typedef TimerHandle_t os_tmr_t;

typedef struct os_timer
{
   os_tmr_t handle;
   void (*fn) (struct os_timer *, void * arg);
   void * arg;
   uint32_t us;
} os_timer_t;

#ifdef __cplusplus
}
#endif

#endif /* OSAL_SYS_H */
