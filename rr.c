/*
 * rr.c - Round Robin CPU Scheduling Simulator
 *
 * Compile: gcc -o rr rr.c
 * Usage:   ./rr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10
#define TIME_QUANTUM  3    /* <-- adjust quantum here */

/* ---------- Process structure ---------- */
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining;
    int waiting_time;
    int turnaround;
    int finish_time;
} Process;

/* ---------- Simple circular queue ---------- */
typedef struct {
    int data[MAX_PROCESSES * 200];
    int front;
    int rear;
    int size;
} Queue;

static void queue_init(Queue *q) { q->front = 0; q->rear = 0; q->size = 0; }
static int  queue_empty(Queue *q) { return q->size == 0; }
static void enqueue(Queue *q, int val) {
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % (MAX_PROCESSES * 200);
    q->size++;
}
static int dequeue(Queue *q) {
    int val = q->data[q->front];
    q->front = (q->front + 1) % (MAX_PROCESSES * 200);
    q->size--;
    return val;
}

/* ---------- Utility: print a separator ---------- */
static void print_separator(void) {
    printf("+-----+---------------+-----------+---------------+-----------------+\n");
}

/* ================================================================
 * Round Robin scheduler
 * ================================================================ */
void rr_schedule(Process proc[], int n, int quantum) {
    /* Reset dynamic fields */
    for (int i = 0; i < n; i++) {
        proc[i].remaining   = proc[i].burst_time;
        proc[i].waiting_time = 0;
        proc[i].turnaround  = 0;
        proc[i].finish_time = 0;
    }

    Queue ready;
    queue_init(&ready);

    int time      = 0;
    int completed = 0;
    int in_rq[MAX_PROCESSES];   /* flag: already added to queue? */
    memset(in_rq, 0, sizeof(in_rq));

    /* Enqueue all processes that arrive at time 0 */
    for (int i = 0; i < n; i++) {
        if (proc[i].arrival_time == 0) {
            enqueue(&ready, i);
            in_rq[i] = 1;
        }
    }

    /* If nothing arrives at 0, advance time to first arrival */
    if (queue_empty(&ready)) {
        int min_arr = proc[0].arrival_time;
        for (int i = 1; i < n; i++)
            if (proc[i].arrival_time < min_arr) min_arr = proc[i].arrival_time;
        time = min_arr;
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time == time) {
                enqueue(&ready, i);
                in_rq[i] = 1;
            }
        }
    }

    while (completed < n) {
        if (queue_empty(&ready)) {
            /* CPU idle – advance to next arriving process */
            time++;
            for (int i = 0; i < n; i++) {
                if (!in_rq[i] && proc[i].arrival_time <= time && proc[i].remaining > 0) {
                    enqueue(&ready, i);
                    in_rq[i] = 1;
                }
            }
            continue;
        }

        int idx = dequeue(&ready);

        /* Run for up to quantum time units */
        int run = (proc[idx].remaining < quantum) ? proc[idx].remaining : quantum;
        proc[idx].remaining -= run;
        time += run;

        /* Enqueue any process that arrived during this slice */
        for (int i = 0; i < n; i++) {
            if (!in_rq[i] && proc[i].arrival_time <= time && proc[i].remaining > 0) {
                enqueue(&ready, i);
                in_rq[i] = 1;
            }
        }

        if (proc[idx].remaining == 0) {
            /* Process finished */
            proc[idx].finish_time  = time;
            proc[idx].turnaround   = time - proc[idx].arrival_time;
            proc[idx].waiting_time = proc[idx].turnaround - proc[idx].burst_time;
            completed++;
        } else {
            /* Re-enqueue (preempted by quantum expiry) */
            enqueue(&ready, idx);
        }
    }

    /* ---------- Output ---------- */
    printf("\n=========================================================\n");
    printf("                    Round Robin (Quantum = %d)\n", quantum);
    printf("=========================================================\n\n");

    printf("Execution Order: ");
    for (int i = 0; i < n; i++) {
        printf("P%d", proc[i].pid);
        if (i < n - 1) printf(" -> ");
    }
    printf("\n\n");

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
 * main – hard-coded sample processes 
 * ================================================================ */
int main(void) {
Process procs[] = {
    {1, 0, 24, 0, 0, 0, 0},
    {2, 0,  3, 0, 0, 0, 0},
    {3, 0,  3, 0, 0, 0, 0},
};
    int n = sizeof(procs) / sizeof(procs[0]);

    rr_schedule(procs, n, TIME_QUANTUM);
    return 0;
}
