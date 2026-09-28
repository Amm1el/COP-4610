/*
 * lexer.c
 *
 * Reads a raw line of input and splits it into tokens. Tokens are either
 * "words" (arguments/commands/filenames) or single-character operators:
 * '<', '>', '|', '&'. Operators are always emitted as their own token,
 * whether or not they are surrounded by whitespace, so both "cmd > file"
 * and "cmd>file" tokenize the same way.
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *get_input(void) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t len = getline(&line, &cap, stdin);

    if (len < 0) {
        free(line);
        return NULL; /* EOF or read error */
    }

    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }

    return line;
}

tokenlist *new_tokenlist(void) {
    tokenlist *tokens = malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = malloc(sizeof(char *));
    tokens->items[0] = NULL;
    return tokens;
}

void add_token(tokenlist *tokens, const char *item) {
    size_t i = tokens->size;
    tokens->items = realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = malloc(strlen(item) + 1);
    strcpy(tokens->items[i], item);
    tokens->items[i + 1] = NULL;
    tokens->size += 1;
}

static int is_operator_char(char c) {
    return c == '<' || c == '>' || c == '|' || c == '&';
}

tokenlist *get_tokens(const char *input) {
    tokenlist *tokens = new_tokenlist();
    size_t i = 0;
    size_t n = strlen(input);

    while (i < n) {
        while (i < n && isspace((unsigned char)input[i])) {
            i++;
        }
        if (i >= n) {
            break;
        }

        if (is_operator_char(input[i])) {
            char op[2] = { input[i], '\0' };
            add_token(tokens, op);
            i++;
            continue;
        }

        size_t start = i;
        while (i < n && !isspace((unsigned char)input[i]) && !is_operator_char(input[i])) {
            i++;
        }
        size_t len = i - start;
        char *word = malloc(len + 1);
        memcpy(word, input + start, len);
        word[len] = '\0';
        add_token(tokens, word);
        free(word);
    }

    return tokens;
}

void free_tokens(tokenlist *tokens) {
    if (!tokens) return;
    for (size_t i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }
    free(tokens->items);
    free(tokens);
}
