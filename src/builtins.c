/*
 * builtins.c
 *
 * Part 9: internal command execution. exit, cd and jobs are implemented
 * from scratch here (no execv is used for builtins, per the project
 * restrictions) and run directly in the shell's own process.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

bool is_builtin(const char *cmd) {
    return cmd && (strcmp(cmd, "exit") == 0 ||
                   strcmp(cmd, "cd") == 0 ||
                   strcmp(cmd, "jobs") == 0);
}

static void builtin_cd(simple_cmd *cmd) {
    int nargs = cmd->argc - 1; /* arguments after "cd" */

    if (nargs > 1) {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }

    const char *target;
    if (nargs == 0) {
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    } else {
        target = cmd->argv[1];
    }

    struct stat st;
    if (stat(target, &st) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", target);
        return;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "cd: %s: Not a directory\n", target);
        return;
    }
    if (chdir(target) != 0) {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        return;
    }

    sync_pwd_env();
}

static void builtin_exit(void) {
    if (jobs_active_count() > 0) {
        jobs_wait_all();
    }
    history_print_on_exit();
    exit(0);
}

void run_builtin(simple_cmd *cmd, command_line *cl) {
    (void)cl;

    if (strcmp(cmd->argv[0], "cd") == 0) {
        builtin_cd(cmd);
    } else if (strcmp(cmd->argv[0], "jobs") == 0) {
        jobs_print();
    } else if (strcmp(cmd->argv[0], "exit") == 0) {
        builtin_exit();
    }
}
