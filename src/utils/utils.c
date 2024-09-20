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

static cyhal_wdt_t wd;

void utils_reset (void)
{
   cy_rslt_t result = cyhal_wdt_init (&wd, 1);
   if (result != CY_RSLT_SUCCESS)
   {
      printf (
         "Failed to initialize watchdog. Error code: 0x%08" PRIx32 "\n",
         (uint32_t)result);
      return;
   }

   cyhal_wdt_start (&wd);
}

/**
 * Override the OSAL system reset function.
 * os_system_reset() is defined with a weak attribute in the OSAL
 * implementation.
 */
void os_system_reset (void)
{
   utils_reset();
   while (1)
      ;
}

int _cmd_reboot (int argc, char * argv[])
{
   printf ("Device will reboot shortly...\n");
   printf ("Note that watchdog reset will not work if device is connected to a "
           "debugger.\n");
   os_system_reset();
}

const shell_cmd_t cmd_reboot = {
   .cmd = _cmd_reboot,
   .name = "reboot",
   .help_short = "reboot the device",
   .help_long = "Trigger a system reset using the hw watchdog."};

SHELL_CMD (cmd_reboot);
