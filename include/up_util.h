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
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

/**
 * @file
 * @brief Features useful for demonstration applications
 *
 * Contains for example functions for reading files with cyclic data
 * and to trigger alarms.
 */

#ifndef UP_UTIL_H
#define UP_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "up_api.h"
#include <stdint.h>

typedef struct up_util up_util_t;

/**
 * Initialise a static variable containing pointers to the
 * device configuration, the U-Phy instance and variables.
 * Set the status of all input signals to OK.
 *
 *
 * Intended for use with demonstration applications
 *
 * @param device     Device configuration
 * @param up         U-Phy instance
 * @param up_vars    Variables for U-Phy
 * @return 0 on success, -1 on error.
 */
int up_util_init (up_device_t * device, up_t * up, up_signal_info_t * up_vars);

/**
 * Get the pointer to the up_util instance
 *
 * Will trigger an assertion if the pointer is NULL.
 *
 * @return pointer to the up_util instance
 */
up_util_t * up_util_get_instance (void);

/**
 * Execute commands from a file.
 *
 * Use one command per line.
 *
 * @param file_name     File name
 * @return 0 on success, -1 on error.
 */
int up_util_poll_cmd_file (const char * file_name);

/**
 * Save current values to a human-readable file
 *
 * The file will contain input and output signal values,
 * and parameters
 *
 * @param file_name     File name
 */
void up_util_write_status_file (const char * file_name);

/**
 * Generate a template input file
 *
 * Intended for use with \a up_util_read_input_file()
 *
 * @param file_name     File name
 */
void up_util_write_input_file (const char * file_name);

/**
 * Read sample application input file and update
 * device signal data accordingly.
 *
 * Expected format: "ix name value status"
 * where:
 *   - ix      index in up_vars
 *   - name    ignored
 *   - value   signal value
 *   - status  "OK" if data is valid.
 *             Other value means that signal data is invalid
 *
 * Sample lines:
 *   0 "Input 8 bits" 0 OK
 *   2 "Input 8 bits" 0 OK
 *
 * @param file_name     File name
 */
void up_util_read_input_file (const char * file_name);

/**
 * Determine if timeout has expired
 *
 * This function returns true if the time elapsed between @a previous
 * and @a current is greater than @a timeout, even if @a current has
 * overflown and is smaller than @a previous.
 *
 * @param current       current timestamp
 * @param previous      previous timestamp
 * @param timeout       timeout
 * @return true if timeout has expired, false otherwise
 */
static inline int up_is_expired (uint32_t current, uint32_t previous, uint32_t timeout)
{
   uint32_t delta = current - previous;
   return delta >= timeout;
}

#ifdef __cplusplus
}
#endif

#endif /* UP_UTIL_H */
