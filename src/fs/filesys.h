#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <sys/types.h>
#include <fcntl.h>

#define STORAGE_ROOT "/" /* No trailing slash */

/**
 * @brief Initialize the filesystem.
 *
 * @return int 0 on success, negative value on error.
 */
int fs_init (void);

/**
 * @brief Open a file.
 *
 * @param path The path to the file.
 * @param flags The flags for opening the file (e.g., O_RDONLY, O_WRONLY).
 * @return int The file descriptor on success, negative value on error.
 */
int fs_open (const char * path, int flags);

/**
 * @brief Close a file.
 *
 * @param fd The file descriptor of the file to close.
 * @return int 0 on success, negative value on error.
 */
int fs_close (int fd);

/**
 * @brief Write to a file.
 *
 * @param fd The file descriptor of the file to write to.
 * @param buf The buffer containing the data to write.
 * @param count The number of bytes to write.
 * @return int The number of bytes written on success, negative value on error.
 */
int fs_write (int fd, const void * buf, size_t count);

/**
 * @brief Read from a file.
 *
 * @param fd The file descriptor of the file to read from.
 * @param buf The buffer to store the read data.
 * @param count The number of bytes to read.
 * @return int The number of bytes read on success, negative value on error.
 */
int fs_read (int fd, void * buf, size_t count);

/**
 * @brief Delete a file.
 *
 * @param path The path to the file to delete.
 * @return int 0 on success, negative value on error.
 */
int fs_unlink (const char * path);

/**
 * @brief Format the filesystem.
 *
 * @return int 0 on success, negative value on error.
 */
int fs_format (void);

#endif // FILESYSTEM_H
