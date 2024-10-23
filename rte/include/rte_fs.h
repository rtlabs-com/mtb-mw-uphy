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

#ifndef RTE_FS_H
#define RTE_FS_H

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_ROOT "/" /* No trailing slash */

typedef void RTE_FILE;

/**
 * These are equivalent with stdio definitions.
 * stdio.h define these as macros, so to avoid
 * name collition, a prefix is added.
 */
typedef enum rte_fs_whence
{
   rte_fs_SEEK_SET = 0,
   rte_fs_SEEK_CUR = 1,
   rte_fs_SEEK_END = 2,
} rte_fs_whence_t;

/**
 * @brief Mounts filesystem
 *
 * The mount syntax and requirements differ greatly between libraries.
 * Additional configuration parameters can be added to rte_config.h
 *
 * @return int Success 0, Failure -1
 */
int rte_fs_mount (void);

/**
 * @brief Unmounts filesystem
 *
 * @return int Success 0, Failure -1
 */
int rte_fs_unmount (void);

/**
 * @brief Formats filesystem
 *
 * @return int Success 0, Failure -1
 */
int rte_fs_format (void);

/**
 * @brief Removes a file or directory.
 *
 * The remove() function deletes a name from the file system. If name refers to
 * a file, it is removed. If name refers to a directory, it is removed only if
 * it is empty.
 *
 * @param pathname The path to the file or directory to be removed.
 * @return Zero if the removal succeeds. On error, -1 is returned, and errno is
 * set appropriately.
 */
int rte_fs_remove (const char * path);

/**
 * @brief Creates a directory.
 *
 * This function creates a directory specified by path with the given mode.
 *
 * @param path The path to the directory to be created.
 * @return 0 on success, or a negative value on error.
 */
int rte_fs_mkdir (const char * path);

/**
 * @brief Returns a string representing the last error encountered,
 * on a file stream.
 *
 * Can be thought of as strerror(ferror(file))
 * This is only meant to give more detail to error logs.
 *
 * If the file handler is NULL or invalid, the generic error call
 * is checked instead. This function can be used regardless.
 *
 * @return returns a pointer to a string that describes the error
 */
const char * rte_fs_error (RTE_FILE * file);

/*********************************************************************
 * ANSI C99 stdio.h abstractions
 ********************************************************************/

/**
 * @brief Opens a file.
 *
 * Stick to only mode specifiers within this list;
 * {r, w, a, r+, w+, a+}
 *
 * @param pathname The path to the file to be opened.
 * @param mode The mode in which the file is to be opened.
 * @return a RTE_FILE pointer, or NULL if the open fails.
 */
RTE_FILE * rte_fs_fopen (const char * path, const char * mode);

/**
 * @brief Closes a file.
 *
 * @param file Pointer to a RTE_FILE that specifies the file to be
 * closed.
 * @return Zero if the file is successfully closed. If an error is detected, EOF
 * (negative value) is returned.
 */
int rte_fs_fclose (RTE_FILE * file);

/**
 * @brief Reads data from a file.
 *
 * @param buffer Pointer to the memory block where the data is stored.
 * @param size Size in bytes of each element to be read.
 * @param count Number of elements to be read.
 * @param file Pointer to a RTE_FILE that specifies an input file.
 * @return The number of elements successfully read. If an error occurs, or the
 * end-of-file is reached, the return value is a short item count (or zero).
 */
size_t rte_fs_fread (void * buffer, size_t size, size_t count, RTE_FILE * file);

/**
 * @brief Writes data to a file.
 *
 * @param buffer Pointer to the memory block that is the source of data to be
 * written.
 * @param size Size in bytes of each element to be written.
 * @param count Number of elements to be written.
 * @param file Pointer to a RTE_FILE that specifies an output file.
 * @return The number of elements successfully written. If an error occurs, the
 * return value is less than count.
 */
size_t rte_fs_fwrite (const void * buffer, size_t size, size_t count, RTE_FILE * file);

/**
 * @brief Checks the end-of-file indicator of a stream.
 *
 * @param file Pointer to the FILE stream.
 * @return Non-zero if end-of-file is set, zero otherwise.
 */
int rte_fs_feof (RTE_FILE * file);

/**
 * @brief Sets the file position indicator for a file.
 *
 * The fseek() function sets the file position indicator for the file pointed to
 * by file. The new position, measured in bytes, is obtained by adding offset
 * bytes to the position specified by whence.
 *
 * @param file Pointer to a RTE_FILE that identifies the file.
 * @param offset The number of bytes to offset from whence.
 * @param whence Specifies the starting position for the offset. It can be:
 * - SEEK_SET: Beginning of the file.
 * - SEEK_CUR: Current position of the file pointer.
 * - SEEK_END: End of the file.
 * @return Zero if the request succeeds. Otherwise, it returns a nonzero value.
 */
int rte_fs_fseek (RTE_FILE * file, long offset, rte_fs_whence_t whence);

/**
 * @brief Gets the current file position indicator for a file.
 *
 * The ftell() function obtains the current value of the file position indicator
 * for the file pointed to by file.
 *
 * @param file Pointer to a RTE_FILE that identifies the file.
 * @return The current value of the file position indicator, or -1L on error.
 */
long rte_fs_ftell (RTE_FILE * file);

/**
 * @brief Reads a line from a file.
 *
 * @param buffer Pointer to the buffer where the string is stored.
 * @param size Maximum number of characters to be read, including the
 * terminating null byte.
 * @param file Pointer to a RTE_FILE that identifies the file.
 * @return str on success, or NULL on error or end-of-file while no characters
 * have been read.
 */
char * rte_fs_fgets (char * buffer, int size, RTE_FILE * file);

/**
 * @brief Prints a line to a file file.
 *
 * fputs does not automatically add newline unline puts.
 *
 * @param str null terminated string to print.
 * @param file Pointer to a RTE_FILE that identifies the file.
 * @return The number of elements successfully written. Negative value if error.
 */
int rte_fs_fputs (const char * str, RTE_FILE * file);

/**
 * @brief Writes formatted output to a file.
 *
 * Warning! This function use an internal buffer to for the format string
 * before writing to file. Any string longer than FPRINTF_BUFFER_SIZE (1024)
 * will be cut off.
 *
 * It is adviced to use rte_fs_fputs directly, to have proper control over this
 * buffer.
 *
 * @param file Pointer to a RTE_FILE that identifies the output stream.
 * @param format Pointer to a null-terminated multibyte character string that
 * specifies the format of the output.
 * @param ...    Variable number of arguments, depending on the format string.'
 * @return -1 if failure. Positive number if success.
 */
int rte_fs_fprintf (RTE_FILE * file, const char * format, ...);

#ifdef __cplusplus
}
#endif

#endif // RTE_FS_H
