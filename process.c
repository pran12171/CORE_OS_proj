#include <stdio.h>
#include <stdlib.h>
#include "process.h"

Process *create_processes(int n)
{
    Process *processes = malloc(n * sizeof(Process));

    if (processes == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    return processes;
}

void initialize_process(Process *p, int pid, int arrival, int burst, int priority)
{
    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->remaining_time = burst;
    p->priority = priority;

    p->start_time = -1;
    p->completion_time = 0;

    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->response_time = -1;

    p->state = NEW;
}

const char *get_state_name(ProcessState state)
{
    switch (state)
    {
        case NEW:
            return "NEW";

        case READY:
            return "READY";

        case RUNNING:
            return "RUNNING";

        case WAITING:
            return "WAITING";

        case TERMINATED:
            return "TERMINATED";

        default:
            return "UNKNOWN";
    }
}

void display_processes(Process *processes, int n)
{
    printf("\n");
    printf("---------------------------------------------------------------\n");
    printf(" PID  Arrival  Burst  Remaining  Priority  State\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        printf(" %-4d %-8d %-6d %-10d %-9d %s\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].remaining_time,
               processes[i].priority,
               get_state_name(processes[i].state));
    }

    printf("---------------------------------------------------------------\n");
}

void free_processes(Process *processes)
{
    free(processes);
}
