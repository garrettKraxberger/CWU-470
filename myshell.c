/*
 * myshell.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* Configuration limits */
#define MAX_INPUT 1024 /* maximum characters in an input line */
#define MAX_ARGS 64    /* maximum number of token/arguments parsed */

/* Function prototypes with brief purpose notes */
void parse_command(char *input, char **args, char **input_file, char **output_file, int *append_mode);
    /* Parse `input` into argv-style `args`, and fill in optional
     * `input_file` and `output_file` when redirection operators are seen.
     * `append_mode` is set to 1 when '>>' is used. */

void execute_command(char **args, char *input_file, char *output_file, int append_mode);
    /* Forks and executes `args`; sets up redirection before exec. */

int is_builtin(char **args);
    /* Returns non-zero if `args[0]` is a built-in command handled by shell. */

void handle_builtin(char **args);
    /* Execute built-in commands such as `cd` and `exit`. */

void trim_quotes(char *str);
    /* Remove surrounding double quotes from `str` if present. */

int main() {
    char input[MAX_INPUT];        /* input buffer for a single command line */
    char *args[MAX_ARGS];         /* parsed arguments (NULL-terminated) */
    char *input_file = NULL;      /* filename for '<' redirection */
    char *output_file = NULL;     /* filename for '>' or '>>' redirection */
    int append_mode = 0;          /* non-zero if using '>>' */

    /* REPL loop: read, parse, execute */
    while (1) {
        /* Print prompt */
        printf("myshell> ");
        fflush(stdout);

        /* Read a line from stdin; if EOF, exit loop */
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; /* CTRL-D or EOF */
        }

        /* Strip trailing newline (fgets keeps it) */
        input[strcspn(input, "\n")] = 0;

        /* Ignore empty lines to avoid spawning processes for blank input */
        if (strlen(input) == 0) {
            continue;
        }

        /* Reset redirection state for each new command */
        input_file = NULL;
        output_file = NULL;
        append_mode = 0;

        /* Tokenize the command line and detect redirections */
        parse_command(input, args, &input_file, &output_file, &append_mode);

        /* No command parsed (e.g., only redirections) -> continue */
        if (args[0] == NULL) {
            continue;
        }

        /* Handle built-in commands in the parent process */
        if (is_builtin(args)) {
            handle_builtin(args);
        } else {
            /* Launch external command in a child process */
            execute_command(args, input_file, output_file, append_mode);
        }
    }

    return 0;
}

/*
 * parse_command
 * Walks the input line and extracts tokens (arguments) respecting
 * double-quoted strings. It also recognizes '<', '>' and '>>' for
 * input/output redirection. Allocates strings for each argument and
 * for filenames returned via `input_file`/`output_file` (caller must free).
 */
void parse_command(char *input, char **args, char **input_file, char **output_file, int *append_mode) {
    int i = 0;                   /* index into args[] */
    int in_quotes = 0;           /* whether parser is currently inside quotes */
    char *token_start = NULL;    /* start pointer for a filename token */
    char temp_token[MAX_INPUT];  /* temporary buffer for a single argument */
    int temp_idx = 0;

    /* Initialize args to NULL so later code can check args[0] */
    for (int j = 0; j < MAX_ARGS; j++) {
        args[j] = NULL;
    }

    char *ptr = input;

    while (*ptr != '\0') {
        /* Skip whitespace between tokens */
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (*ptr == '\0') break; /* end of input */

        /* Handle input redirection: < filename */
        if (*ptr == '<' && !in_quotes) {
            ptr++; /* skip '<' */
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            token_start = ptr;
            /* filename ends at whitespace or another redirection */
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '>' && *ptr != '<') {
                ptr++;
            }
            int len = ptr - token_start;
            *input_file = malloc(len + 1);
            strncpy(*input_file, token_start, len);
            (*input_file)[len] = '\0';
            trim_quotes(*input_file); /* allow quoted filenames */
            continue;
        } else if (*ptr == '>' && !in_quotes) {
            /* Handle output redirection: > filename  OR  >> filename */
            ptr++; /* skip '>' */
            if (*ptr == '>') {
                /* '>>' found -> append mode */
                *append_mode = 1;
                ptr++;
            }
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            token_start = ptr;
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '>' && *ptr != '<') {
                ptr++;
            }
            int len = ptr - token_start;
            *output_file = malloc(len + 1);
            strncpy(*output_file, token_start, len);
            (*output_file)[len] = '\0';
            trim_quotes(*output_file);
            continue;
        }

        /* Parse a normal token (argument), respecting double quotes */
        temp_idx = 0;
        in_quotes = 0;

        while (*ptr != '\0') {
            if (*ptr == '"') {
                /* Toggle quote state and skip the quote character */
                in_quotes = !in_quotes;
                ptr++;
                continue;
            }

            /* If we're not inside quotes, stop at whitespace or redirection */
            if (!in_quotes && (*ptr == ' ' || *ptr == '\t' || *ptr == '<' || *ptr == '>')) {
                break;
            }

            /* Append character to the temporary token buffer */
            temp_token[temp_idx++] = *ptr;
            ptr++;
        }

        temp_token[temp_idx] = '\0';

        if (temp_idx > 0) {
            /* Allocate and store the token in args[] */
            args[i] = malloc(strlen(temp_token) + 1);
            strcpy(args[i], temp_token);
            i++;
        }
    }

    /* NULL-terminate the argv array */
    args[i] = NULL;
}

/* Remove surrounding double quotes from `str` if both ends are quoted.
 * This is used to allow filenames or arguments like "my file.txt" to be
 * stored without the quote characters. */
void trim_quotes(char *str) {
    int len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len-1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

/* is_builtin: detect shell built-ins */
int is_builtin(char **args) {
    if (args[0] == NULL) {
        return 0;
    }

    /* Currently support `cd` and `exit` as built-ins */
    if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0) {
        return 1;
    }

    return 0;
}

/* handle_builtin: implement `cd` and `exit` behavior
 * - `exit`: free allocated argument memory and terminate the shell
 * - `cd [path]`: change directory; if no argument, change to $HOME */
void handle_builtin(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        /* Free any allocated argument strings (prevents leaks in short-lived shell) */
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        char *path;

        if (args[1] == NULL) {
            /* No argument: go to HOME */
            path = getenv("HOME");
            if (path == NULL) {
                fprintf(stderr, "cd: HOME not set\n");
                return;
            }
        } else {
            path = args[1];
        }

        if (chdir(path) != 0) {
            perror("cd");
        }
    }
}

/* execute_command
 * Fork a child process, set up any requested redirection, and exec the
 * requested program. The parent waits for the child to finish.
 * After execution, free any allocated memory for args and filenames.
 */
void execute_command(char **args, char *input_file, char *output_file, int append_mode) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        /* Child process: perform redirections, then exec */

        /* Input redirection: open file and dup to STDIN */
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror(input_file);
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        /* Output redirection: open file with correct flags and dup to STDOUT */
        if (output_file != NULL) {
            int fd_out;
            if (append_mode) {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }

            if (fd_out < 0) {
                perror(output_file);
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        /* Replace the child process image with the requested program */
        execvp(args[0], args);

        /* If execvp returns, an error occurred (command not found or exec failed) */
        fprintf(stderr, "%s: command not found\n", args[0]);
        exit(1);
    } else {
        /* Parent process: wait for the child to finish */
        int status;
        waitpid(pid, &status, 0);
    }

    /* Free allocated memory for arguments and filenames */
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    if (input_file != NULL) {
        free(input_file);
    }
    if (output_file != NULL) {
        free(output_file);
    }
}
