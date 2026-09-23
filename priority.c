#include <stdio.h>
#include "priority.h"

void priority_scheduling(Process *processes, int n)
{
    int current_time = 0;
    int completed = 0;

    int total_burst_time = 0;
    int first_start_time = -1;
    int final_completion_time = 0;

    printf("\n========================================\n");
    printf("       PRIORITY SCHEDULING\n");
    printf("========================================\n");

    /*
     * Reset scheduling information.
     */
    for (int i = 0; i < n; i++)
    {
        total_burst_time += processes[i].burst_time;

        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = 0;
        processes[i].start_time = -1;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].response_time = -1;
        processes[i].state = READY;
    }

    printf("\nExecution Order:\n");

    while (completed < n)
    {
        int selected = -1;

        /*
         * Find the highest-priority process
         * among processes that have arrived.
         *
         * Lower priority number = higher priority.
         */
        for (int i = 0; i < n; i++)
        {
            if (processes[i].state != TERMINATED &&
                processes[i].arrival_time <= current_time)
            {
                if (selected == -1 ||
                    processes[i].priority <
                    processes[selected].priority)
                {
                    selected = i;
                }
                /*
                 * Tie-breaker:
                 * If priority is equal, choose
                 * the earlier arrival time.
                 */
                else if (
                    processes[i].priority ==
                    processes[selected].priority &&
                    processes[i].arrival_time <
                    processes[selected].arrival_time)
                {
                    selected = i;
                }
            }
        }

        /*
         * No process has arrived yet.
         */
        if (selected == -1)
        {
            int next_arrival = -1;

            for (int i = 0; i < n; i++)
            {
                if (processes[i].state != TERMINATED)
                {
                    if (next_arrival == -1 ||
                        processes[i].arrival_time <
                        next_arrival)
                    {
                        next_arrival =
                            processes[i].arrival_time;
                    }
                }
            }

            printf("CPU Idle: %d -> %d\n",
                   current_time,
                   next_arrival);

            current_time = next_arrival;

            continue;
        }

        /*
         * Record the first time CPU starts.
         */
        if (first_start_time == -1)
        {
            first_start_time = current_time;
        }

        /*
         * Process enters RUNNING state.
         */
        processes[selected].state = RUNNING;

        processes[selected].start_time = current_time;

        /*
         * Response Time.
         */
        processes[selected].response_time =
            processes[selected].start_time -
            processes[selected].arrival_time;

        printf("P%d: %d -> %d "
               "(Priority %d)\n",
               processes[selected].pid,
               current_time,
               current_time +
               processes[selected].burst_time,
               processes[selected].priority);

        /*
         * Execute process completely.
         */
        current_time += processes[selected].burst_time;

        /*
         * Process completed.
         */
        processes[selected].completion_time =
            current_time;

        processes[selected].turnaround_time =
            processes[selected].completion_time -
            processes[selected].arrival_time;

        processes[selected].waiting_time =
            processes[selected].turnaround_time -
            processes[selected].burst_time;

        processes[selected].remaining_time = 0;

        processes[selected].state = TERMINATED;

        completed++;

        final_completion_time = current_time;
    }

    /*
     * CPU Utilization.
     */
    if (final_completion_time > first_start_time)
    {
        double cpu_utilization =
            ((double) total_burst_time /
             (final_completion_time -
              first_start_time)) * 100.0;

        printf("\nCPU Utilization: %.2f%%\n",
               cpu_utilization);
    }

    printf("\nPriority scheduling completed successfully.\n");
}
