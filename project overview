# CPU Scheduling Simulation System

A C-based Operating Systems and Systems Programming project that simulates CPU scheduling algorithms and demonstrates important Linux/POSIX Operating System concepts.

## Project Overview

The **CPU Scheduling Simulation System** demonstrates how an Operating System manages multiple processes and allocates CPU time using different scheduling techniques.

The system implements four major CPU scheduling algorithms:

- First Come First Serve (FCFS)
- Shortest Job First (SJF)
- Priority Scheduling
- Round Robin

The simulator maintains process information such as:

- Process ID
- Arrival Time
- Burst Time
- Priority
- Remaining Time
- Process State
- Completion Time
- Turnaround Time
- Waiting Time
- Response Time

It also demonstrates process management, inter-process communication, dynamic memory allocation, signals, and timer-based CPU scheduling.

---

## Features

### CPU Scheduling Algorithms

| Algorithm | Description |
|---|---|
| FCFS | Executes processes according to their arrival order |
| SJF | Selects the ready process with the shortest burst time |
| Priority | Selects processes according to priority |
| Round Robin | Allocates CPU time using a fixed time quantum |

### Process Management

The project demonstrates:

- Process Control Block (PCB)
- Dynamic memory allocation
- Process creation using `fork()`
- Parent-child process synchronization
- Process state tracking

### Inter-Process Communication

The system demonstrates:

- Anonymous Pipes
- Named Pipes (FIFO)

### Signal and Timer Handling

Signals and timers are used to demonstrate CPU timer interrupts and Round Robin time-slice expiration.

The project uses:

- POSIX signals
- `SIGALRM`
- `setitimer()`

### Performance Metrics

The simulator calculates:

- Completion Time (CT)
- Turnaround Time (TAT)
- Waiting Time (WT)
- Response Time (RT)
- CPU Utilization

### Gantt Charts

Scheduling results are represented using Gantt charts to visualize:

- Process execution order
- CPU idle periods
- Process execution intervals
- Scheduling timeline

---

## System Architecture

```text
                    CPU Scheduling Simulator
                              |
             +----------------+----------------+
             |                                 |
       Scheduling Module                 OS Concepts
             |                                 |
     +-------+-------+                 +-------+-------+
     |       |       |                 |       |       |
    FCFS    SJF   Priority            IPC    Signals  Process
     |       |       |                 |       |      Management
     +-------+-------+                 |       |
             |                         |       |
        Round Robin                  Pipe/FIFO Timer
             |                         |
             +------------+------------+
                          |
                    Process / PCB
                          |
                 Performance Metrics
                          |
                    Gantt Chart
