#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

#define MAX_BG_JOBS   16
#define MAX_JOB_PIDS  8
#define MAX_HISTORY   3
#define MAX_CMDLINE   256

/* ---------------- lexer ---------------- */

typedef struct {
    char **items;
    size_t size;
} tokenlist;

char *get_input(void);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, const char *item);
tokenlist *get_tokens(const char *input);
void free_tokens(tokenlist *tokens);

/* ---------------- expansion ---------------- */

void expand_tokens(tokenlist *tokens);

/* ---------------- parser ---------------- */

typedef struct {
    char **argv;   /* NULL-terminated argument vector */
    int argc;
    char *infile;  /* NULL if no input redirection */
    char *outfile; /* NULL if no output redirection */
} simple_cmd;

typedef struct {
    simple_cmd *cmds;
    int n_cmds;
    bool background;
    bool parse_error;
    char error_msg[256];
} command_line;

command_line *parse_tokens(tokenlist *tokens);
void free_command_line(command_line *cl);

/* ---------------- path search ---------------- */

char *path_search(const char *cmd);

/* ---------------- execution ---------------- */

void execute_command_line(command_line *cl, const char *raw_cmdline);

/* ---------------- builtins ---------------- */

bool is_builtin(const char *cmd);
void run_builtin(simple_cmd *cmd, command_line *cl);

/* ---------------- jobs ---------------- */

void jobs_init(void);
int jobs_add(pid_t *pids, int n_pids, const char *cmdline);
void jobs_check(void);
void jobs_print(void);
void jobs_wait_all(void);
int jobs_active_count(void);

/* ---------------- history ---------------- */

void history_add(const char *cmdline);
void history_print_on_exit(void);

/* ---------------- prompt / env ---------------- */

void print_prompt(void);
void sync_pwd_env(void);
