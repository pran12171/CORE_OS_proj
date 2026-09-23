#include <stdio.h>

#include "process.h"
#include "fcfs.h"
#include "sjf.h"
#include "priority.h"
#include "round_robin.h"
#include "process_manager.h"
#include "ipc_pipe.h"
#include "fifo.h"
#include "signals.h"

int main()
{
    int n;
    int choice;

    printf("========================================\n");
    printf("   CPU SCHEDULING SIMULATION SYSTEM\n");
    printf("========================================\n");

    printf("\nEnter number of processes: ");
    scanf("%d", &n);

    if (n <= 0)
    {
        printf("Invalid number of processes.\n");
        return 1;
    }

    Process *processes = create_processes(n);

    /*
     * Input process information.
     */
    for (int i = 0; i < n; i++)
    {
        int arrival;
        int burst;
        int priority;

        printf("\nProcess P%d\n", i + 1);

        printf("Arrival Time: ");
        scanf("%d", &arrival);

        printf("Burst Time: ");
        scanf("%d", &burst);

        printf("Priority: ");
        scanf("%d", &priority);

        initialize_process(
            &processes[i],
            i + 1,
            arrival,
            burst,
            priority
        );

        processes[i].state = READY;
    }

    printf("\n========================================\n");
    printf("         SCHEDULING ALGORITHMS\n");
    printf("========================================\n");

    printf("1. FCFS\n");
    printf("2. SJF\n");
    printf("3. Priority Scheduling\n");
    printf("4. Round Robin\n");
    printf("5. Process Creation (fork/wait)\n");
    printf("6. Pipe IPC\n");
    printf("7. FIFO IPC\n");
    printf("8. Signal + Timer Demonstration\n");

    printf("\nEnter your choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
        case 1:
            fcfs(processes, n);
            break;

        case 2:
            sjf(processes, n);
            break;

        case 3:
            priority_scheduling(processes, n);
            break;

        case 4:
        {
            int quantum;

            printf("\nEnter Time Quantum: ");
            scanf("%d", &quantum);

            if (quantum <= 0)
            {
                printf("Invalid time quantum.\n");
                free_processes(processes);
                return 1;
            }

            round_robin(processes, n, quantum);

            break;
        }
        case 5:
    demonstrate_process_creation(&processes[0]);
    break;
    
    case 6:
    demonstrate_pipe(&processes[0]);
    break;
    
    case 7:
    demonstrate_fifo(&processes[0]);
    break;
    case 8:
    demonstrate_timer_signal();
    break;

        default:
            printf("Invalid choice.\n");
            free_processes(processes);
            return 1;
    }

    /*
     * Display final results.
     */
    printf("\n========================================\n");
    printf("             RESULTS\n");
    printf("========================================\n");

    printf("\n");
    printf("PID\tAT\tBT\tCT\tTAT\tWT\tRT\n");
    printf("---------------------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time);
    }

    free_processes(processes);

    return 0;
}
