/*
 * jobs.c
 *
 * Part 8: background job tracking. Jobs are not reaped via signals (the
 * spec explicitly allows polling); jobs_check() is called once per REPL
 * iteration and uses waitpid(..., WNOHANG) to notice completed jobs.
 */

#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

typedef struct {
    int job_num;
    pid_t pids[MAX_JOB_PIDS];
    bool reaped[MAX_JOB_PIDS];
    int n_pids;
    bool running;
    char cmdline[MAX_CMDLINE];
} job_slot;

static job_slot jobs[MAX_BG_JOBS];
static int next_job_num = 1;

void jobs_init(void) {
    memset(jobs, 0, sizeof(jobs));
}

int jobs_add(pid_t *pids, int n_pids, const char *cmdline) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (jobs[i].running) continue;

        jobs[i].job_num = next_job_num++;
        jobs[i].n_pids = (n_pids > MAX_JOB_PIDS) ? MAX_JOB_PIDS : n_pids;
        for (int j = 0; j < jobs[i].n_pids; j++) {
            jobs[i].pids[j] = pids[j];
            jobs[i].reaped[j] = false;
        }
        strncpy(jobs[i].cmdline, cmdline, MAX_CMDLINE - 1);
        jobs[i].cmdline[MAX_CMDLINE - 1] = '\0';
        jobs[i].running = true;
        return jobs[i].job_num;
    }
    return -1;
}

void jobs_check(void) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (!jobs[i].running) continue;

        bool all_done = true;
        for (int j = 0; j < jobs[i].n_pids; j++) {
            if (jobs[i].reaped[j]) continue;
            int status;
            pid_t r = waitpid(jobs[i].pids[j], &status, WNOHANG);
            if (r == jobs[i].pids[j]) {
                jobs[i].reaped[j] = true;
            } else {
                all_done = false;
            }
        }

        if (all_done) {
            pid_t report_pid = jobs[i].pids[jobs[i].n_pids - 1];
            printf("[%d]  + %d done %s\n\n", jobs[i].job_num, report_pid, jobs[i].cmdline);
            jobs[i].running = false;
        }
    }
}

void jobs_print(void) {
    bool any = false;
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (!jobs[i].running) continue;
        pid_t report_pid = jobs[i].pids[jobs[i].n_pids - 1];
        printf("[%d]  + %d running %s\n", jobs[i].job_num, report_pid, jobs[i].cmdline);
        any = true;
    }
    if (!any) {
        printf("jobs: no active background processes\n");
    }
}

void jobs_wait_all(void) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (!jobs[i].running) continue;
        for (int j = 0; j < jobs[i].n_pids; j++) {
            if (jobs[i].reaped[j]) continue;
            int status;
            waitpid(jobs[i].pids[j], &status, 0);
            jobs[i].reaped[j] = true;
        }
        jobs[i].running = false;
    }
}

int jobs_active_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (jobs[i].running) count++;
    }
    return count;
}
