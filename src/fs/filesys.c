/*******************************************************************************
 * File Name: filesys.c
 *
 * Description:
 *   This code is based on the ModusToolbox Serial Flash Read and Write
 *   example.
 *
 * Related Document: See README.md
 *
 *
 ********************************************************************************
 * Copyright 2018-2023, Cypress Semiconductor Corporation (an Infineon company)
 *or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 *******************************************************************************/
#include "cy_retarget_io.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cy_pdl.h"
#include "lfs.h"
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include "rte_fs.h"
#include "shell.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define LITTLEFS_TASK_STACK_SIZE (10000)
#define RESULT_OK                (0)
#define RESULT_ERROR             (-1)
#define GET_INT_RETURN_VALUE(result)                                           \
   ((CY_RSLT_SUCCESS == (result)) ? RESULT_OK : RESULT_ERROR)


#define _REENT_SET_ERRNO(x, y)

cyhal_nvm_t obj;
uint32_t flash_addr_offset = 0;
cy_stc_flash_programrow_config_t new_conf;

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/*******************************************************************************
 * Function Definitions
 *******************************************************************************/

/*******************************************************************************
 * Function Name: lfs_flash_bd_read,
 *lfs_flash_bd_prog,lfs_flash_bd_erase
 ********************************************************************************
 * Summary:
 *   Callback function for littlefs used to perform read and write operations on
 *external flash
 *******************************************************************************/
int lfs_flash_bd_read (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   void * buffer,
   lfs_size_t size)
{
   cy_rslt_t result;
   result = cyhal_nvm_read (
      &obj,
      CY_FLASH_SM_SBM_BASE + flash_addr_offset + (block * lfs_cfg->block_size) + off,
      buffer,
      size);
   int res = GET_INT_RETURN_VALUE (result);
   return res;
}

int lfs_flash_bd_prog (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   const void * buffer,
   lfs_size_t size)
{
   cy_rslt_t result;
   Cy_Flashc_MainWriteEnable();

   taskENTER_CRITICAL();
   result = cyhal_nvm_program (
      &obj,
      CY_FLASH_SM_SBM_BASE + flash_addr_offset + (block * lfs_cfg->block_size) + off,
      buffer);
   taskEXIT_CRITICAL();

   int res = GET_INT_RETURN_VALUE (result);
   return res;
}

int lfs_flash_bd_erase (const struct lfs_config * lfs_cfg, lfs_block_t block)
{
   Cy_Flashc_MainWriteEnable();
   uint32_t addr =
      CY_FLASH_SM_SBM_BASE + flash_addr_offset + block * lfs_cfg->block_size;

   taskENTER_CRITICAL();
   cy_rslt_t result = cyhal_nvm_erase (&obj, addr);
   taskEXIT_CRITICAL();

   int res = GET_INT_RETURN_VALUE (result);

   return res;
}

/* Simply return zero because the block does not have any write cache
 */
int lfs_flash_bd_sync (const struct lfs_config * lfs_cfg)
{
   CY_UNUSED_PARAMETER (lfs_cfg);

   return 0;
}

int fs_init (void)
{
   cy_rslt_t result;
   int error;

   result = cyhal_nvm_init (&obj);
   flash_addr_offset = (CY_FLASH_SM_SBM_SIZE == 0x00040000) ? 0x00020000 : 0;

   if (result != CY_RSLT_SUCCESS)
   {
      printf ("Failed to init flash on internal flash\n");
      return -1;
   }

   error = rte_fs_mount();

   if (error)
   {
      printf ("Error - format filesystem failed\n");
   }

   return 0;
}

int _cmd_format (int argc, char * argv[])
{
   rte_fs_format();
   return 0;
}

const shell_cmd_t cmd_format = {
   .cmd = _cmd_format,
   .name = "format_fs",
   .help_short = "format the filesystem",
   .help_long = "Format the filesystem.\n"
                "The device must be reset after the command has been run.\n"};

SHELL_CMD (cmd_format);

/* [] END OF FILE */
