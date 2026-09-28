/*
 * history.c
 *
 * Part 9 (exit): tracks the last three "valid" (successfully parsed,
 * non-empty) command lines entered, printed when the shell exits.
 * The "exit" command itself is never added to this history.
 */

#include "shell.h"
#include <stdio.h>
#include <string.h>

static char history[MAX_HISTORY][MAX_CMDLINE];
static int history_count = 0; /* number of slots currently filled, <= MAX_HISTORY */
static int total_valid = 0;   /* total valid commands ever entered */

void history_add(const char *cmdline) {
    if (history_count == MAX_HISTORY) {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strncpy(history[MAX_HISTORY - 1], cmdline, MAX_CMDLINE - 1);
        history[MAX_HISTORY - 1][MAX_CMDLINE - 1] = '\0';
    } else {
        strncpy(history[history_count], cmdline, MAX_CMDLINE - 1);
        history[history_count][MAX_CMDLINE - 1] = '\0';
        history_count++;
    }
    total_valid++;
}

void history_print_on_exit(void) {
    if (total_valid == 0) {
        printf("No valid commands were entered.\n");
        return;
    }

    if (total_valid < MAX_HISTORY) {
        printf("Last valid command: %s\n", history[history_count - 1]);
        return;
    }

    printf("Last (%d) valid commands:\n", MAX_HISTORY);
    for (int i = 0; i < MAX_HISTORY; i++) {
        printf("[%d]: %s\n", i + 1, history[i]);
    }
}
