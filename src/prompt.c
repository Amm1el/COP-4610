/*
 * prompt.c
 *
 * Part 1: the shell prompt, "USER@MACHINE:PWD>", built from the $USER,
 * $MACHINE and $PWD environment variables (with sane fallbacks if any of
 * them happen to be unset).
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

void sync_pwd_env(void) {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) {
        setenv("PWD", buf, 1);
    }
}

void print_prompt(void) {
    char *user = getenv("USER");
    if (!user) user = getlogin();
    if (!user) user = "user";

    char hostbuf[256];
    char *machine = getenv("MACHINE");
    if (!machine) {
        if (gethostname(hostbuf, sizeof(hostbuf)) == 0) {
            machine = hostbuf;
        } else {
            machine = "machine";
        }
    }

    char cwdbuf[PATH_MAX];
    char *pwd = getenv("PWD");
    if (!pwd) {
        if (getcwd(cwdbuf, sizeof(cwdbuf))) {
            pwd = cwdbuf;
        } else {
            pwd = ".";
        }
    }

    printf("%s@%s:%s>", user, machine, pwd);
    fflush(stdout);
}
