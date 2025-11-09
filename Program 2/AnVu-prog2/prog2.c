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
typedef struct
{
    int idle;     // total CPU idle time
    int run_time; // time spent running current process (this quantum)
    int proc_ndx; // -1 if CPU idle, else process index
} CPU;

typedef struct
{
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
    for (casenum = 1;; casenum++)
    {
        if (!getdata())
            break;
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
    if (scanf("%d", &num_cpu) != 1)
        return 0; // EOF
    if (scanf("%d%d", &num_proc, &quantum) != 2)
        return 0;

    for (i = 0; i < num_proc; i++)
    {
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
    if (casenum > 1)
        putchar('\n');
    printf("Simulation # %d\n", casenum);
    printf("--------------\n");
    printf("Input:\n");
    printf("     %d CPU%s, %d process%s, quantum size = %d\n",
           num_cpu, (num_cpu > 1) ? "s" : "",
           num_proc, (num_proc > 1) ? "es" : "", quantum);
    for (i = 0; i < num_proc; i++)
    {
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
    int i;
    last_t = 0;
    t = proc[0].t_submit;
    nfinished = 0;

    // initialize processes
    for (i = 0; i < num_proc; i++)
    {
        proc[i].state = NOTYETSUBMITTED;
        proc[i].t_event = proc[i].t_submit;
        proc[i].next_ready = -1;
        proc[i].tot_cpu = 0;
        proc[i].tot_io = 0;
        proc[i].t_finish = -1;
    }

    // initialize CPUs
    for (i = 0; i < num_cpu; i++)
    {
        cpu[i].idle = 0;
        cpu[i].proc_ndx = -1;
        cpu[i].run_time = 0;
    }

    ready_head = ready_tail = -1;

    // jump to first submission
    do_submit();
    schedule_cpus();

    // main simulation loop
    while (nfinished < num_proc)
    {
        run_proc_fin();
        cpu_cycle_done();
        io_completed();
        do_submit();
        schedule_cpus();
        set_next_event_time();
    }
}

/* =================== EVENT HANDLERS (EMPTY PLACEHOLDERS) =================== */
void run_proc_fin(void)
{
    for (int i = 0; i < num_cpu; i++)
    {
        int p = cpu[i].proc_ndx;
        if (p == -1)
            continue;
        if (proc[p].state != RUNNING)
            continue;

        if (proc[p].tot_cpu >= proc[p].tot_cpu_req)
        {
            proc[p].state = COMPLETED;
            proc[p].t_finish = t;
            cpu[i].proc_ndx = -1;
            nfinished++;
            printf("*** %d: process %d running->done\n", t, proc[p].pid);
        }
    }
}

void io_completed(void)
{
    for (int i = 0; i < num_proc; i++)
    {
        if (proc[i].state != BLOCKED)
            continue;
        if (proc[i].t_event == t)
        {
            proc[i].t_event = -1;
            add_to_ready(i);
            printf("*** %d: process %d blocked->ready\n", t, proc[i].pid);
        }
    }
}

void do_submit(void)
{
    int i;
    for (i = 0; i < num_proc; i++)
    {
        if (proc[i].state != NOTYETSUBMITTED)
            continue;
        if (proc[i].t_event == t)
        {
            printf("*** %d: process %d submitted\n", t, proc[i].pid);
            proc[i].t_event = -1;
            add_to_ready(i);
        }
    }
}

void cpu_cycle_done(void)
{
    for (int i = 0; i < num_cpu; i++)
    {
        int p = cpu[i].proc_ndx;
        if (p == -1)
            continue;
        if (proc[p].state != RUNNING)
            continue;

        // If this process finishes a CPU cycle
        if (proc[p].tot_cpu < proc[p].tot_cpu_req &&
            proc[p].tot_cpu % proc[p].cpu_cycle == 0 &&
            proc[p].tot_cpu != 0)
        {

            proc[p].state = BLOCKED;
            proc[p].t_event = t + proc[p].io_cycle;
            cpu[i].proc_ndx = -1;
            printf("*** %d: process %d running(cpu %d)->blocked\n", t, proc[p].pid, i);
        }
    }
}

void quantum_runout(void) {}

void schedule_cpus(void)
{
    int i;

    for (i = 0; i < num_cpu; i++)
    {
        if (cpu[i].proc_ndx != -1)
            continue; // already running

        if (ready_head == -1)
            return; // nothing ready

        int p = ready_head;
        ready_head = proc[p].next_ready;
        if (ready_head == -1)
            ready_tail = -1;

        proc[p].state = RUNNING;
        cpu[i].proc_ndx = p;
        cpu[i].run_time = 0;

        printf("*** %d: process %d ready->running(cpu %d)\n", t, proc[p].pid, i);
    }
}

void do_preempt(void) {}
void set_next_event_time(void)
{
    int next_t = -1;

    // find next event time
    for (int i = 0; i < num_proc; i++)
    {
        if (proc[i].state == COMPLETED)
            continue;
        if (proc[i].t_event == -1)
            continue;
        if (next_t == -1 || proc[i].t_event < next_t)
            next_t = proc[i].t_event;
    }

    if (next_t == -1)
        return; // no more events

    last_t = t;
    t = next_t;

    // simulate CPU progress between last_t and t
    for (int i = 0; i < num_cpu; i++)
    {
        int p = cpu[i].proc_ndx;
        if (p == -1)
            continue;
        if (proc[p].state != RUNNING)
            continue;
        proc[p].tot_cpu += (t - last_t);
    }
}

/* =================== READY QUEUE =================== */
void add_to_ready(int k)
{
    printf("*** %d: process %d moved to ready\n", t, proc[k].pid);
}
