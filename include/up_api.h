/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifndef UP_API_H
#define UP_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "up_export.h"
#include "up_types.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define UP_BITS_TO_BYTES(n) (((n) + 7) >> 3)

typedef struct up up_t;

/**
 * U-Phy device configuration including:
 * - Device data configuration
 * - Fieldbus configuration
 * - U-Phy API callback configuration
 *
 *  Typically the device data and fieldbus configurations are
 *  generated using the U-Phy device builder and U-Phy generator
 *  tools.
 */
typedef struct up_cfg
{
   /**
    * Device configuration including slots, signals and more.
    * When a generated device is used, the up_device variable
    * in model.h/c shall be used. Initialization: .device = &up_device
    */
   up_device_t * device;

   /**
    *  Fieldbus configuration.
    *  Note that the \a up_busconf_t is a union type and that
    *  only fieldbus at a time can be active.
    */
   up_busconf_t * busconf;

   /**
    * List of references to all signal and parameter variables.
    * When a generated device is used, the up_vars in model.h/c shall
    * be used here. Initialization: .vars = up_vars
    */
   up_signal_info_t * vars;

   /**
    * Indication that core has received (output) data from the PLC.
    * On this event the application is expected to call
    * \a up_read_outputs().
    *
    * @param up         The U-Phy instance.
    * @param user_arg   User-defined data (not used by U-Phy)
    */
   void (*avail) (up_t * up, void * user_arg);

   /**
    * Indication that core is about to send (input) data to the PLC.
    * On this event the application is expected to call
    * \a up_write_inputs().
    *
    * @param up         The U-Phy instance.
    * @param user_arg   User-defined data (not used by U-Phy)
    */
   void (*sync) (up_t * up, void * user_arg);

   /**
    * Indication generated by the core when one or more
    * parameters have been written (by remote PLC or similar).
    * On this indication the application shall call \a up_param_get_write_req()
    * until all parameters have been synchronized.
    *
    * @param up         The U-Phy instance.
    * @param user_arg   User-defined data (not used by U-Phy)
    */
   void (*param_write_ind) (up_t * up, void * user_arg);

   /**
    * Indication generated by the core every 10 ms.
    *
    * @param up         The U-Phy instance.
    * @param user_arg   User-defined data (not used by U-Phy)
    */
   void (*poll_ind) (up_t * up, void * user_arg);

   /**
    * Indication that the core status has changed.
    *
    * @param up         The U-Phy instance.
    * @param status     New status
    * @param user_arg   User-defined data (not used by U-Phy)
    *
    */
   void (*status_ind) (up_t * up, uint32_t status, void * user_arg);

   /**
    * Indication that an error has occurred.
    * See up_error_t for possible errors.
    *
    * @param up         The U-Phy instance.
    * @param error      Type of error
    * @param user_arg   User-defined data (not used by U-Phy)
    */
   void (*error_ind) (up_t * up, up_error_t error, void * user_arg);

   void (*profinet_signal_led_ind) (up_t * up, void * user_arg);

   /**
    *  Userdata passed to callbacks, not used by U-Phy
    */
   void * cb_arg;
} up_cfg_t;

/**
 * Get up version.
 *
 * @return version of up stack
 */
UP_EXPORT const char * up_version (void);

/**
 * Initialise up stack.
 *
 * @param cfg                stack configuration
 *
 * @return up handle
 */
UP_EXPORT up_t * up_init (const up_cfg_t * cfg);

/**
 * Get current configuration of the up stack.
 * The configuration passed to \a up_init() will be
 * returned.
 *
 * @param up         The U-Phy instance.
 * @return up stack configuration
 */
UP_EXPORT const up_cfg_t * up_get_cfg (up_t * up);

/**
 * Generate input frame.
 * Write all input variables / signals into buffer.
 *
 * @param up            The U-Phy instance.
 * @param frame         The buffer to which signal values are written
 */
UP_EXPORT void up_pack_input_frame (up_t * up, uint8_t * frame);

/**
 * Update output variables / signals.
 * Read buffer and update output variables / signal.
 *
 * @param up            The U-Phy instance.
 * @param frame         The buffer from which signal values are read
 */
UP_EXPORT void up_unpack_output_frame (up_t * up, uint8_t * frame);

/**
 * Update output signals.
 *
 * Read the output frame from core and update output variables / signals
 * accordingly.
 *
 * Typically this operation is called when the avail() callback
 * has been triggered by the core (after it has received output data from the PLC).
 *
 * @param up         The U-Phy instance.
 */
void up_read_outputs (up_t * up);

/**
 * Update input signals / variables.
 *
 * Update the inputs frame and write it to the core.
 *
 * Typically this operation is called when the sync() callback
 * has been triggered by the core (when it is about to send input data to the PLC).
 *
 * @param up         The U-Phy instance.
 */
void up_write_inputs (up_t * up);

/**
 * Check for parameter updates.
 *
 * This function is intended to be called when a \e param_write_ind()
 * callback has been triggered by the core. Example usage:
 *
 *     while (up_param_get_write_req (up, &slot_ix, &param_ix, &param) == 0)
 *     {
 *        p = &up_device.slots[slot_ix].params[param_ix];
 *        memcpy (up_vars[p->ix], param.data, param.dataLength);
 *        free (param.data);
 *     }
 *
 * Note that the caller must free \a param.data after calling this
 * function.
 *
 * @param up              The U-Phy instance.
 * @param slot_ix         Index of slot in which parameter was written.
 * @param param_ix        Index of written parameter
 * @param param           Size and reference to new value.
 * @return                0 on success
 *                        -1 on error or if all parameter write requests have
 *                        been cleared
 */
int up_param_get_write_req (
   up_t * up,
   uint16_t * slot_ix,
   uint16_t * param_ix,
   binary_t * param);

/**
 * Write parameter.
 *
 * Lets the application write a new value to the core param_frame buffer.
 *
 * Usage example:
 *
 *     binary_t param;
 *     param.dataLength = sizeof(up_data.I8O8.Parameter_1);
 *     param.data = &up_data.I8O8.Parameter_1;
 *     up_write_param (up, slot_ix, param_ix, &param);
 *
 * @param up              The U-Phy instance.
 * @param slot_ix         Index of the slot for which the parameter should be
 *                        written.
 * @param param_ix        Index of the parameter which should be written.
 * @param param           Size and reference to value.
 * @return                0 on success, -1 on error
 */
int up_write_param (up_t * up, uint16_t slot_ix, uint16_t param_ix, binary_t * param);

/**
 * Read current value of a parameter
 *
 * Lets the application read a value from the core param_frame buffer.
 *
 * Usage example:
 *
 *     binary_t param;
 *     up_read_param (up, slot_ix, param_ix, &param);
 *     ...
 *     free (param.data);
 *
 * Note that the caller must free \a param.data after calling this
 * function.
 *
 * @param up              The U-Phy instance.
 * @param slot_ix         Index of the slot for which the parameter should be
 *                        read.
 * @param param_ix        Index of the parameter which should be read.
 * @param param           Size and reference to value.
 * @return                0 on success, -1 on error
 */
int up_read_param (up_t * up, uint16_t slot_ix, uint16_t param_ix, binary_t * param);

/**
 * Add alarm.
 * The alarm message is defined by the station description file.
 *
 * @param up              The U-Phy instance.
 * @param slot_ix         Index of the slot for which alarm shall be set.
 * @param alarm           The alarm level and error code.
 * @return                0 on success, -1 on error
 */
int up_add_alarm (up_t * up, uint16_t slot_ix, const up_alarm_t * alarm);

/**
 * Remove alarm previously added using \a up_add_alarm().
 *
 * @param up              The U-Phy instance.
 * @param slot_ix         Index of the slot for which alarm shall be set.
 * @param alarm           The alarm level and error code.
 * @return                0 on success, -1 on error
 */
int up_remove_alarm (up_t * up, uint16_t slot_ix, const up_alarm_t * alarm);

/**
 * Read current up status.
 * The status is not read from core, but the stored value is returned.
 * Note that the status indication callback is always generated when
 * the status changes.
 *
 * @param up              The U-Phy instance.
 * @return                Current status
 */
uint32_t up_read_status (up_t * up);

/**
 * Initialize the RPC communication with mod01 or other core device.
 * The RPC transport must be initialized before this function is called.
 *
 * @param up           The U-Phy instance.
 * @return             0 on success, -1 on error
 */
int up_rpc_init (up_t * up);

/**
 * Start RPC communication.
 *
 * @param up           The U-Phy instance.
 * @param reset_core   Reset core device during start procedure.
 * @return             0 on success, -1 on error
 */
int up_rpc_start (up_t * up, bool reset_core);

/**
 * Initialize device configuration
 * Writes the device configuration to U-Phy core. The
 * Fieldbus is not started.
 *
 * @param up           The U-Phy instance.
 * @return             0 on success, -1 on error
 */
int up_init_device (up_t * up);

/**
 * Start device
 * Starts the configured fieldbus.
 *
 * @param up           The U-Phy instance.
 * @return             0 on success, -1 on error
 */
int up_start_device (up_t * up);

/**
 * Enable or disable the core communication watchdog.
 * The communication watchdog supervises the communication
 * between the host and the core. If a problem is detected
 * the application is notified using the error_ind callback
 * with error type UP_ERROR_CORE_COMMUNICATION.
 *
 * Calling this function with enable=true also enables a
 * communication watchdog on the core side.
 * The watchdog shall not be enabled when debugging the application
 * since stopping on a breakpoint will trigger a watchdog timeout on
 * the core side.
 *
 * @param up           The U-Phy instance.
 * @param enable       true to enable watchdog, false to disable watchdog
 * @return             0 on success, -1 on error
 */
int up_enable_watchdog (up_t * up, bool enable);

 /**
   * The default free running mode of operation uses the predefined
   * event mask UP_EVENT_MASK_FREE_RUNNING_MODE. This is set during 
   * initialization.
   * To start synchronous mode of operation write the event mask
   * UP_EVENT_MASK_SYNCHRONOUS_MODE which enables UP_SYNC and UP_AVAIL events
   *
   * @param up          The U-Phy instance.
   * @param mask        Event mask
   * @return            0 on success, -1 on error
   */
int up_write_event_mask(up_t * up, uint32_t mask);

/**
 * Update EtherCAT EEPROM content.
 * Note that EtherCAT must be initialized when
 * this function is called, or an error will
 * be returned.
 *
 * @param up            The U-Phy instance.
 * @param data          Pointer to start of eeprom data
 * @param size          The size of the eeprom data
 * @return              0 on success, -1 on error
 */
int up_write_ecat_eeprom (up_t * up, const uint8_t * data, uint16_t size);

/**
 * Indicate interrupt service request
 *
 * This function shall be called when the core interrupt fires.
 */
void up_event_ind (void);

/**
 * U-Phy thread function, intended to be executed in an endless loop.
 * Normally true is returned.
 * If an error that prevents the normal operation of U-Phy lib is detected
 * false is returned. For example if the communication with U-Phy Core
 * is not working, false is returned.
 *
 * @param up      U-Phy instance
 * @return        true on success, false on error
 */
bool up_worker (up_t * up);

/**
 * Initialise TCP transport
 *
 * @param up         The U-Phy instance.
 * @param ip         IP address to connect to
 * @param port       TCP port number to connect to
 * @return           0 on success, -1 on error
 */
int up_tcp_transport_init (up_t * up, const char * ip, uint16_t port);

/**
 * Initialise SPI transport
 *
 * @param up         The U-Phy instance.
 * @param name       Name
 * @return           0 on success, -1 on error
 */
int up_spi_master_transport_init (up_t * up, const char * name);

/**
 * Initialise UART transport
 *
 * @param up         The U-Phy instance.
 * @param name       Name
 * @return           0 on success, -1 on error
 */
int up_uart_transport_init (up_t * up, const char * name);

/**
 * Initialise a PC serial port transport
 *
 * @param up         The U-Phy instance.
 * @param name       Name
 * @return           0 on success, -1 on error
 */
int up_serial_transport_init (up_t * up, const char * name);

/**
 * Return the error literal
 *
 * @param error   Error code
 * @return Pointer to string literal
 */
const char * up_error_to_str (up_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* UP_API_H */
