#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef struct {
    int pid;

    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;

    int start_time;
    int completion_time;

    int waiting_time;
    int turnaround_time;
    int response_time;

    ProcessState state;
} Process;

Process *create_processes(int n);
void initialize_process(Process *p, int pid, int arrival, int burst, int priority);
void display_processes(Process *processes, int n);
void free_processes(Process *processes);

const char *get_state_name(ProcessState state);

#endif
