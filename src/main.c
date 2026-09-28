/*
 * main.c
 *
 * The shell's REPL: print prompt -> read line -> check on background jobs
 * -> tokenize -> expand $VARs and ~ -> parse into a command_line -> run it
 * (builtin or external, possibly piped/redirected/backgrounded).
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    jobs_init();

    if (!getenv("PWD")) {
        sync_pwd_env();
    }

    while (1) {
        print_prompt();

        char *line = get_input();
        if (line == NULL) {
            /* EOF (e.g. Ctrl+D): behave like "exit" */
            printf("\n");
            if (jobs_active_count() > 0) {
                jobs_wait_all();
            }
            history_print_on_exit();
            break;
        }

        jobs_check();

        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        size_t tlen = strlen(trimmed);
        while (tlen > 0 && (trimmed[tlen - 1] == ' ' || trimmed[tlen - 1] == '\t')) {
            trimmed[--tlen] = '\0';
        }

        if (tlen == 0) {
            free(line);
            continue;
        }

        tokenlist *tokens = get_tokens(trimmed);
        if (tokens->size == 0) {
            free_tokens(tokens);
            free(line);
            continue;
        }

        expand_tokens(tokens);
        command_line *cl = parse_tokens(tokens);

        if (cl->parse_error) {
            fprintf(stderr, "shell: %s\n", cl->error_msg);
        } else {
            /* Build the label used for history/jobs display: the raw
             * command line as typed, minus a trailing '&'. */
            char histbuf[MAX_CMDLINE];
            strncpy(histbuf, trimmed, MAX_CMDLINE - 1);
            histbuf[MAX_CMDLINE - 1] = '\0';
            size_t hl = strlen(histbuf);
            if (hl > 0 && histbuf[hl - 1] == '&') {
                histbuf[--hl] = '\0';
                while (hl > 0 && (histbuf[hl - 1] == ' ' || histbuf[hl - 1] == '\t')) {
                    histbuf[--hl] = '\0';
                }
            }

            bool is_exit_cmd = (cl->n_cmds == 1 && strcmp(cl->cmds[0].argv[0], "exit") == 0);
            if (!is_exit_cmd) {
                history_add(histbuf);
            }

            if (cl->n_cmds == 1 && is_builtin(cl->cmds[0].argv[0])) {
                run_builtin(&cl->cmds[0], cl);
            } else {
                execute_command_line(cl, histbuf);
            }
        }

        free_command_line(cl);
        free_tokens(tokens);
        free(line);
    }

    return 0;
}
