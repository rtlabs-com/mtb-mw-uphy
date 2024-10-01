/********************************************************************
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

#include "cyhal_wdt.h"
#include "shell.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

/**
 * Override the OSAL system reset function.
 * os_system_reset() is defined with a weak attribute in the OSAL
 * implementation.
 */
void os_system_reset (void)
{
   NVIC_SystemReset();
}

int _cmd_reboot (int argc, char * argv[])
{
   os_system_reset();
   return 0;
}

const shell_cmd_t cmd_reboot = {
   .cmd = _cmd_reboot,
   .name = "reboot",
   .help_short = "reboot the device",
   .help_long = "Trigger a system reset using the hw watchdog."};

SHELL_CMD (cmd_reboot);
