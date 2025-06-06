/******************************************************************************
 * File Name:   ring_buffer.c
 *
 * Description: This is the source code for the XMC MCU: UART Shell Example
 *              for ModusToolbox. This file implements a ringbuffer.
 *              Used to receive and send data from/to UART.
 *
 * Related Document: See README.md
 *
 ******************************************************************************
 *
 * Copyright (c) 2015-2024, Infineon Technologies AG
 * All rights reserved.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "ring_buffer.h"

/*******************************************************************************
 * Function Name: ring_buffer_is_empty
 ********************************************************************************
 * Summary:
 * Check if ringbuffer is empty
 *
 * Parameters:
 *  ring_buffer_t *const rb: Pointer to ring buffer
 *
 * Return:
 *  bool: RING_BUFFER_EMPTY     - if ring buffer is empty
 *        RING_BUFFER_NOT_EMPTY - if ring buffer is not empty
 *
 *******************************************************************************/
static bool ring_buffer_is_empty (ring_buffer_t * const rb)
{
   uint32_t head = rb->head;
   uint32_t tail = rb->tail;
   return (head == tail);
}

/*******************************************************************************
 * Function Name: ring_buffer_is_full
 ********************************************************************************
 * Summary:
 * Check if ringbuffer is full
 *
 * Parameters:
 *  ring_buffer_t *const rb: Pointer to ring buffer
 *
 * Return:
 *  bool: RING_BUFFER_FULL      - if ring buffer is full
 *        RING_BUFFER_NOT_FULL  - if ring buffer is not full
 *
 *******************************************************************************/
static bool ring_buffer_is_full (ring_buffer_t * const rb)
{
   uint32_t head = rb->head;
   uint32_t tail = rb->tail;
   return (((head + 1) % rb->len) == tail);
}

/*******************************************************************************
 * Function Name: ring_buffer_avail
 ********************************************************************************
 * Summary:
 * Check for available space inside ring buffer
 *
 * Parameters:
 *  ring_buffer_t *const rb: Pointer to ring buffer
 *
 * Return:
 *  uint32_t: available space
 *
 *******************************************************************************/
uint32_t ring_buffer_avail (ring_buffer_t * const rb)
{
   uint32_t head = rb->head;
   uint32_t tail = rb->tail;
   return (head - tail) % rb->len;
}

/*******************************************************************************
 * Function Name: ring_buffer_put
 ********************************************************************************
 * Summary:
 * Push one character item into ringbuffer
 *
 * Parameters:
 *  ring_buffer_t *const rb: Pointer to ring buffer
 *  uint8_t c: Character to put into ring buffer
 *
 * Return:
 *  int32_t: RING_BUFFER_OK         - on success
 *           RING_BUFFER_FULL_ERROR - on buffer full error
 *
 *******************************************************************************/
int32_t ring_buffer_put (ring_buffer_t * const rb, uint8_t c)
{
   if (ring_buffer_is_full (rb))
   {
      return RING_BUFFER_FULL_ERROR;
   }

   rb->buffer[rb->head] = c;
   rb->head = (rb->head + 1) % rb->len;

   return RING_BUFFER_OK;
}

/*******************************************************************************
 * Function Name: ring_buffer_get
 ********************************************************************************
 * Summary:
 * Pop one character item from ringbuffer
 *
 * Parameters:
 *  ring_buffer_t *const rb: Pointer to ring buffer
 *  uint8_t *const c: Pointer of character popped from ringbuffer
 *
 * Return:
 *  int32_t: RING_BUFFER_OK          - on success
 *           RING_BUFFER_EMPTY_ERROR - on buffer empty error
 *
 *******************************************************************************/
int32_t ring_buffer_get (ring_buffer_t * const rb, uint8_t * const c)
{
   if (ring_buffer_is_empty (rb))
   {
      return RING_BUFFER_EMPTY_ERROR;
   }

   *c = rb->buffer[rb->tail];
   rb->tail = (rb->tail + 1) % rb->len;

   return RING_BUFFER_OK;
}

/* [] END OF FILE */
