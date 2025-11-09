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
    for (int i = 0; i < num_cpu; i++) {
        if (cpu[i].proc_ndx != -1) continue;   // CPU busy
        if (ready_head == -1) return;          // nothing ready

        int p = ready_head;
        ready_head = proc[p].next_ready;
        if (ready_head == -1) ready_tail = -1;

        proc[p].state = RUNNING;
        cpu[i].proc_ndx = p;
        cpu[i].run_time = 0;

        // compute how long until next CPU-cycle boundary or process finish
        int rem_to_finish = proc[p].tot_cpu_req - proc[p].tot_cpu;
        int in_cycle = proc[p].tot_cpu % proc[p].cpu_cycle;       // 0 at cycle start
        int rem_to_cycle = (in_cycle == 0) ? proc[p].cpu_cycle    : (proc[p].cpu_cycle - in_cycle);

        int delta = (rem_to_finish < rem_to_cycle) ? rem_to_finish : rem_to_cycle;
        proc[p].t_event = t + delta;   // quantum ignored for now (Stage 2)

        printf("*** %d: process %d ready->running(cpu %d)\n", t, proc[p].pid, i);
    }
}

void do_preempt(void) {}
void set_next_event_time(void)
{
    int next_t = -1;

    // find earliest pending event among all processes
    for (int i = 0; i < num_proc; i++) {
        if (proc[i].state == COMPLETED) continue;
        if (proc[i].t_event == -1) continue;
        if (next_t == -1 || proc[i].t_event < next_t) next_t = proc[i].t_event;
    }

    if (next_t == -1) return; // no more events

    last_t = t;
    t = next_t;

    int dt = t - last_t;

    // update CPU run/idle and tot_cpu for running procs
    for (int i = 0; i < num_cpu; i++) {
        int p = cpu[i].proc_ndx;
        if (p == -1) {
            cpu[i].idle += dt;
        } else if (proc[p].state == RUNNING) {
            proc[p].tot_cpu += dt;
            cpu[i].run_time += dt;
        }
    }
}

/* =================== READY QUEUE =================== */
void add_to_ready(int k)
{
    int prev = -1, curr = ready_head;

    proc[k].state = READY;

    // insert by descending priority, tie-breaker: smaller PID first
    while (curr != -1) {
        if (proc[k].prio > proc[curr].prio) break;
        if (proc[k].prio == proc[curr].prio && proc[k].pid < proc[curr].pid) break;
        prev = curr;
        curr = proc[curr].next_ready;
    }

    if (prev == -1) ready_head = k;
    else proc[prev].next_ready = k;

    proc[k].next_ready = curr;
    if (curr == -1) ready_tail = k;

    printf("*** %d: process %d moved to ready\n", t, proc[k].pid);
}
