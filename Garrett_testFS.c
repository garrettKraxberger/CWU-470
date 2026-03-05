/*
 * Garrett_testFS.c
 * Test Application for libFS – User-Level File System Library
 * Author: Garrett
 *
 * Menu-driven program that demonstrates all six libFS operations:
 *   fileCreate  →  fileOpen  →  fileWrite  →  fileClose
 *   fileOpen    →  fileRead  →  fileClose
 *   fileDelete
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Garrett_libFS.h"

/* ─────────────────────────────────────────────
   Constants
   ───────────────────────────────────────────── */
#define TARGET_FILE "Claude_Introduction.txt"

/* Introduction text (2 paragraphs, 4 sentences) */
#define INTRO_TEXT \
    "Hello! My name is Claude, and I am an AI assistant created by Anthropic.\n" \
    "I am designed to be helpful, harmless, and honest in all of my interactions.\n" \
    "\n" \
    "I love exploring topics ranging from computer science to creative writing.\n" \
    "Working on projects like this libFS library brings me great joy, as it " \
    "combines low-level systems programming with practical software design.\n"

/* ─────────────────────────────────────────────
   UI Helpers
   ───────────────────────────────────────────── */

static void print_banner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║          libFS Test Application  –  Claude           ║\n");
    printf("║          User-Level File System Library Demo         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n");
}

static void print_separator(void) {
    printf("──────────────────────────────────────────────────────\n");
}

static void print_menu(void) {
    print_separator();
    printf("  MENU – select an operation:\n");
    print_separator();
    printf("  [1] fileCreate  – Create '%s'\n", TARGET_FILE);
    printf("  [2] fileWrite   – Write introduction to the file\n");
    printf("  [3] fileRead    – Read & print file contents\n");
    printf("  [4] fileDelete  – Delete the file\n");
    printf("  [5] Run ALL     – Execute steps 1-3 automatically\n");
    printf("  [0] Exit\n");
    print_separator();
    printf("  Enter choice: ");
}

static void pause_prompt(void) {
    printf("\n  Press [ENTER] to continue...");
    while (getchar() != '\n');  /* consume newline */
}

/* ─────────────────────────────────────────────
   Operation Functions
   ───────────────────────────────────────────── */

/* Step 1: Create the file */
static void demo_create(void) {
    printf("\n>>> STEP 1: fileCreate\n");
    print_separator();
    printf("  Creating file: '%s'\n", TARGET_FILE);

    if (fileCreate(TARGET_FILE) == LIBFS_SUCCESS) {
        printf("  ✓ File created successfully.\n");
    } else {
        libFS_perror("fileCreate failed");
    }
}

/* Step 2: Open → Write → Close */
static void demo_write(void) {
    printf("\n>>> STEP 2: fileOpen + fileWrite + fileClose\n");
    print_separator();

    /* Check file exists first */
    if (!fileExists(TARGET_FILE)) {
        printf("  ✗ File does not exist. Please run fileCreate (option 1) first.\n");
        return;
    }

    printf("  Opening '%s' for writing...\n", TARGET_FILE);
    int fd = fileOpen(TARGET_FILE, FS_MODE_READWRITE);
    if (fd == LIBFS_ERROR) {
        libFS_perror("fileOpen failed");
        return;
    }
    printf("  ✓ File opened (fd = %d).\n", fd);

    /* Write the introduction */
    const char *content = INTRO_TEXT;
    int len = (int)strlen(content);

    printf("  Writing introduction (%d bytes)...\n", len);
    int written = fileWrite(fd, content, len);
    if (written == LIBFS_ERROR) {
        libFS_perror("fileWrite failed");
        fileClose(fd);
        return;
    }
    printf("  ✓ Wrote %d byte(s).\n", written);

    /* Close */
    printf("  Closing file...\n");
    if (fileClose(fd) == LIBFS_SUCCESS) {
        printf("  ✓ File closed.\n");
    } else {
        libFS_perror("fileClose failed");
    }
}

/* Step 3: Open → Read → Print → Close */
static void demo_read(void) {
    printf("\n>>> STEP 3: fileOpen + fileRead + fileClose\n");
    print_separator();

    if (!fileExists(TARGET_FILE)) {
        printf("  ✗ File does not exist. Please run steps 1 & 2 first.\n");
        return;
    }

    printf("  Opening '%s' for reading...\n", TARGET_FILE);
    int fd = fileOpen(TARGET_FILE, FS_MODE_READ);
    if (fd == LIBFS_ERROR) {
        libFS_perror("fileOpen failed");
        return;
    }
    printf("  ✓ File opened (fd = %d).\n", fd);

    /* Read contents */
    char buffer[LIBFS_MAX_BUFFER];
    memset(buffer, 0, sizeof(buffer));

    printf("  Reading file contents...\n");
    int bytes = fileRead(fd, buffer, (int)sizeof(buffer));
    if (bytes == LIBFS_ERROR) {
        libFS_perror("fileRead failed");
        fileClose(fd);
        return;
    }

    /* Print the contents */
    printf("\n  ┌─── File Contents (%d bytes) ─────────────────────\n", bytes);
    printf("  │\n");
    /* Print each line with a leading "  │ " */
    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        printf("  │  %s\n", line);
        line = strtok(NULL, "\n");
    }
    printf("  │\n");
    printf("  └──────────────────────────────────────────────────\n");

    /* Close */
    printf("\n  Closing file...\n");
    if (fileClose(fd) == LIBFS_SUCCESS) {
        printf("  ✓ File closed.\n");
    } else {
        libFS_perror("fileClose failed");
    }
}

/* Step 4: Delete */
static void demo_delete(void) {
    printf("\n>>> STEP 4: fileDelete\n");
    print_separator();

    if (!fileExists(TARGET_FILE)) {
        printf("  File '%s' does not exist (already deleted or never created).\n",
               TARGET_FILE);
        return;
    }

    printf("  Attempting to delete '%s'...\n", TARGET_FILE);

    /* Confirm with the user */
    printf("  Are you sure you want to delete this file? (y/n): ");
    int ch = getchar();
    while (getchar() != '\n');   /* flush rest of line */

    if (ch != 'y' && ch != 'Y') {
        printf("  Delete cancelled.\n");
        return;
    }

    if (fileDelete(TARGET_FILE) == LIBFS_SUCCESS) {
        printf("  ✓ File deleted successfully.\n");
    } else {
        libFS_perror("fileDelete failed");
    }
}

/* Run all steps in sequence */
static void demo_run_all(void) {
    printf("\n>>> RUN ALL – Executing complete demo sequence\n");
    print_separator();

    demo_create();
    pause_prompt();

    demo_write();
    pause_prompt();

    demo_read();
    pause_prompt();

    printf("\n  All steps completed. File '%s' now exists on disk.\n", TARGET_FILE);
    printf("  You may use option [4] to delete it when ready.\n");
}

/* ─────────────────────────────────────────────
   Main
   ───────────────────────────────────────────── */
int main(void) {
    print_banner();

    /* Initialize libFS */
    printf("  Initializing libFS...\n");
    if (libFS_init() != LIBFS_SUCCESS) {
        fprintf(stderr, "  FATAL: libFS_init() failed. Exiting.\n");
        return EXIT_FAILURE;
    }
    printf("  ✓ libFS ready.\n\n");

    int running = 1;
    while (running) {
        print_menu();

        int choice = 0;
        if (scanf("%d", &choice) != 1) {
            /* Clear bad input */
            while (getchar() != '\n');
            printf("  Invalid input. Please enter a number.\n");
            continue;
        }
        while (getchar() != '\n');   /* consume trailing newline */

        switch (choice) {
            case 1: demo_create();   pause_prompt(); break;
            case 2: demo_write();    pause_prompt(); break;
            case 3: demo_read();     pause_prompt(); break;
            case 4: demo_delete();   pause_prompt(); break;
            case 5: demo_run_all();  pause_prompt(); break;
            case 0:
                printf("\n  Exiting – shutting down libFS...\n");
                running = 0;
                break;
            default:
                printf("\n  Unknown option '%d'. Please choose 0–5.\n", choice);
                break;
        }
    }

    /* Graceful shutdown */
    libFS_shutdown();
    printf("  Goodbye!\n\n");
    return EXIT_SUCCESS;
}
