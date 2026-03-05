#ifndef CLAUDE_LIBFS_H
#define CLAUDE_LIBFS_H

/*
 * Garrett_libFS.h
 * User-Level File System Library (libFS)
 * Author: Garrett
 *
 * Header file defining the libFS API for file system emulation.
 * Provides file creation, opening, reading, writing, closing, and deletion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* ─────────────────────────────────────────────
   Constants & Return Codes
   ───────────────────────────────────────────── */
#define LIBFS_SUCCESS        0
#define LIBFS_ERROR         -1
#define LIBFS_MAX_FILES     64       /* max simultaneously open files */
#define LIBFS_MAX_PATH     256       /* max path/filename length       */
#define LIBFS_MAX_BUFFER  4096       /* max read/write buffer size     */

/* File open mode flags */
#define FS_MODE_READ        0x01
#define FS_MODE_WRITE       0x02
#define FS_MODE_APPEND      0x04
#define FS_MODE_READWRITE   (FS_MODE_READ | FS_MODE_WRITE)

/* ─────────────────────────────────────────────
   File Descriptor Structure
   ───────────────────────────────────────────── */
typedef struct {
    int      fd_id;                  /* internal file descriptor ID   */
    char     path[LIBFS_MAX_PATH];   /* file path                     */
    int      mode;                   /* open mode flags               */
    FILE    *fp;                     /* underlying C file pointer      */
    int      is_open;                /* 1 = open, 0 = closed          */
    long     position;               /* current read/write position   */
} FSFile;

/* ─────────────────────────────────────────────
   Library Lifecycle
   ───────────────────────────────────────────── */

/*
 * libFS_init – Initialize the library. Must be called before any other
 *              libFS function.
 * Returns: LIBFS_SUCCESS on success, LIBFS_ERROR on failure.
 */
int libFS_init(void);

/*
 * libFS_shutdown – Clean up all open file handles and free resources.
 * Returns: LIBFS_SUCCESS on success.
 */
int libFS_shutdown(void);

/* ─────────────────────────────────────────────
   File Access Calls
   ───────────────────────────────────────────── */

/*
 * fileCreate – Create a new file at the given path.
 *   If the file already exists it will be truncated (overwritten).
 * @param path – null-terminated file path string.
 * Returns: LIBFS_SUCCESS on success, LIBFS_ERROR on failure.
 */
int fileCreate(const char *path);

/*
 * fileOpen – Open an existing file and return a file descriptor.
 * @param path – null-terminated file path string.
 * @param mode – one of FS_MODE_READ, FS_MODE_WRITE, FS_MODE_APPEND,
 *               or FS_MODE_READWRITE.
 * Returns: non-negative fd_id on success, LIBFS_ERROR on failure.
 */
int fileOpen(const char *path, int mode);

/*
 * fileRead – Read up to `size` bytes from an open file into `buffer`.
 * @param fd     – file descriptor returned by fileOpen.
 * @param buffer – destination buffer (caller-allocated).
 * @param size   – maximum bytes to read.
 * Returns: number of bytes actually read, or LIBFS_ERROR on failure.
 */
int fileRead(int fd, char *buffer, int size);

/*
 * fileWrite – Write `size` bytes from `buffer` into an open file.
 * @param fd     – file descriptor returned by fileOpen.
 * @param buffer – source data buffer.
 * @param size   – number of bytes to write.
 * Returns: number of bytes written, or LIBFS_ERROR on failure.
 */
int fileWrite(int fd, const char *buffer, int size);

/*
 * fileClose – Close an open file and release its descriptor slot.
 * @param fd – file descriptor returned by fileOpen.
 * Returns: LIBFS_SUCCESS on success, LIBFS_ERROR on failure.
 */
int fileClose(int fd);

/*
 * fileDelete – Delete a file from the (simulated) disk.
 *   The file must NOT be currently open.
 * @param path – null-terminated file path string.
 * Returns: LIBFS_SUCCESS on success, LIBFS_ERROR on failure.
 */
int fileDelete(const char *path);

/* ─────────────────────────────────────────────
   Utility / Diagnostic Helpers
   ───────────────────────────────────────────── */

/*
 * fileExists – Check whether a file exists.
 * Returns: 1 if it exists, 0 if not.
 */
int fileExists(const char *path);

/*
 * libFS_perror – Print a human-readable error message to stderr,
 *                prefixed with `msg`.
 */
void libFS_perror(const char *msg);

#endif /* CLAUDE_LIBFS_H */
