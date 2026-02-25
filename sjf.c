/*
 * sjf.c - Preemptive Shortest Job First (Shortest Remaining Time First) Scheduler
 *
 * Compile: gcc -o sjf sjf.c
 * Usage:   ./sjf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_PROCESSES 10

/* ---------- Process structure ---------- */
typedef struct {
    int pid;           /* Process ID                      */
    int arrival_time;  /* Time process arrives in queue   */
    int burst_time;    /* Original CPU burst time         */
    int remaining;     /* Remaining burst time            */
    int waiting_time;  /* Total waiting time              */
    int turnaround;    /* Turnaround time                 */
    int finish_time;   /* Time process completes          */
    int started;       /* Flag: has process started?      */
} Process;

/* ---------- Utility: print a separator ---------- */
static void print_separator(void) {
    printf("+-----+---------------+-----------+---------------+-----------------+\n");
}

/* ---------- Comparison for sorting by arrival time ---------- */
static int cmp_arrival(const void *a, const void *b) {
    return ((Process *)a)->arrival_time - ((Process *)b)->arrival_time;
}

/* ================================================================
 * Preemptive SJF (SRTF)
 *
 * At every clock tick, pick the ready process with the smallest
 * remaining burst time.  If a tie exists, break by pid.
 * ================================================================ */
void sjf_schedule(Process proc[], int n) {
    int time = 0;
    int completed = 0;
    int exec_order[1000];   /* record which pid runs each tick */
    int exec_len = 0;

    /* Reset dynamic fields */
    for (int i = 0; i < n; i++) {
        proc[i].remaining   = proc[i].burst_time;
        proc[i].waiting_time = 0;
        proc[i].turnaround  = 0;
        proc[i].finish_time = 0;
        proc[i].started     = 0;
    }

    while (completed < n) {
        /* Find process with shortest remaining time that has arrived */
        int sel = -1;
        int min_rem = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= time && proc[i].remaining > 0) {
                if (proc[i].remaining < min_rem ||
                    (proc[i].remaining == min_rem && proc[i].pid < proc[sel].pid)) {
                    min_rem = proc[i].remaining;
                    sel = i;
                }
            }
        }

        if (sel == -1) {
            /* CPU idle – advance to next arrival */
            time++;
            continue;
        }

        /* Run selected process for 1 time unit */
        if (exec_len == 0 || exec_order[exec_len - 1] != proc[sel].pid) {
            exec_order[exec_len++] = proc[sel].pid;
        }

        proc[sel].remaining--;
        time++;

        if (proc[sel].remaining == 0) {
            proc[sel].finish_time   = time;
            proc[sel].turnaround    = time - proc[sel].arrival_time;
            proc[sel].waiting_time  = proc[sel].turnaround - proc[sel].burst_time;
            completed++;
        }
    }

    /* ---------- Output ---------- */
    printf("\n=========================================================\n");
    printf("         Preemptive SJF (Shortest Remaining Time First)\n");
    printf("=========================================================\n\n");

    /* Execution order */
    printf("Execution Order: ");
    for (int i = 0; i < exec_len; i++) {
        printf("P%d", exec_order[i]);
        if (i < exec_len - 1) printf(" -> ");
    }
    printf("\n\n");

    /* Per-process table */
    print_separator();
    printf("| PID | Arrival Time  | Burst Time| Waiting Time  | Turnaround Time |\n");
    print_separator();

    double total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        printf("|  P%-2d |    %-10d |    %-6d |    %-10d |    %-12d |\n",
        proc[i].pid,
        proc[i].arrival_time,
        proc[i].burst_time,
        proc[i].waiting_time,
        proc[i].turnaround);
        total_wt  += proc[i].waiting_time;
        total_tat += proc[i].turnaround;
    }
    print_separator();

    printf("\nAverage Waiting Time    : %.2f\n", total_wt  / n);
    printf("Average Turnaround Time : %.2f\n",  total_tat / n);
    printf("\n");
}

/* ================================================================
 * main – hard-coded sample processes (edit as needed)
 * ================================================================ */
int main(void) {
    Process procs[] = {
        /* pid  arrival  burst */
        {1, 0, 8},
        {2, 1, 4},
        {3, 2, 9},
        {4, 3, 5},
    };
    int n = sizeof(procs) / sizeof(procs[0]);

    /* Sort by arrival time before scheduling */
    qsort(procs, n, sizeof(Process), cmp_arrival);

    sjf_schedule(procs, n);
    return 0;
}
