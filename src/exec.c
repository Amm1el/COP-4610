/*
 * exec.c
 *
 * Part 4 ($PATH search), Part 5 (external command execution),
 * Part 6 (I/O redirection) and Part 7 (piping), plus the fork/exec side
 * of Part 8 (background processing).
 *
 * Only fork() and execv() are used to create and run external commands,
 * per the project restrictions (no execvp/execlp/system/etc).
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* Part 4: $PATH search. If cmd already contains a '/', treat it as a path
 * (relative or absolute) and just check it's executable. Otherwise search
 * each ':'-separated directory in $PATH for an executable of that name. */
char *path_search(const char *cmd) {
    if (strchr(cmd, '/') != NULL) {
        if (access(cmd, X_OK) == 0) {
            return strdup(cmd);
        }
        return NULL;
    }

    char *path_env = getenv("PATH");
    if (!path_env) {
        return NULL;
    }

    char *path_copy = strdup(path_env);
    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    char candidate[4096];

    while (dir != NULL) {
        snprintf(candidate, sizeof(candidate), "%s/%s", dir, cmd);
        if (access(candidate, X_OK) == 0) {
            free(path_copy);
            return strdup(candidate);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
    return NULL;
}

/* Part 6: open an input-redirection file. Must exist and be a regular file;
 * opened read-only so the shell never modifies it. */
static int open_infile(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        fprintf(stderr, "shell: %s: No such file or directory\n", path);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "shell: %s: Not a regular file\n", path);
        return -1;
    }
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "shell: %s: %s\n", path, strerror(errno));
        return -1;
    }
    return fd;
}

/* Part 6: open (creating/truncating) an output-redirection file with mode
 * -rw------- (0600), as required by the spec. */
static int open_outfile(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "shell: %s: %s\n", path, strerror(errno));
        return -1;
    }
    return fd;
}

/* Runs one or more piped external commands (Part 5/6/7), and if
 * cl->background is set, hands the resulting job off to the job table
 * instead of waiting on it (Part 8). */
void execute_command_line(command_line *cl, const char *raw_cmdline) {
    int n = cl->n_cmds;
    int (*pipes)[2] = NULL;

    if (n > 1) {
        pipes = malloc(sizeof(int[2]) * (n - 1));
        for (int i = 0; i < n - 1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("shell: pipe");
                free(pipes);
                return;
            }
        }
    }

    pid_t *pids = malloc(sizeof(pid_t) * n);
    int launched = 0;

    for (int i = 0; i < n; i++) {
        simple_cmd *sc = &cl->cmds[i];
        char *exe = path_search(sc->argv[0]);
        if (!exe) {
            fprintf(stderr, "shell: %s: command not found\n", sc->argv[0]);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("shell: fork");
            free(exe);
            continue;
        }

        if (pid == 0) {
            /* ---- child ---- */
            if (sc->infile) {
                int fd = open_infile(sc->infile);
                if (fd < 0) _exit(1);
                dup2(fd, STDIN_FILENO);
                close(fd);
            } else if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (sc->outfile) {
                int fd = open_outfile(sc->outfile);
                if (fd < 0) _exit(1);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } else if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int k = 0; k < n - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            execv(exe, sc->argv);
            fprintf(stderr, "shell: %s: %s\n", sc->argv[0], strerror(errno));
            _exit(127);
        }

        /* ---- parent ---- */
        pids[launched++] = pid;
        free(exe);
    }

    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    free(pipes);

    if (launched == 0) {
        free(pids);
        return;
    }

    if (cl->background) {
        int job_num = jobs_add(pids, launched, raw_cmdline);
        pid_t report_pid = pids[launched - 1];
        printf("[%d] %d\n", job_num, report_pid);
    } else {
        for (int i = 0; i < launched; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }

    free(pids);
}
