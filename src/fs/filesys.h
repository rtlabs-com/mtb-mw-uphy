#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <sys/types.h>
#include <lfs.h>

#define STORAGE_ROOT "/" /* No trailing slash */

int lfs_spi_flash_bd_read (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   void * buffer,
   lfs_size_t size);

int lfs_spi_flash_bd_prog (
   const struct lfs_config * lfs_cfg,
   lfs_block_t block,
   lfs_off_t off,
   const void * buffer,
   lfs_size_t size);

int lfs_spi_flash_bd_erase (const struct lfs_config * lfs_cfg, lfs_block_t block);

int lfs_spi_flash_bd_sync (const struct lfs_config * lfs_cfg);


/**
 * @brief Initialize the filesystem.
 *
 * @return int 0 on success, negative value on error.
 */
int fs_init (void);

#endif // FILESYSTEM_H
