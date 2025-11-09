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

// Toggle color output
#define COLOR 1
#if COLOR
#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"
#else
#define RED ""
#define BLUE ""
#define RESET ""
#endif

// Toggle this to 1 if you want to show trace logs (*** ...), 0 to hide them
#define TRACE 0 // Change to 1 if need debug

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
void simulate(void);

/* Event functions */
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
    printf(BLUE "CSCI 4500 Program 2 by " RESET RED "An Vu" RESET "\n");
    printf("\n");
    printf(BLUE "Program 2:" RESET "\n");
    for (casenum = 1;; casenum++)
    {
        if (!getdata())
            break;
        display_heading();
        simulate();
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
    printf(BLUE "Simulation # " RESET RED "%d" RESET "\n", casenum);
    printf(BLUE "--------------" RESET "\n");
    printf(BLUE "Input:" RESET "\n");
    printf("     " RED "%d" RESET " " BLUE "CPU%s, " RESET RED "%d" RESET " " BLUE "process%s, quantum size = " RESET RED "%d" RESET "\n",
           num_cpu, (num_cpu > 1) ? "s" : "",
           num_proc, (num_proc > 1) ? "es" : "", quantum);
    for (i = 0; i < num_proc; i++)
    {
        printf("     " BLUE "PID " RESET RED "%d" RESET ", " BLUE "prio = " RESET RED "%d" RESET ", " BLUE "submit = " RESET RED "%d" RESET ", " BLUE "totCPU = " RESET RED "%d" RESET ", " BLUE "CPU = " RESET RED "%d" RESET ", " BLUE "I/O = " RESET RED "%d" RESET "\n",
               proc[i].pid, proc[i].prio, proc[i].t_submit,
               proc[i].tot_cpu_req, proc[i].cpu_cycle, proc[i].io_cycle);
    }
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

    // first process submission
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

    // --- after simulation loop ends ---
    int total_turnaround = 0;
    int total_idle = 0;
    int t_last_finish = 0;

    for (int i = 0; i < num_proc; i++)
    {
        int turnaround = proc[i].t_finish - proc[i].t_submit;
        total_turnaround += turnaround;
        if (proc[i].t_finish > t_last_finish)
            t_last_finish = proc[i].t_finish;
    }

    for (int i = 0; i < num_cpu; i++)
        total_idle += cpu[i].idle;

    double avg_turnaround = (double)total_turnaround / num_proc;
    double avg_idle = (double)total_idle / num_cpu;
    double idle_percent = (avg_idle / t_last_finish) * 100.0;

    printf(BLUE "Schedule Output:" RESET "\n");
    for (int i = 0; i < num_proc; i++)
    {
        int turnaround = proc[i].t_finish - proc[i].t_submit;
        printf("     " BLUE "PID " RESET RED "%d" RESET " " BLUE "completed execution at " RESET RED "%d" RESET ", " BLUE "turnaround time = " RESET RED "%d" RESET "\n",
               proc[i].pid, proc[i].t_finish, turnaround);
    }
    printf("     " BLUE "Average CPU idle time = " RESET RED "%.0f" RESET " (" RED "%.0f%%" RESET ")\n",
           avg_idle, idle_percent);
    printf("     " BLUE "Average process turnaround time = " RESET RED "%.0f" RESET "\n",
           avg_turnaround);
}

/* =================== EVENT HANDLERS =================== */
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
            if (TRACE)
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
            if (TRACE)
                printf("*** %d: process %d blocked->ready\n", t, proc[i].pid);
        }
    }
}

void do_submit(void)
{
    for (int i = 0; i < num_proc; i++)
    {
        if (proc[i].state != NOTYETSUBMITTED)
            continue;
        if (proc[i].t_event == t)
        {
            if (TRACE)
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

        if (proc[p].tot_cpu < proc[p].tot_cpu_req &&
            proc[p].tot_cpu % proc[p].cpu_cycle == 0 &&
            proc[p].tot_cpu != 0)
        {
            proc[p].state = BLOCKED;
            proc[p].t_event = t + proc[p].io_cycle;
            cpu[i].proc_ndx = -1;
            if (TRACE)
                printf("*** %d: process %d running(cpu %d)->blocked\n", t, proc[p].pid, i);
        }
    }
}

void quantum_runout(void) {}

void schedule_cpus(void)
{
    for (int i = 0; i < num_cpu; i++)
    {
        if (cpu[i].proc_ndx != -1)
            continue;
        if (ready_head == -1)
            return;

        int p = ready_head;
        ready_head = proc[p].next_ready;
        if (ready_head == -1)
            ready_tail = -1;

        proc[p].state = RUNNING;
        cpu[i].proc_ndx = p;
        cpu[i].run_time = 0;

        int rem_to_finish = proc[p].tot_cpu_req - proc[p].tot_cpu;
        int in_cycle = proc[p].tot_cpu % proc[p].cpu_cycle;
        int rem_to_cycle = (in_cycle == 0) ? proc[p].cpu_cycle : (proc[p].cpu_cycle - in_cycle);
        int delta = (rem_to_finish < rem_to_cycle) ? rem_to_finish : rem_to_cycle;
        proc[p].t_event = t + delta;

        if (TRACE)
            printf("*** %d: process %d ready->running(cpu %d)\n", t, proc[p].pid, i);
    }
}

void do_preempt(void) {}

void set_next_event_time(void)
{
    int next_t = -1;
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
        return;

    last_t = t;
    t = next_t;
    int dt = t - last_t;

    for (int i = 0; i < num_cpu; i++)
    {
        int p = cpu[i].proc_ndx;
        if (p == -1)
            cpu[i].idle += dt;
        else if (proc[p].state == RUNNING)
        {
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

    while (curr != -1)
    {
        if (proc[k].prio > proc[curr].prio)
            break;
        if (proc[k].prio == proc[curr].prio && proc[k].pid < proc[curr].pid)
            break;
        prev = curr;
        curr = proc[curr].next_ready;
    }

    if (prev == -1)
        ready_head = k;
    else
        proc[prev].next_ready = k;

    proc[k].next_ready = curr;
    if (curr == -1)
        ready_tail = k;

    if (TRACE)
        printf("*** %d: process %d moved to ready\n", t, proc[k].pid);
}
