/*
    An Vu
    CSCI4500 Operating Systems
    October 10, 2025
*/

/*
    Program 2 Scheduling
*/

#include <stdio.h>
#include <stdlib.h>

/* =================== CONSTANTS =================== */
#define MAXCPU 4
#define MAXPROC 25

/* Process states */
#define NOTYETSUBMITTED 1
#define READY 2
#define RUNNING 3
#define BLOCKED 4
#define COMPLETED 5

/* =================== STRUCTURES =================== */
typedef struct {
    int idle;       // total CPU idle time
    int run_time;   // time spent running current process (this quantum)
    int proc_ndx;   // -1 if CPU idle, else process index
} CPU;

typedef struct {
    /* Input data */
    int pid;
    int prio;
    int t_submit;
    int tot_cpu_req;
    int cpu_cycle;
    int io_cycle;

    /* Dynamic state data */
    int state;
    int t_event;
    int tot_cpu;
    int tot_io;
    int t_finish;
    int next_ready;
} Process;

/* =================== GLOBALS =================== */
int casenum = 0;
int num_cpu, num_proc, quantum;
int nfinished;
int t, last_t;

CPU cpu[MAXCPU];
Process proc[MAXPROC];
int ready_head, ready_tail;

/* =================== FUNCTION DECLARATIONS =================== */
int getdata(void);
void display_heading(void);
void display_results(void);
void simulate(void);

/* Event functions (placeholders for later) */
void run_proc_fin(void);
void io_completed(void);
void do_submit(void);
void cpu_cycle_done(void);
void quantum_runout(void);
void schedule_cpus(void);
void do_preempt(void);
void set_next_event_time(void);

/* Ready queue helper */
void add_to_ready(int k);

/* =================== MAIN =================== */
int main(void)
{
    printf("Program 2:\n");
    for (casenum = 1; ; casenum++) {
        if (!getdata()) break;
        display_heading();
        simulate();
        display_results();
    }
    return 0;
}

/* =================== INPUT =================== */
int getdata(void)
{
    int i;
    if (scanf("%d", &num_cpu) != 1) return 0; // EOF
    if (scanf("%d%d", &num_proc, &quantum) != 2) return 0;

    for (i = 0; i < num_proc; i++) {
        scanf("%d%d%d%d%d%d",
              &proc[i].pid, &proc[i].prio, &proc[i].t_submit,
              &proc[i].tot_cpu_req, &proc[i].cpu_cycle, &proc[i].io_cycle);
    }
    return 1;
}

/* =================== DISPLAY =================== */
void display_heading(void)
{
    int i;
    if (casenum > 1) putchar('\n');
    printf("Simulation # %d\n", casenum);
    printf("--------------\n");
    printf("Input:\n");
    printf("     %d CPU%s, %d process%s, quantum size = %d\n",
           num_cpu, (num_cpu > 1) ? "s" : "",
           num_proc, (num_proc > 1) ? "es" : "", quantum);
    for (i = 0; i < num_proc; i++) {
        printf("     PID %d, prio = %d, submit = %d, totCPU = %d, CPU = %d, I/O = %d\n",
               proc[i].pid, proc[i].prio, proc[i].t_submit,
               proc[i].tot_cpu_req, proc[i].cpu_cycle, proc[i].io_cycle);
    }
}

void display_results(void)
{
    printf("Schedule Output:\n");
    printf("     (results will appear here once simulation works)\n");
}

/* =================== SIMULATION CORE =================== */
void simulate(void)
{
    printf("     [Simulation placeholder - not implemented yet]\n");
    // Here you’ll later add: init states, main event loop, etc.
}

/* =================== EVENT HANDLERS (EMPTY PLACEHOLDERS) =================== */
void run_proc_fin(void)     {}
void io_completed(void)     {}
void do_submit(void)        {}
void cpu_cycle_done(void)   {}
void quantum_runout(void)   {}
void schedule_cpus(void)    {}
void do_preempt(void)       {}
void set_next_event_time(void) {}

/* =================== READY QUEUE =================== */
void add_to_ready(int k)
{
    // Placeholder for ready queue insertion logic
}
