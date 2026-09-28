/*
 * expand.c
 *
 * Part 2 (Environment Variables) and Part 3 (Tilde Expansion).
 *
 * A token that is a whole "$NAME" argument is replaced by the value of the
 * environment variable NAME (or the empty string if it is unset). A token
 * that is exactly "~" or that begins with "~/" has that leading "~" replaced
 * with the value of $HOME.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void expand_tokens(tokenlist *tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        char *tok = tokens->items[i];

        if (tok[0] == '$' && tok[1] != '\0') {
            char *val = getenv(tok + 1);
            char *newval = strdup(val ? val : "");
            free(tokens->items[i]);
            tokens->items[i] = newval;
            continue;
        }

        if (strcmp(tok, "~") == 0 || (tok[0] == '~' && tok[1] == '/')) {
            char *home = getenv("HOME");
            if (!home) home = "";
            const char *rest = (tok[0] == '~' && tok[1] == '/') ? tok + 1 : "";
            size_t len = strlen(home) + strlen(rest) + 1;
            char *newval = malloc(len);
            snprintf(newval, len, "%s%s", home, rest);
            free(tokens->items[i]);
            tokens->items[i] = newval;
        }
    }
}
