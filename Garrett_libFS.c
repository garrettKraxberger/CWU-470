/*
 * Garrett_libFS.c
 * User-Level File System Library (libFS) – Implementation
 * Author: Garrett
 *
 * Implements the libFS API declared in Garrett_libFS.h.
 * Uses standard C I/O (FILE*) to emulate disk-level operations,
 * and maintains an internal file-descriptor table for tracking
 * open files.
 */

#include "Garrett_libFS.h"

/* ─────────────────────────────────────────────
   Internal State
   ───────────────────────────────────────────── */

/* Global file-descriptor table */
static FSFile fd_table[LIBFS_MAX_FILES];

/* Tracks whether libFS_init() has been called */
static int libfs_initialized = 0;

/* Last error message string */
static char last_error[256] = "";

/* ─────────────────────────────────────────────
   Internal Helpers
   ───────────────────────────────────────────── */

/* Set the internal error message */
static void set_error(const char *msg) {
    strncpy(last_error, msg, sizeof(last_error) - 1);
    last_error[sizeof(last_error) - 1] = '\0';
}

/* Find a free slot in the fd_table; returns index or LIBFS_ERROR */
static int find_free_fd(void) {
    for (int i = 0; i < LIBFS_MAX_FILES; i++) {
        if (!fd_table[i].is_open) {
            return i;
        }
    }
    return LIBFS_ERROR;
}

/* Validate that fd is in range and currently open */
static int validate_fd(int fd) {
    if (fd < 0 || fd >= LIBFS_MAX_FILES) {
        set_error("Invalid file descriptor: out of range");
        return LIBFS_ERROR;
    }
    if (!fd_table[fd].is_open) {
        set_error("Invalid file descriptor: file is not open");
        return LIBFS_ERROR;
    }
    return LIBFS_SUCCESS;
}

/* ─────────────────────────────────────────────
   Library Lifecycle
   ───────────────────────────────────────────── */

int libFS_init(void) {
    if (libfs_initialized) {
        set_error("libFS already initialized");
        return LIBFS_SUCCESS;   /* idempotent – not a fatal error */
    }

    /* Zero out the entire fd_table */
    memset(fd_table, 0, sizeof(fd_table));

    /* Mark every slot as free */
    for (int i = 0; i < LIBFS_MAX_FILES; i++) {
        fd_table[i].is_open = 0;
        fd_table[i].fd_id   = i;
        fd_table[i].fp      = NULL;
    }

    libfs_initialized = 1;
    printf("[libFS] Library initialized. Max open files: %d\n", LIBFS_MAX_FILES);
    return LIBFS_SUCCESS;
}

int libFS_shutdown(void) {
    if (!libfs_initialized) {
        set_error("libFS not initialized");
        return LIBFS_ERROR;
    }

    /* Close any files that were left open */
    int closed = 0;
    for (int i = 0; i < LIBFS_MAX_FILES; i++) {
        if (fd_table[i].is_open && fd_table[i].fp != NULL) {
            fclose(fd_table[i].fp);
            fd_table[i].is_open = 0;
            fd_table[i].fp      = NULL;
            closed++;
        }
    }

    if (closed > 0) {
        printf("[libFS] Warning: %d file(s) were still open at shutdown and have been closed.\n",
               closed);
    }

    libfs_initialized = 0;
    printf("[libFS] Library shut down cleanly.\n");
    return LIBFS_SUCCESS;
}

/* ─────────────────────────────────────────────
   File Access Calls
   ───────────────────────────────────────────── */

/*
 * fileCreate
 * Creates a new empty file (or truncates an existing one).
 * We open with "w" mode, write nothing, then close immediately.
 */
int fileCreate(const char *path) {
    if (!libfs_initialized) {
        set_error("libFS not initialized – call libFS_init() first");
        return LIBFS_ERROR;
    }
    if (path == NULL || strlen(path) == 0) {
        set_error("fileCreate: invalid path (NULL or empty)");
        return LIBFS_ERROR;
    }
    if (strlen(path) >= LIBFS_MAX_PATH) {
        set_error("fileCreate: path exceeds LIBFS_MAX_PATH");
        return LIBFS_ERROR;
    }

    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        snprintf(last_error, sizeof(last_error),
                 "fileCreate: failed to create '%s': %s", path, strerror(errno));
        return LIBFS_ERROR;
    }

    fclose(fp);
    printf("[libFS] fileCreate: '%s' created successfully.\n", path);
    return LIBFS_SUCCESS;
}

/*
 * fileOpen
 * Opens an existing file in the requested mode and returns an fd_id.
 */
int fileOpen(const char *path, int mode) {
    if (!libfs_initialized) {
        set_error("libFS not initialized – call libFS_init() first");
        return LIBFS_ERROR;
    }
    if (path == NULL || strlen(path) == 0) {
        set_error("fileOpen: invalid path");
        return LIBFS_ERROR;
    }

    /* Determine the fopen mode string */
    const char *fmode = NULL;
    if (mode == FS_MODE_READ)      fmode = "r";
    else if (mode == FS_MODE_WRITE)     fmode = "r+";  /* write without truncate */
    else if (mode == FS_MODE_APPEND)    fmode = "a";
    else if (mode == FS_MODE_READWRITE) fmode = "r+";
    else {
        set_error("fileOpen: unsupported mode flags");
        return LIBFS_ERROR;
    }

    /* Find a free descriptor slot */
    int slot = find_free_fd();
    if (slot == LIBFS_ERROR) {
        set_error("fileOpen: too many open files (fd_table full)");
        return LIBFS_ERROR;
    }

    FILE *fp = fopen(path, fmode);
    if (fp == NULL) {
        snprintf(last_error, sizeof(last_error),
                 "fileOpen: failed to open '%s' (mode '%s'): %s",
                 path, fmode, strerror(errno));
        return LIBFS_ERROR;
    }

    /* Populate the fd_table entry */
    fd_table[slot].is_open  = 1;
    fd_table[slot].fp       = fp;
    fd_table[slot].mode     = mode;
    fd_table[slot].position = 0;
    strncpy(fd_table[slot].path, path, LIBFS_MAX_PATH - 1);
    fd_table[slot].path[LIBFS_MAX_PATH - 1] = '\0';

    printf("[libFS] fileOpen: '%s' opened with fd=%d (mode=0x%02X).\n",
           path, slot, mode);
    return slot;   /* return fd_id */
}

/*
 * fileRead
 * Reads up to `size` bytes from the file associated with `fd`.
 * Null-terminates the buffer if there is space.
 */
int fileRead(int fd, char *buffer, int size) {
    if (validate_fd(fd) == LIBFS_ERROR) return LIBFS_ERROR;
    if (buffer == NULL || size <= 0) {
        set_error("fileRead: invalid buffer or size");
        return LIBFS_ERROR;
    }
    if (!(fd_table[fd].mode & FS_MODE_READ)) {
        set_error("fileRead: file not opened for reading");
        return LIBFS_ERROR;
    }

    int bytes_read = (int)fread(buffer, 1, (size_t)(size - 1), fd_table[fd].fp);
    buffer[bytes_read] = '\0';   /* null-terminate */

    if (bytes_read < 0) {
        snprintf(last_error, sizeof(last_error),
                 "fileRead: read error on fd=%d: %s", fd, strerror(errno));
        return LIBFS_ERROR;
    }

    fd_table[fd].position += bytes_read;
    printf("[libFS] fileRead: read %d byte(s) from fd=%d.\n", bytes_read, fd);
    return bytes_read;
}

/*
 * fileWrite
 * Writes `size` bytes from `buffer` into the file associated with `fd`.
 */
int fileWrite(int fd, const char *buffer, int size) {
    if (validate_fd(fd) == LIBFS_ERROR) return LIBFS_ERROR;
    if (buffer == NULL || size <= 0) {
        set_error("fileWrite: invalid buffer or size");
        return LIBFS_ERROR;
    }
    if (!(fd_table[fd].mode & FS_MODE_WRITE) &&
        !(fd_table[fd].mode & FS_MODE_APPEND)) {
        set_error("fileWrite: file not opened for writing");
        return LIBFS_ERROR;
    }

    int bytes_written = (int)fwrite(buffer, 1, (size_t)size, fd_table[fd].fp);
    fflush(fd_table[fd].fp);   /* ensure data is flushed to disk layer */

    if (bytes_written < size) {
        snprintf(last_error, sizeof(last_error),
                 "fileWrite: partial write on fd=%d (%d of %d bytes): %s",
                 fd, bytes_written, size, strerror(errno));
        return LIBFS_ERROR;
    }

    fd_table[fd].position += bytes_written;
    printf("[libFS] fileWrite: wrote %d byte(s) to fd=%d.\n", bytes_written, fd);
    return bytes_written;
}

/*
 * fileClose
 * Flushes and closes the file, freeing the descriptor slot.
 */
int fileClose(int fd) {
    if (validate_fd(fd) == LIBFS_ERROR) return LIBFS_ERROR;

    fflush(fd_table[fd].fp);
    if (fclose(fd_table[fd].fp) != 0) {
        snprintf(last_error, sizeof(last_error),
                 "fileClose: fclose failed for fd=%d: %s", fd, strerror(errno));
        return LIBFS_ERROR;
    }

    printf("[libFS] fileClose: fd=%d ('%s') closed.\n",
           fd, fd_table[fd].path);

    /* Clear the slot */
    fd_table[fd].is_open  = 0;
    fd_table[fd].fp       = NULL;
    fd_table[fd].mode     = 0;
    fd_table[fd].position = 0;
    fd_table[fd].path[0]  = '\0';

    return LIBFS_SUCCESS;
}

/*
 * fileDelete
 * Removes a file from the disk. The file must not be currently open.
 */
int fileDelete(const char *path) {
    if (!libfs_initialized) {
        set_error("libFS not initialized");
        return LIBFS_ERROR;
    }
    if (path == NULL || strlen(path) == 0) {
        set_error("fileDelete: invalid path");
        return LIBFS_ERROR;
    }

    /* Ensure the file is not open in our fd_table */
    for (int i = 0; i < LIBFS_MAX_FILES; i++) {
        if (fd_table[i].is_open &&
            strncmp(fd_table[i].path, path, LIBFS_MAX_PATH) == 0) {
            snprintf(last_error, sizeof(last_error),
                     "fileDelete: '%s' is currently open (fd=%d). Close it first.",
                     path, i);
            return LIBFS_ERROR;
        }
    }

    if (remove(path) != 0) {
        snprintf(last_error, sizeof(last_error),
                 "fileDelete: failed to delete '%s': %s", path, strerror(errno));
        return LIBFS_ERROR;
    }

    printf("[libFS] fileDelete: '%s' deleted successfully.\n", path);
    return LIBFS_SUCCESS;
}

/* ─────────────────────────────────────────────
   Utility / Diagnostic Helpers
   ───────────────────────────────────────────── */

int fileExists(const char *path) {
    if (path == NULL) return 0;
    FILE *fp = fopen(path, "r");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

void libFS_perror(const char *msg) {
    if (msg && strlen(msg) > 0)
        fprintf(stderr, "[libFS ERROR] %s: %s\n", msg, last_error);
    else
        fprintf(stderr, "[libFS ERROR] %s\n", last_error);
}
