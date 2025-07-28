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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <lfs.h>
#include <filesys.h>
#include <cycfg_qspi_memslot.h>
#include <cy_serial_flash_qspi.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

#include "rte_fs.h"

#ifdef LFS_THREADSAFE
#include "semphr.h"
#endif

// #define FS_LOGS_ENABLE

/***************************************************************************************
 * RTE FS INTERNALS
 */

/* littlefs global state */
static lfs_t lfs;
static int fs_errno; // Last error

typedef struct
{
   lfs_file_t file;
} fs_file_t;

typedef struct
{
   lfs_t * lfs;        // Pointer to the LittleFS context
   lfs_file_t file;    // LittleFS file object
   char * buffer;      // Streaming buffer for fprintf
   size_t buffer_size; // Size of the streaming buffer
} fs_file_stream_t;

#define FPRINTF_BUFFER_SIZE 1024

#ifdef LFS_THREADSAFE

static SemaphoreHandle_t lfs_mutex;

static int fs_lock (const struct lfs_config * c)
{
   xSemaphoreTake (lfs_mutex, portMAX_DELAY);
   return 0;
}

static int fs_unlock (const struct lfs_config * c)
{
   xSemaphoreGive (lfs_mutex);
   return 0;
}
#endif

// block device configuration refer to the data sheet of S25FL512S

#define readSize                   (1U)
#define blockSize                  (262144U)
#define blockCount                 (10)
#define lookaheadSize              (256U)
#define blockCycles                (500U)
#define LFS_CFG_LOOKAHEAD_SIZE_MIN (64UL)

static struct lfs_config lfs_configuration = {
   // block device operations
   .read = lfs_spi_flash_bd_read,
   .prog = lfs_spi_flash_bd_prog,
   .erase = lfs_spi_flash_bd_erase,
   .sync = lfs_spi_flash_bd_sync,

#ifdef LFS_THREADSAFE
   .lock = fs_lock,
   .unlock = fs_unlock,
#endif

   // block device configuration
   .read_size = readSize,
   .prog_size = 0U,
   .block_size = 0U,
   .block_count = blockCount,
   .cache_size = 0U,
   .lookahead_size = lookaheadSize,
   .block_cycles = blockCycles,
};

static const char * lfs_strerror (int err)
{
   switch (err)
   {
   case LFS_ERR_OK:
      return "No error";
   case LFS_ERR_IO:
      return "Error during device operation";
   case LFS_ERR_CORRUPT:
      return "Corrupted";
   case LFS_ERR_NOENT:
      return "No directory entry";
   case LFS_ERR_EXIST:
      return "Entry already exists";
   case LFS_ERR_NOTDIR:
      return "Entry is not a directory";
   case LFS_ERR_ISDIR:
      return "Entry is a directory";
   case LFS_ERR_NOTEMPTY:
      return "Directory is not empty";
   case LFS_ERR_BADF:
      return "Bad file number";
   case LFS_ERR_FBIG:
      return "File too large";
   case LFS_ERR_INVAL:
      return "Invalid parameter";
   case LFS_ERR_NOSPC:
      return "No space left on device";
   case LFS_ERR_NOMEM:
      return "No more memory available";
   case LFS_ERR_NOATTR:
      return "No data/attribute available";
   case LFS_ERR_NAMETOOLONG:
      return "File name too long";
   default:
      return "Unknown error code";
   }
}

#ifdef FS_LOGS_ENABLE

static size_t fs_get_fileusage_recursive (lfs_t * lfs, const char * path)
{
   lfs_dir_t dir;
   struct lfs_info info;
   size_t total_bytes = 0;

   if (lfs_dir_open (lfs, &dir, path) < 0)
   {
      return 0;
   }

   while (lfs_dir_read (lfs, &dir, &info) > 0)
   {
      // Skip current and parent directory entries
      if (strcmp (info.name, ".") == 0 || strcmp (info.name, "..") == 0)
      {
         continue;
      }

      if (info.type == LFS_TYPE_REG)
      {
         total_bytes += info.size;
      }
      else if (info.type == LFS_TYPE_DIR)
      {
         // Build full path for subdirectory
         char subpath[256];
         snprintf (subpath, sizeof (subpath), "%s/%s", path, info.name);

         total_bytes += fs_get_fileusage_recursive (lfs, subpath);
      }
   }

   lfs_dir_close (lfs, &dir);
   return total_bytes;
}

static size_t fs_get_fileusage (lfs_t * lfs)
{
   return fs_get_fileusage_recursive (lfs, "/");
}
#endif // #ifdef FS_LOGS_ENABLE

static int fs_mount (const struct lfs_config * config)
{
#ifdef LFS_THREADSAFE
   lfs_mutex = xSemaphoreCreateMutex();
   if (lfs_mutex == NULL)
   {
      printf ("Failed to create lfs mutex\n");
      return -1;
   }
#endif

   fs_errno = lfs_mount (&lfs, config);

   /* reformat if we can't mount the filesystem
      this should only happen on the first boot */
   if (fs_errno)
   {
      printf ("%s, format file system.\n", rte_fs_error (NULL));
      fs_errno = lfs_format (&lfs, config);
      if (fs_errno)
      {
         printf ("Error - format filesystem failed (%s)\n", rte_fs_error (NULL));
         return -1;
      }
      fs_errno = lfs_mount (&lfs, config);
      if (fs_errno)
      {
         printf ("Error - mounting filesystem failed (%s)\n", rte_fs_error (NULL));
         return -1;
      }
   }
   return fs_errno;
}

static int fs_unmount (void)
{
   return lfs_unmount (&lfs);
}

static int fs_format (const struct lfs_config * config)
{
   fs_errno = lfs_format (&lfs, config);
   if (fs_errno)
   {
      printf ("Error - format filesystem failed (%s)\n", rte_fs_error (NULL));
   }
   else
   {
      printf ("Ok - filesystem formatted. Reset the device.\n");
   }
   return fs_errno;
}

static int fs_remove (const char * path)
{
   int result = lfs_remove (&lfs, path);
   return result;
}

static int fs_mkdir (const char * path)
{
   /* embedded filesystems such as lfs doesn't manage mode */
   int result = lfs_mkdir (&lfs, path);
   return result;
}

static int fs_feof (fs_file_stream_t * stream)
{
   /* check current position */
   lfs_soff_t current_pos = lfs_file_tell (stream->lfs, &stream->file);
   if (current_pos < 0)
   {
      /* error during tell, treat as not EOF */
      return 0;
   }

   /* compare position with file size */
   lfs_soff_t size = lfs_file_size (stream->lfs, &stream->file);
   if (size < 0)
   {
      /* error during size fetch, treat as not EOF */
      return 0;
   }

   /* If current position >= file size, we are at EOF */
   return (current_pos >= size) ? 1 : 0;
}

/*
 * FILE STREAM APIs
 */

static RTE_FILE * fs_fopen (const char * path, const char * mode)
{
   /* translate mode to LittleFS flags */
   int flags = 0;
   if (mode[0] == 'r')
      flags |= LFS_O_RDONLY;
   if (mode[0] == 'w')
      flags |= LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC;
   if (mode[0] == 'a')
      flags |= LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;

   fs_file_stream_t * file =
      (fs_file_stream_t *)malloc (sizeof (fs_file_stream_t));
   if (!file)
   {
      return NULL;
   }

   file->lfs = &lfs;
   file->buffer = (char *)malloc (FPRINTF_BUFFER_SIZE);
   if (!file->buffer)
   {
      free (file);
      return NULL;
   }
   file->buffer_size = FPRINTF_BUFFER_SIZE;

   if (lfs_file_open (file->lfs, &file->file, path, flags) < 0)
   {
      free (file->buffer);
      free (file);
      return NULL;
   }

   return (RTE_FILE *)file;
}

static int fs_fclose (RTE_FILE * file)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;

   if (!stream)
      return -1;

   int res = lfs_file_close (stream->lfs, &stream->file);

   free (stream->buffer);
   free (stream);

   return res;
}

static size_t fs_fread (void * ptr, size_t size, size_t count, RTE_FILE * file)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;

   if (!stream)
      return 0;

   lfs_ssize_t read =
      lfs_file_read (stream->lfs, &stream->file, ptr, size * count);
   return read < 0 ? 0 : (size_t)(read / size);
}
static size_t fs_fwrite (const void * ptr, size_t size, size_t count, RTE_FILE * file)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;

   if (!stream)
      return 0;

   lfs_ssize_t written =
      lfs_file_write (stream->lfs, &stream->file, ptr, size * count);
   return written < 0 ? 0 : (size_t)(written / size);
}

static int fs_fseek (RTE_FILE * file, long offset, rte_fs_whence_t whence)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;
   if (!stream)
      return -1;

   int lfs_whence = 0;
   switch (whence)
   {
   case rte_fs_SEEK_SET:
      lfs_whence = LFS_SEEK_SET;
      break;
   case rte_fs_SEEK_CUR:
      lfs_whence = LFS_SEEK_CUR;
      break;
   case rte_fs_SEEK_END:
      lfs_whence = LFS_SEEK_END;
      break;
   default:
      return -1;
   }

   return (int)lfs_file_seek (stream->lfs, &stream->file, offset, lfs_whence);
}

static int fs_ftell (RTE_FILE * file)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;
   if (!stream)
      return -1;

   return (int)lfs_file_tell (stream->lfs, &stream->file);
}

/*********************************************************************
 * ANSI C stdio.h abstractions
 ********************************************************************/

int rte_fs_mount (void)
{
   /* autodetect qspi flash */
   lfs_configuration.block_size =
      cy_serial_flash_qspi_get_erase_size (SFDP_SlaveSlot_0.baseAddress);
   lfs_configuration.prog_size =
      cy_serial_flash_qspi_get_prog_size (SFDP_SlaveSlot_0.baseAddress);
   lfs_configuration.cache_size =
      cy_serial_flash_qspi_get_prog_size (SFDP_SlaveSlot_0.baseAddress);

   int retval;
   retval = fs_mount (&lfs_configuration);

#ifdef FS_LOGS_ENABLE
   lfs_ssize_t used_blocks = lfs_fs_size (&lfs);
   size_t qspi_sz = cy_serial_flash_qspi_get_size();
   size_t total_space =
      lfs_configuration.block_count * lfs_configuration.block_size;

   printf (
      "Autodetect QSPI flash %.2f MB (%d bytes configured)\n",
      qspi_sz / 1048576.0,
      total_space);

   /* Note -- littlefs using extra sectors due to filesystem metadata / overhead
    */
   printf (
      "  sector sz : %ld (used %ld sectors out of %ld)\n",
      lfs_configuration.block_size,
      used_blocks,
      lfs_configuration.block_count);

   printf ("  filesystem usage %d bytes\n", fs_get_fileusage (&lfs));
#endif

   return retval;
}

int rte_fs_unmount()
{
   return fs_unmount();
}

int rte_fs_format (void)
{
   return fs_format (&lfs_configuration);
}

int rte_fs_remove (const char * path)
{
   return fs_remove (path);
}

int rte_fs_mkdir (const char * path)
{
   return fs_mkdir (path);
}

const char * rte_fs_error (RTE_FILE * file)
{
   /* return last global fs error */
   return lfs_strerror (fs_errno);
}

/* FILE STREAM APIs */

RTE_FILE * rte_fs_fopen (const char * path, const char * mode)
{
   return fs_fopen (path, mode);
}

int rte_fs_fclose (RTE_FILE * file)
{
   return fs_fclose (file);
}

size_t rte_fs_fread (void * ptr, size_t size, size_t count, RTE_FILE * stream)
{
   return fs_fread (ptr, size, count, stream);
}

size_t rte_fs_fwrite (const void * ptr, size_t size, size_t count, RTE_FILE * stream)
{
   return fs_fwrite (ptr, size, count, stream);
}

int rte_fs_feof (RTE_FILE * file)
{
   return fs_feof ((fs_file_stream_t *)file);
}

int rte_fs_fseek (RTE_FILE * stream, long offset, rte_fs_whence_t whence)
{
   return (int)fs_fseek (stream, offset, whence);
}

long rte_fs_ftell (RTE_FILE * stream)
{
   return (long)fs_ftell (stream);
}

char * rte_fs_fgets (char * buffer, int size, RTE_FILE * stream)
{
   if (!buffer || size <= 0 || !stream)
   {
      return NULL;
   }

   int i = 0;
   while (i < size - 1)
   {
      char c;
      int res = fs_fread (&c, 1, 1, stream);

      if (res <= 0)
      {
         break; // End of file or error
      }

      buffer[i++] = c;
      if (c == '\n')
      {
         break;
      }
   }

   buffer[i] = '\0';
   return (i > 0) ? buffer : NULL;
}

int rte_fs_fputs (const char * str, RTE_FILE * file)
{
   if (!str || !file)
   {
      return -1;
   }

   fs_file_stream_t * stream = (fs_file_stream_t *)file;

   lfs_ssize_t written =
      lfs_file_write (stream->lfs, &stream->file, str, strlen (str));

   return (written < 0) ? -1 : (int)written;
}

int rte_fs_fprintf (RTE_FILE * file, const char * format, ...)
{
   fs_file_stream_t * stream = (fs_file_stream_t *)file;
   if (!stream || !stream->buffer)
      return -1;

   va_list args;
   va_start (args, format);

   int len = vsnprintf (stream->buffer, stream->buffer_size, format, args);
   va_end (args);

   if (len < 0 || len >= (int)stream->buffer_size)
   {
      return -1;
   }

   lfs_ssize_t written =
      lfs_file_write (stream->lfs, &stream->file, stream->buffer, len);

   return written < 0 ? -1 : len;
}
