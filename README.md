# Shell — COP4610 Project 1

A shell interface supporting prompt display, environment variable and tilde
expansion, `$PATH` search, external command execution, I/O redirection,
piping (unlimited pipe stages), background processing, and internal
(built-in) commands.

## Group Members

- **Ammiel Bowen**
- **Don Damier**
- **Widens Filsaime**

Group Number: 37

## Division of Labor (before)

| Part | Assigned to |
|---|---|
| Part 1: Prompt | Ammiel Bowen (lead), Widens Filsaime (support) |
| Part 2: Environment Variables | Widens Filsaime (lead), Don Damier (support) |
| Part 3: Tilde Expansion | Don Damier (lead), Ammiel Bowen (support) |
| Part 4: $PATH Search | Ammiel Bowen (lead), Don Damier (support) |
| Part 5: External Command Execution | Widens Filsaime (lead), Ammiel Bowen (support) |
| Part 6: I/O Redirection | Ammiel Bowen (lead), Widens Filsaime (support) |
| Part 7: Piping | Don Damier (lead), Widens Filsaime (support) |
| Part 8: Background Processing | Widens Filsaime (lead), Don Damier (support) |
| Part 9: Internal Command Execution | Don Damier (lead), Ammiel Bowen (support) |
| Extra Credit | All members |

## Division of Labor (after)

Work was completed as planned above:

| Part | Completed by |
|---|---|
| Part 1: Prompt | Ammiel Bowen, Widens Filsaime |
| Part 2: Environment Variables | Widens Filsaime, Don Damier |
| Part 3: Tilde Expansion | Don Damier, Ammiel Bowen |
| Part 4: $PATH Search | Ammiel Bowen, Don Damier |
| Part 5: External Command Execution | Widens Filsaime, Ammiel Bowen |
| Part 6: I/O Redirection | Ammiel Bowen, Widens Filsaime |
| Part 7: Piping | Don Damier, Widens Filsaime |
| Part 8: Background Processing | Widens Filsaime, Don Damier |
| Part 9: Internal Command Execution | Don Damier, Ammiel Bowen |
| Extra Credit (unlimited pipes, piping+redirection, shell-ception) | All members |

*[If this isn't actually how it shook out — e.g. someone ended up covering
a different part, or the group worked through code together rather than
split cleanly by part — edit this table before submitting so it matches
what really happened.]*

## File Listing

```
shell/
│
├── src/
│   ├── main.c        # REPL: prompt, read, expand, parse, dispatch
│   ├── lexer.c        # Part 0: tokenizer (words + <, >, |, & operators)
│   ├── expand.c        # Part 2/3: $VAR and ~ expansion
│   ├── parser.c        # Builds pipeline/redirection/background structure from tokens
│   ├── exec.c        # Part 4/5/6/7: $PATH search, fork/execv, redirection, piping
│   ├── builtins.c        # Part 9: exit, cd, jobs (implemented without execv)
│   ├── jobs.c        # Part 8: background job table, non-blocking reaping
│   ├── history.c        # Part 9 (exit): last 3 valid commands
│   └── prompt.c        # Part 1: USER@MACHINE:PWD> prompt
│
├── include/
│   └── shell.h        # Shared types and function declarations
│
├── bin/
│   └── shell        # Build output (not committed; produced by `make`)
│
├── README.md
└── Makefile
```

## How to Compile & Execute

### Requirements
- `gcc` (C99), GNU `make`, a POSIX/Linux environment (tested on `linprog`).

### Compilation
```bash
make
```
This builds `bin/shell`.

### Execution
```bash
make run
```
or directly:
```bash
./bin/shell
```

## Development Log

*[Each member: fill in what you worked on and when.]*

### Ammiel Bowen

| Date | Work Completed / Notes |
|---|---|
| | |

### Don Damier

| Date | Work Completed / Notes |
|---|---|
| | |

### Widens Filsaime

| Date | Work Completed / Notes |
|---|---|
| | |

## Meetings

*[Document your actual in-person/virtual meetings here.]*

| Date | Attendees | Topics Discussed | Outcomes / Decisions |
|---|---|---|---|
| | | | |

## Design Notes

- **Tokenizer** (`lexer.c`) splits on whitespace and always emits `<`, `>`,
  `|`, `&` as their own tokens (even with no surrounding whitespace), so the
  same parser logic handles `cmd > file` regardless of spacing.
- **Parsing** (`parser.c`) builds a `command_line` holding an array of
  `simple_cmd` pipeline stages. Each stage independently tracks its own
  `infile`/`outfile`, and the whole line has one `background` flag. `&` is
  only valid as the last token; empty pipeline segments (e.g. `ls | | wc`,
  a leading `|`, or a trailing `|`) are rejected as syntax errors.
- **Execution** (`exec.c`) only uses `fork()`/`execv()` as required. `$PATH`
  search (`path_search`) checks each `:`-separated directory with
  `access(..., X_OK)`. Output-redirection files are created with mode
  `0600` (`-rw-------`) and truncated if they already exist; input-redirect
  files are validated to exist and be regular files before opening
  read-only.
- **Background jobs** (`jobs.c`) are tracked in a fixed-size table (job
  numbers increment and are never reused) and reaped with
  `waitpid(..., WNOHANG)`, polled once per REPL iteration — no signal
  handling is used, per the assignment's note that this is not required.
- **Built-ins** (`builtins.c`) — `exit`, `cd`, `jobs` — run directly in the
  shell process and never call `execv()`. `exit` waits for any remaining
  background jobs before printing the last-valid-commands summary and
  terminating.

## Bugs

*[None known. List any discovered during testing.]*

## Extra Credit

- **Unlimited pipes** — the parser and executor build a dynamically-sized
  array of pipeline stages rather than assuming a fixed max of two, so any
  number of `|` stages is supported (e.g. `ls | sort | cat | cat`).
- **Piping and I/O redirection together** — each pipeline stage carries its
  own optional `infile`/`outfile` independently of its pipe connections, so
  e.g. `cat < in.txt | sort | uniq > out.txt` works: the first stage reads
  from the file (not a pipe), the last stage writes to the file (not a
  pipe), and the middle stage(s) connect via pipes as usual.
- **Shell-ception** — since the shell is a normal executable found via
  `$PATH`/relative-path search like any other command, running `./shell` (or
  `shell` if `bin/` is on `$PATH`) from inside a running instance works
  without any special-casing, and can be nested arbitrarily deep.

*(Documented per the assignment's requirement that extra credit be noted
here to receive credit.)*

## Considerations

- Built-ins (`exit`, `cd`, `jobs`) do not participate in piping or
  redirection — only external commands do, consistent with how the project
  restrictions describe built-ins as implemented "from scratch" rather than
  through the same `fork`/`execv` pipeline machinery.
- A command whose executable can't be found via `$PATH` search is reported
  with `command not found` and simply not launched; if it was one stage of
  a larger pipeline, the other stages still run (they'll just see closed
  pipe ends).
