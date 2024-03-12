/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2022 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifndef CORE_H
#define CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "up_types.h"

#include <stdint.h>

#define UP_BITS_TO_BYTES(n) (((n) + 7) >> 3)

#define UP_CORE_MSG_QUEUE_SIZE 10

typedef struct up_param_frame
{
   uint16_t n_bytes;
   uint8_t data[];
} up_param_frame_t;

typedef struct up_adapter
{
   /** Uphy frame sent from host to core. Inputs to PLC */
   uint8_t * tx_frame;

   /** Uphy frame sent from core to host. Outputs from PLC */
   uint8_t * rx_frame;

   /**
    * Error callback. Used to notify the adapter that an error
    * has occurred in the Core.
    */
   void (*error_ind_cb) (up_error_t error);
} up_adapter_t;

typedef struct core core_t;

/**
 * Calculate frame information from
 * device data model.
 *
 * @param device           Device configuration
 * @param tx_frame_info    Tx / input frame info
 * @param rx_frame_info    Rx / output frame info
 */
void up_core_init_frame_info (
   up_device_t * device,
   up_frame_info_t * tx_frame_info,
   up_frame_info_t * rx_frame_info);

/**
 * Summarize the data sizes in a signal array
 *
 * @param n_signals     Number of signals in the array
 * @param signal        Array of signals
 * @return the total size, in bytes.
 */
uint16_t up_core_summarize_datasizes (uint32_t n_signals, up_signal_t * signals);

/**
 * Print out frame signals and sizes
 *
 * @param n_slots          Number of slots
 * @param slots            Array of slots
 */
void up_core_show_frame_bytes (uint32_t n_slots, const up_slot_t * slots);

/**
 * Get offset to status for first signal in slot.
 *
 * @param device        Device configuration
 * @param slot_ix       Slot index
 * @param is_input      Set to true for input / tx frame
 *
 * @return Calculated offset
 */
uint16_t up_core_get_status_offset_by_slot_ix (
   up_device_t * device,
   uint16_t slot_id,
   bool is_input);

/**
 * Allocate a \a up_param_frame_t large enough to hold all parameters.
 * If the device does not have any parameters, NULL is returned.
 *
 * @param device     Device configuration
 * @return a pointer to allocated parameter frame
 *         NULL if device has no parameters
 */
up_param_frame_t * up_core_alloc_params_area (up_device_t * device);

/**
 * Initialise all parameters (in core) with their default values
 *
 * The "parameter frame" is the local storage of parameter values in core.
 *
 * @param param_frame   Parameter frame to be initialised
 * @param device        Device (with parameters incl. default values)
 * @return 0 on success, and -1 on error.
 */
int up_core_init_params_default (
   up_param_frame_t * param_frame,
   up_device_t * device);

/**
 * Indication to the host that outputdata from the PLC is available.
 *
 * Also update watchdog and monitor connection status.
 */
void up_avail (void);

/**
 * Indication to the host that it is time to prepare inputdata to the PLC.
 *
 * Also update watchdog and monitor connection status.
 */
void up_sync (void);

/**
 * Send message to host
 * The message is added to the message queue.
 *
 * @param message  Message to send. Includes message id and optional parameters
 */
void up_send (up_message_t * message);

/**
 * Send error message to host
 * This function creates a message with id = UP_MESSAGE_ID_ERROR
 * and adds the error code as a event parameter.
 *
 * @param error_code  Error code to set
 */
void up_send_error (up_error_t error_code);

/**
 * Get string representation of message id
 *
 * @param id Message id
 * @return Id string
 */
const char * up_message_id_to_str (up_message_id_t id);

/**
 * Get string representation of error code
 *
 * @param error_code Error code
 * @return Error code string
 */
const char * error_code_to_str (up_error_t error_code);

/**
 * Return the bustype literal
 *
 * @param bus_type   Bus type
 * @return Pointer to string literal
 */
const char * bustype_to_str (up_bustype_t bus_type);

/**
 * Set one or several status flags.
 * Supported status flags are defined in up_core_status_t
 * If the status is changed a status indication event is
 * triggered.
 *
 * @param status Status flags to set
 */
void up_core_set_status (uint32_t status);

/**
 * Clear one or several status flags.
 * Supported status flags are defined in up_core_status_t
 * If the status is changed a status indication event is
 * triggered.
 *
 * @param status Status flags to clear
 */
void up_core_clear_status (uint32_t status);

/**
 * Write parameter data (received from PLC) to the core, for sending to host.
 *
 * Stores data locally in core.
 * Sends a message to the host via \a upi_param_write_ind().
 *
 * @param slot_ix          Slot index
 * @param param_ix         Parameter index in U-phy data model (not the index
 *                         used by Profinet)
 * @param data_bitlength   Length of parameterdata, in bits
 * @param data             Pointer to parameter data from PLC
 */
int up_core_param_write (
   uint16_t slot_ix,
   uint32_t param_ix,
   uint32_t data_bitlength,
   uint8_t * data);

/**
 * Read parameter data (for sending to PLC) from the core (received from host).
 *
 * Uses data stored locally in core (in the "parameter frame").
 *
 * @param slot_ix          Slot index
 * @param param_ix         Parameter index in U-phy data model (not the index
 *                         used by Profinet)
 * @param data_bitlength   Length of parameterdata, in bits
 * @param data             Destination for resulting data
 */
int up_core_param_read (
   uint16_t slot_ix,
   uint32_t param_ix,
   uint32_t data_bitlength,
   uint8_t * data);

/**
 * Check is connected status is set
 *
 * @return true if connection with host is up
 * @return false if not connected to host
 */
bool up_core_is_connected (void);

/**
 * Check if device configuration has been set
 *
 * @return true if a configuration is active
 * @return false if not
 */
bool up_core_is_configured (void);

/**
 * Check if controller / PLC is connected to the device
 *
 * @return true If a controller / PLC is connected
 * @return false if not
 */
bool up_core_is_running (void);

/**
 * Check rpc watchdog is enabled.
 * Typically the watchdog i activated after the device configuration
 * has been downloaded to the core.
 * @return true if rpc watchdog is enabled
 * @return false if rpc watchdog is disabled
 */
bool up_core_is_watchdog_enabled (void);

/**
 * Initialize core data structures
 * Note that calling this operation will not initialize or start rpc.
 * For a monolithic build this initializes the core.
 * For a core/host solution the core side additionally should
 * initialize and start rpc client and servers.
 */
void up_core_init (void);

/**
 * Get the device instance
 */
up_device_t * core_get_device (void);

/**
 * Get pointer to frame info for the rx / output data frame
 *
 * @return Frame info of type \a up_frame_info_t
 */
up_frame_info_t * core_get_rx_frame_info (void);

/**
 * Get pointer to frame info for the tx / input data frame
 *
 * @return Frame info of type \a up_frame_info_t
 */
up_frame_info_t * core_get_tx_frame_info (void);

/**
 * Get the adapter instance
 */
up_adapter_t * core_get_adapter (void);

/**
 * Get the parameter frame (local storage of parameter values)
 */
up_param_frame_t * core_get_param_frame (void);

/**
 * Increase a crc error counter TBD
 */
void core_add_comm_crc_err (void);

/**
 * Indicate that core is disconnected from the host.
 *
 * Update LED state.
 */
void core_disconnected (void);

void core_set_error (up_error_t error);

void core_set_transport (up_transport_t transport);

/**
 * Get string representation of up data type
 *
 * @param dtype      U-Phy data type
 * @return string representing the type. or "unknown data type" if
 *         data type is not known.
 */
const char * core_dtype_to_str (up_dtype_t dtype);

/**
 * Get bit length of an up data type
 *
 * @param dtype      U-Phy data type
 * @return bit length of data type. 1 if the data type is unknown
 */
uint16_t core_dtype_bitlen (up_dtype_t dtype);

/**
 * Get signal array length.
 * 1 is returned for scalar signals.
 *
 * @param signal Signal reference
 * @return uint16_t Length of signal array. 1 for scalar signals. 0 if
 *         signal is NULL.
 */
uint16_t core_signal_array_len (up_signal_t * signal);

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
