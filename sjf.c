#include <stdio.h>

#include "sjf.h"
#include "gantt.h"

void sjf(Process *processes, int n)
{
    int current_time = 0;
    int completed = 0;

    int total_burst_time = 0;
    int first_start_time = -1;
    int final_completion_time = 0;

    /*
     * Initialize unified Gantt chart.
     */
    GanttChart chart;
    gantt_init(&chart, n + 1);

    /*
     * Reset scheduling information.
     */
    for (int i = 0; i < n; i++)
    {
        total_burst_time += processes[i].burst_time;

        processes[i].remaining_time =
            processes[i].burst_time;

        processes[i].completion_time = 0;
        processes[i].start_time = -1;
        processes[i].waiting_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].response_time = -1;

        processes[i].state = READY;
    }

    printf("\n========================================\n");
    printf("           SJF SCHEDULING\n");
    printf("========================================\n");

    printf("\nExecution Order:\n");

    while (completed < n)
    {
        int selected = -1;

        /*
         * Find the shortest available process.
         *
         * Tie-breaking:
         * 1. Shorter burst time
         * 2. Earlier arrival time
         * 3. Lower PID
         */
        for (int i = 0; i < n; i++)
        {
            if (processes[i].state != TERMINATED &&
                processes[i].arrival_time <= current_time)
            {
                if (selected == -1)
                {
                    selected = i;
                }
                else if (
                    processes[i].burst_time <
                    processes[selected].burst_time)
                {
                    selected = i;
                }
                else if (
                    processes[i].burst_time ==
                        processes[selected].burst_time &&
                    processes[i].arrival_time <
                        processes[selected].arrival_time)
                {
                    selected = i;
                }
                else if (
                    processes[i].burst_time ==
                        processes[selected].burst_time &&
                    processes[i].arrival_time ==
                        processes[selected].arrival_time &&
                    processes[i].pid <
                        processes[selected].pid)
                {
                    selected = i;
                }
            }
        }

        /*
         * No process is available.
         * CPU remains idle until the next arrival.
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

            /*
             * Add idle period to Gantt chart.
             * PID 0 = CPU IDLE.
             */
            gantt_add(
                &chart,
                0,
                current_time,
                next_arrival
            );

            current_time = next_arrival;

            continue;
        }

        /*
         * Record first CPU start time.
         */
        if (first_start_time == -1)
        {
            first_start_time = current_time;
        }

        /*
         * Process enters RUNNING state.
         */
        processes[selected].state = RUNNING;

        processes[selected].start_time =
            current_time;

        int execution_start = current_time;

        /*
         * Response Time.
         */
        processes[selected].response_time =
            processes[selected].start_time -
            processes[selected].arrival_time;

        printf("P%d: %d -> %d\n",
               processes[selected].pid,
               current_time,
               current_time +
                   processes[selected].burst_time);

        /*
         * Execute process completely.
         */
        current_time +=
            processes[selected].burst_time;

        /*
         * Add execution segment to Gantt chart.
         */
        gantt_add(
            &chart,
            processes[selected].pid,
            execution_start,
            current_time
        );

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
            ((double)total_burst_time /
             (final_completion_time -
              first_start_time)) *
            100.0;

        printf("\nCPU Utilization: %.2f%%\n",
               cpu_utilization);
    }

    /*
     * Display unified Gantt chart.
     */
    gantt_display(&chart);

    /*
     * Free Gantt chart memory.
     */
    gantt_free(&chart);
}
