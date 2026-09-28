/*
 * parser.c
 *
 * Turns an expanded tokenlist into a command_line: a sequence of one or
 * more simple_cmd pipeline stages (split on '|'), each of which may carry
 * an input file (from '<'), an output file (from '>'), plus an overall
 * background flag (trailing '&').
 *
 * Supports an unlimited number of pipe stages, and redirection combined
 * with piping on any individual stage.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static command_line *new_command_line(void) {
    command_line *cl = malloc(sizeof(command_line));
    cl->cmds = NULL;
    cl->n_cmds = 0;
    cl->background = false;
    cl->parse_error = false;
    cl->error_msg[0] = '\0';
    return cl;
}

static simple_cmd *add_cmd_slot(command_line *cl) {
    cl->cmds = realloc(cl->cmds, sizeof(simple_cmd) * (cl->n_cmds + 1));
    simple_cmd *sc = &cl->cmds[cl->n_cmds];
    sc->argv = malloc(sizeof(char *));
    sc->argv[0] = NULL;
    sc->argc = 0;
    sc->infile = NULL;
    sc->outfile = NULL;
    cl->n_cmds++;
    return sc;
}

static void cmd_add_arg(simple_cmd *sc, const char *arg) {
    sc->argv = realloc(sc->argv, sizeof(char *) * (sc->argc + 2));
    sc->argv[sc->argc] = strdup(arg);
    sc->argv[sc->argc + 1] = NULL;
    sc->argc++;
}

static bool is_op(const char *tok) {
    return strcmp(tok, "|") == 0 || strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0;
}

command_line *parse_tokens(tokenlist *tokens) {
    command_line *cl = new_command_line();
    int n = (int)tokens->size;
    int end = n;

    /* '&' must be the very last token; anywhere else is a syntax error. */
    for (int i = 0; i < n; i++) {
        if (strcmp(tokens->items[i], "&") == 0) {
            if (i != n - 1) {
                cl->parse_error = true;
                snprintf(cl->error_msg, sizeof(cl->error_msg),
                         "syntax error near unexpected token '&'");
                return cl;
            }
            cl->background = true;
            end = i;
        }
    }

    if (end == 0) {
        cl->parse_error = true;
        snprintf(cl->error_msg, sizeof(cl->error_msg), "no command specified");
        return cl;
    }

    simple_cmd *cur = add_cmd_slot(cl);
    bool has_word_in_segment = false;

    for (int i = 0; i < end; i++) {
        char *tok = tokens->items[i];

        if (strcmp(tok, "|") == 0) {
            if (!has_word_in_segment) {
                cl->parse_error = true;
                snprintf(cl->error_msg, sizeof(cl->error_msg),
                         "syntax error near unexpected token '|'");
                return cl;
            }
            cur = add_cmd_slot(cl);
            has_word_in_segment = false;
            continue;
        }

        if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0) {
            bool is_input = (tok[0] == '<');
            if (i + 1 >= end || is_op(tokens->items[i + 1])) {
                cl->parse_error = true;
                snprintf(cl->error_msg, sizeof(cl->error_msg),
                         "syntax error: expected filename after '%s'", tok);
                return cl;
            }
            i++;
            if (is_input) {
                free(cur->infile);
                cur->infile = strdup(tokens->items[i]);
            } else {
                free(cur->outfile);
                cur->outfile = strdup(tokens->items[i]);
            }
            continue;
        }

        cmd_add_arg(cur, tok);
        has_word_in_segment = true;
    }

    if (!has_word_in_segment) {
        cl->parse_error = true;
        snprintf(cl->error_msg, sizeof(cl->error_msg),
                 "syntax error near unexpected token '|'");
        return cl;
    }

    return cl;
}

void free_command_line(command_line *cl) {
    if (!cl) return;
    for (int i = 0; i < cl->n_cmds; i++) {
        simple_cmd *sc = &cl->cmds[i];
        for (int j = 0; j < sc->argc; j++) {
            free(sc->argv[j]);
        }
        free(sc->argv);
        free(sc->infile);
        free(sc->outfile);
    }
    free(cl->cmds);
    free(cl);
}
