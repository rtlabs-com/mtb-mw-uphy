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
#include "cy_serial_flash_qspi.h"
#include "cybsp.h"
#include "cycfg_qspi_memslot.h"
#include "cyhal.h"
#include "lfs.h"
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include "filesys.h"
#include "shell.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define LITTLEFS_TASK_STACK_SIZE (10000)
#define MEM_SLOT_NUM             (0u) /* Slot number of the memory to use */
#define QSPI_BUS_FREQUENCY_HZ    (50000000lu)
#define RESULT_OK                (0)
#define RESULT_ERROR             (-1)
#define GET_INT_RETURN_VALUE(result)                                           \
   ((CY_RSLT_SUCCESS == (result)) ? RESULT_OK : RESULT_ERROR)

// block device configuration refer to the data sheet of S25FL512S
#define readSize                   (1U)
#define progSize                   (512U)    // minimal programming page
#define blockSize                  (262144U) // minimal erase sector
#define blockCount                 (10)
#define cacheSize                  (512U)
#define lookaheadSize              (256U)
#define blockCycles                (500U)
#define LFS_CFG_LOOKAHEAD_SIZE_MIN (64UL)

#define _REENT_SET_ERRNO(x, y)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
lfs_t lfs;

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/

/*******************************************************************************
 * Function Definitions
 *******************************************************************************/

/*******************************************************************************
 * Function Name: lfs_spi_flash_bd_read,
 *lfs_spi_flash_bd_prog,lfs_spi_flash_bd_erase
 ********************************************************************************
 * Summary:
 *   Callback function for littlefs used to perform read and write operations on
 *external flash
 *******************************************************************************/
int lfs_spi_flash_bd_read (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   void * buffer,
   lfs_size_t size)
{
   cy_rslt_t result;
   result =
      cy_serial_flash_qspi_read ((block * lfs_cfg->block_size) + off, size, buffer);
   int res = GET_INT_RETURN_VALUE (result);
   return res;
}

int lfs_spi_flash_bd_prog (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   const void * buffer,
   lfs_size_t size)
{
   cy_rslt_t result = cy_serial_flash_qspi_write (
      (block * lfs_cfg->block_size) + off,
      size,
      buffer);
   int res = GET_INT_RETURN_VALUE (result);
   return res;
}

int lfs_spi_flash_bd_erase (const struct lfs_config * lfs_cfg, lfs_block_t block)
{
   uint32_t addr = block * lfs_cfg->block_size;
   cy_rslt_t result = cy_serial_flash_qspi_erase (addr, lfs_cfg->block_size);
   int res = GET_INT_RETURN_VALUE (result);

   return res;
}

/* Simply return zero because the QSPI block does not have any write cache in
 * MMIO mode.
 */
int lfs_spi_flash_bd_sync (const struct lfs_config * lfs_cfg)
{
   CY_UNUSED_PARAMETER (lfs_cfg);

   return 0;
}

/* Map POSIX flags to LittleFS flags */
int to_lfs_flags (int flags)
{
   int lfs_flags = 0;

   if (flags == O_RDONLY)
   {
      return LFS_O_RDONLY;
   }

   if (flags & O_WRONLY)
   {
      lfs_flags |= LFS_O_WRONLY;
   }
   if (flags & O_RDWR)
   {
      lfs_flags |= LFS_O_RDWR;
   }
   if (flags & O_CREAT)
   {
      lfs_flags |= LFS_O_CREAT;
   }
   if (flags & O_EXCL)
   {
      lfs_flags |= LFS_O_EXCL;
   }
   if (flags & O_TRUNC)
   {
      lfs_flags |= LFS_O_TRUNC;
   }
   if (flags & O_APPEND)
   {
      lfs_flags |= LFS_O_APPEND;
   }

   return lfs_flags;
}

int fs_open (const char * path, int flags)
{
   int res;
   int lfs_flags = to_lfs_flags (flags);

   lfs_file_t * fd = malloc (sizeof (lfs_file_t));
   if (fd == NULL)
   {
      return -1;
   }

   res = lfs_file_open (&lfs, fd, path, lfs_flags);
   if (res != RESULT_OK)
   {
      free (fd);
      return RESULT_ERROR;
   }

   return (int)fd;
}

int fs_close (int fd)
{
   int res = lfs_file_close (&lfs, (lfs_file_t *)fd);
   if (res != RESULT_OK)
   {
      return RESULT_ERROR;
   }

   free ((void *)fd);
   return RESULT_OK;
}

int fs_write (int fd, const void * buf, size_t count)
{
   return (int)lfs_file_write (&lfs, (lfs_file_t *)fd, buf, count);
}

int fs_read (int fd, void * buf, size_t count)
{
   return (int)lfs_file_read (&lfs, (lfs_file_t *)fd, buf, count);
}

int fs_unlink (const char * path)
{
   int res = lfs_remove (&lfs, path);
   if (res != RESULT_OK)
   {
      return RESULT_ERROR;
   }

   return RESULT_OK;
}

static const struct lfs_config lfs_configuration = {
   // block device operations
   .read = lfs_spi_flash_bd_read,
   .prog = lfs_spi_flash_bd_prog,
   .erase = lfs_spi_flash_bd_erase,
   .sync = lfs_spi_flash_bd_sync,

   // block device configuration
   .read_size = readSize,
   .prog_size = progSize,
   .block_size = blockSize,
   .block_count = blockCount,
   .cache_size = cacheSize,
   .lookahead_size = lookaheadSize,
   .block_cycles = blockCycles,
};

int fs_init (void)
{
   cy_rslt_t result;
   int error;

   result = cy_serial_flash_qspi_init (
      smifMemConfigs[MEM_SLOT_NUM],
      CYBSP_QSPI_D0,
      CYBSP_QSPI_D1,
      CYBSP_QSPI_D2,
      CYBSP_QSPI_D3,
      NC,
      NC,
      NC,
      NC,
      CYBSP_QSPI_SCK,
      CYBSP_QSPI_SS,
      QSPI_BUS_FREQUENCY_HZ);

   if (result != CY_RSLT_SUCCESS)
   {
      printf ("Failed to init flash on qspi\n");
      return -1;
   }

   // error = lfs_format (&lfs, &lfs_configuration);
   error = lfs_mount (&lfs, &lfs_configuration);
   // reformat if we can't mount the filesystem
   // this should only happen on the first boot
   if (error)
   {
      printf ("Format file system.\n");
      error = lfs_format (&lfs, &lfs_configuration);
      if (error)
      {
         printf ("Error - format filesystem failed\n");

         printf (
            "-------------------------------------------------------------\n"
            "The EVK is built with different serial flash memories.\n"
            "The application flash memory configuration is a build time\n"
            "configuration and no detection or autoconfiguration is done\n"
            "in this sample.\n"
            "Change the flash configuration using the Modus Toolbox\n"
            "QSPI Configurator tool and rebuild the application.\n"
            "Identify mounted flash on your EVK using the text printed on the\n"
            "QSPI Flash circuit.\n"
            "Tested with S25FL512S and S25HL512T(Uniform)\n"
            "-------------------------------------------------------------\n");

         return -1;
      }
      error = lfs_mount (&lfs, &lfs_configuration);
      if (error)
      {
         printf ("Error - mounting filesystem failed\n");
         return -1;
      }
   }

   return 0;
}

int mkdir (const char * path, mode_t mode)
{
   int result = lfs_mkdir (&lfs, path);
   return GET_INT_RETURN_VALUE (result);
}

int fs_format (void)
{
   int error = lfs_format (&lfs, &lfs_configuration);
   if (error)
   {
      printf ("Error - format filesystem failed\n");
      return RESULT_ERROR;
   }

   printf ("Ok - filesystem formatted. Reset the device.\n");
   return RESULT_OK;
}

int _cmd_format (int argc, char * argv[])
{
   fs_format();
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
