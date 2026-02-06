/*
 * Garrett Kraxberger
 * Lab2.c - Process Management Program
 * 
 * This program demonstrates process creation, process execution, and process 
 * synchronization using fork(), execvp(), and waitpid() system calls.
 * 
 * The program creates 15 child processes, each executing different commands or
 * exhibiting different termination behaviors (normal exit, exit with error, 
 * signal-based termination). The parent process then waits for all children in 
 * creation order and reports on their termination status.
 */

#include <unistd.h>   /* for fork(), execvp(), _exit() */
#include <sys/wait.h> /* for waitpid(), WIFEXITED(), WTERMSIG(), etc. */
#include <stdio.h>    /* for printf(), perror(), fflush() */
#include <stdlib.h>   /* for exit(), EXIT_SUCCESS, EXIT_FAILURE */
#include <signal.h>   /* for abort(), strsignal() */
/* strsignal() prototype is provided by <string.h> on many platforms */
#include <string.h>   /* for strsignal() */

/* Total number of child processes to create */
#define NCHILD 15

/* Enumeration to classify job types for children */
typedef enum { 
    JOB_EXEC,    /* Execute a valid Linux command */
    JOB_INVALID, /* Attempt to execute a non-existent command (will fail with exit code 127) */
    JOB_ABORT    /* Call abort() to terminate via signal (SIGABRT) */
} job_type_t; 

int main(void) {
    /* Array to store the PIDs of all created child processes */
    pid_t child_pids[NCHILD];
    
    /* Counter for number of children successfully created */
    int created = 0;

    /* Print the parent process ID */
    printf("Parent PID: %d\n", getpid());

    /* 
     * Define command arguments for each child process.
     * Each cmd[] is a NULL-terminated array of strings (char pointers).
     * The first element is the command name, followed by its arguments.
     * NULL terminator signals the end of the argument list.
     */
    
    /* 11 valid commands that will execute successfully */
    char *cmd0[] = {"ls", "-l", NULL};           /* List files with details */
    char *cmd1[] = {"pwd", NULL};                /* Print working directory */
    char *cmd2[] = {"date", NULL};               /* Print current date and time */
    char *cmd3[] = {"whoami", NULL};             /* Print current user name */
    char *cmd4[] = {"id", NULL};                 /* Print user and group IDs */
    char *cmd5[] = {"uname", "-a", NULL};       /* Print system information */
    char *cmd6[] = {"echo", "Hello", "Garrett", NULL}; /* Required: echo "Hello <Your Name>" */
    char *cmd7[] = {"sleep", "0", NULL};        /* Sleep for 0 seconds (exits immediately) */
    char *cmd8[] = {"hostname", NULL};          /* Print system hostname */
    char *cmd9[] = {"uptime", NULL};            /* Print system uptime */
    char *cmd10[] = {"env", NULL};              /* Print environment variables */

    /* 
     * Two invalid commands that do not exist in the system.
     * When execvp() tries to execute these, it will fail with "command not found"
     * and the child process will exit with code 127.
     */
    char *bad0[] = {"nonexistentcmd1", NULL};
    char *bad1[] = {"nonexistentcmd2", NULL};

    /* 
     * Create two parallel arrays:
     * - types[]: Specifies the job type for each child (JOB_EXEC, JOB_INVALID, JOB_ABORT)
     * - argvs[]: Points to the command arguments for each child
     */
    job_type_t types[NCHILD];
    char **argvs[NCHILD];

    /* 
     * Assign job types and commands to the arrays.
     * Each child index (0-14) gets a specific job type and command.
     */
    
    /* Indices 0-10: Valid commands to execute */
    types[0] = JOB_EXEC; argvs[0] = cmd0;
    types[1] = JOB_EXEC; argvs[1] = cmd1;
    types[2] = JOB_EXEC; argvs[2] = cmd2;
    types[3] = JOB_EXEC; argvs[3] = cmd3;
    types[4] = JOB_EXEC; argvs[4] = cmd4;
    types[5] = JOB_EXEC; argvs[5] = cmd5;
    types[6] = JOB_EXEC; argvs[6] = cmd6; /* This child will echo "Hello Garrett" */
    types[7] = JOB_EXEC; argvs[7] = cmd7;
    types[8] = JOB_EXEC; argvs[8] = cmd8;
    types[9] = JOB_EXEC; argvs[9] = cmd9;
    types[10] = JOB_EXEC; argvs[10] = cmd10;

    /* Indices 11-12: Invalid commands (will fail and exit with code 127) */
    types[11] = JOB_INVALID; argvs[11] = bad0;
    types[12] = JOB_INVALID; argvs[12] = bad1;

    /* Indices 13-14: Will call abort() to terminate by signal (SIGABRT) */
    types[13] = JOB_ABORT; argvs[13] = NULL;
    types[14] = JOB_ABORT; argvs[14] = NULL;

    /* 
     * PHASE 1: CREATE CHILD PROCESSES
     * Loop to create NCHILD child processes using fork()
     */
    for (int i = 0; i < NCHILD; ++i) {
        /* fork() creates a new process (child) that is a duplicate of the caller (parent) */
        pid_t pid = fork();
        
        /* Error handling: fork() returns -1 on failure */
        if (pid < 0) {
            perror("fork failed");
            /* Cannot reliably continue if fork fails, so exit immediately */
            exit(EXIT_FAILURE);
        } 
        /* Child process: fork() returns 0 */
        else if (pid == 0) {
            /* This code runs in the child process */
            printf("Child %2d: PID=%d will ", i, getpid());
            
            /* Check the job type for this child */
            if (types[i] == JOB_EXEC) {
                /* 
                 * JOB_EXEC: Execute a valid Linux command
                 * Print the command about to be executed
                 */
                char **a = argvs[i];
                printf("execvp: ");
                /* Print all arguments (command name and its flags/options) */
                for (int k = 0; a[k] != NULL; ++k) {
                    if (k) putchar(' ');
                    fputs(a[k], stdout);
                }
                putchar('\n');
                fflush(stdout); /* Ensure output is printed before execvp replaces the process */

                /* 
                 * execvp() replaces the current process image with the new program.
                 * It searches for the command in PATH.
                 * If successful, this process becomes the executed command.
                 * This line will never return on success.
                 */
                execvp(a[0], a);
                
                /* 
                 * If execution reaches here, execvp() failed.
                 * Print the error message and exit with code 127 (command not found).
                 */
                perror("execvp failed");
                _exit(127);
            } 
            else if (types[i] == JOB_INVALID) {
                /* 
                 * JOB_INVALID: Attempt to execute a non-existent command
                 * This will fail, demonstrating error handling
                 */
                char **a = argvs[i];
                printf("execvp (invalid command): %s\n", a[0]);
                fflush(stdout);

                /* 
                 * Try to execute a command that doesn't exist.
                 * This will always fail and return to the next line.
                 */
                execvp(a[0], a);
                
                /* Print error and exit with code 127 */
                perror("execvp failed (expected for invalid command)");
                _exit(127);
            } 
            else if (types[i] == JOB_ABORT) {
                /* 
                 * JOB_ABORT: Terminate the process via signal
                 * abort() sends SIGABRT to the current process,
                 * causing it to terminate abnormally with a signal.
                 */
                printf("abort() (will terminate by signal SIGABRT)\n");
                fflush(stdout);
                
                /* 
                 * abort() raises SIGABRT, which terminates the process.
                 * The process will not continue past this line.
                 */
                abort();
                /* Code after abort() is never reached */
            }
            
            /* Safety: should never reach here, but exit if we do */
            _exit(EXIT_FAILURE);
        } 
        /* Parent process: fork() returns the child's PID (a positive value) */
        else {
            /* This code runs in the parent process */
            /* Store the child's PID in the array for later synchronization */
            child_pids[i] = pid;
            created++;
        }
    }

    /* 
     * PHASE 2: WAIT FOR CHILD PROCESSES
     * Parent process waits for all children to terminate.
     * Waiting happens in creation order (not completion order),
     * which is important for synchronized, predictable output.
     */
    
    /* Counters for the summary statistics */
    int exited_zero = 0;        /* Children that exited normally with code 0 */
    int exited_nonzero = 0;     /* Children that exited normally with non-zero code */
    int terminated_signal = 0;  /* Children terminated by a signal */

    /* Wait for each child in creation order */
    for (int i = 0; i < created; ++i) {
        int status; /* Variable to store the child's termination status */
        
        /* 
         * waitpid() waits for the specific child process to terminate.
         * - First argument: child PID to wait for
         * - Second argument: pointer to status variable
         * - Third argument: 0 means wait until child terminates (no options)
         * Returns the PID of the child on success, -1 on error
         */
        pid_t w = waitpid(child_pids[i], &status, 0);
        
        /* Error handling */
        if (w == -1) {
            perror("waitpid failed");
            continue;
        }
        
        /* Print which child we just waited for */
        printf("Parent: waited for child index %2d pid %d -> ", i, (int)child_pids[i]);
        
        /* 
         * Use status macros to determine how the child terminated.
         * The status value encodes exit code or signal information.
         */
        
        /* WIFEXITED: Check if the child exited normally (via exit() or return) */
        if (WIFEXITED(status)) {
            /* WEXITSTATUS: Extract the exit code from the status value */
            int code = WEXITSTATUS(status);
            printf("exited normally, exit code=%d\n", code);
            
            /* Update statistics */
            if (code == 0) {
                exited_zero++;
            } else {
                exited_nonzero++;
            }
        } 
        /* WIFSIGNALED: Check if the child was terminated by a signal */
        else if (WIFSIGNALED(status)) {
            /* WTERMSIG: Extract the signal number that terminated the child */
            int sig = WTERMSIG(status);
            /* 
             * strsignal(): Convert signal number to a human-readable string.
             * For example: 6 becomes "Aborted", 15 becomes "Terminated", etc.
             */
            printf("terminated by signal %d (%s)\n", sig, strsignal(sig));
            terminated_signal++;
        } 
        /* Unexpected termination status (rare) */
        else {
            printf("ended abnormally (unknown reason)\n");
        }
    }

    /* 
     * PHASE 3: PRINT SUMMARY
     * After all children have been waited for, print summary statistics.
     */
    printf("\nSummary:\n");
    printf("  Total children created: %d\n", created);
    printf("  Exited normally with exit code 0: %d\n", exited_zero);
    printf("  Exited normally with non-zero exit code: %d\n", exited_nonzero);
    printf("  Terminated by signal: %d\n", terminated_signal);

    /* Program terminates successfully */
    return EXIT_SUCCESS;
}
