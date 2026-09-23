#include <stdio.h>

#include "fcfs.h"
#include "gantt.h"

/* Swap two Process structures */
static void swap_processes(Process *a, Process *b)
{
    Process temp = *a;
    *a = *b;
    *b = temp;
}

/* Sort processes according to Arrival Time */
static void sort_by_arrival(Process *processes, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (processes[j].arrival_time >
                processes[j + 1].arrival_time)
            {
                swap_processes(
                    &processes[j],
                    &processes[j + 1]
                );
            }
        }
    }
}

void fcfs(Process *processes, int n)
{
    int current_time = 0;
    int total_burst_time = 0;
    int first_start_time = -1;
    int final_completion_time = 0;

    /*
     * Initialize unified Gantt chart.
     */
    GanttChart chart;
    gantt_init(&chart, n + 1);

    /*
     * Arrange processes according to arrival time.
     */
    sort_by_arrival(processes, n);

    printf("\n========================================\n");
    printf("           FCFS SCHEDULING\n");
    printf("========================================\n");

    printf("\nExecution Order:\n");

    for (int i = 0; i < n; i++)
    {
        /*
         * CPU remains idle if the next process
         * has not arrived yet.
         */
        if (current_time < processes[i].arrival_time)
        {
            printf("CPU Idle: %d -> %d\n",
                   current_time,
                   processes[i].arrival_time);

            /*
             * Record CPU idle period in Gantt chart.
             * PID 0 represents IDLE.
             */
            gantt_add(
                &chart,
                0,
                current_time,
                processes[i].arrival_time
            );

            current_time = processes[i].arrival_time;
        }

        /*
         * Record first CPU execution time.
         */
        if (first_start_time == -1)
        {
            first_start_time = current_time;
        }

        /*
         * Process enters RUNNING state.
         */
        processes[i].state = RUNNING;

        processes[i].start_time = current_time;

        /*
         * Save execution start time for Gantt chart.
         */
        int execution_start = current_time;

        /*
         * Response Time.
         */
        processes[i].response_time =
            processes[i].start_time -
            processes[i].arrival_time;

        /*
         * Display execution.
         */
        printf("P%d: %d -> %d\n",
               processes[i].pid,
               current_time,
               current_time +
               processes[i].burst_time);

        /*
         * CPU executes process.
         */
        current_time += processes[i].burst_time;

        /*
         * Add process execution to Gantt chart.
         */
        gantt_add(
            &chart,
            processes[i].pid,
            execution_start,
            current_time
        );

        /*
         * Process terminates.
         */
        processes[i].completion_time = current_time;

        processes[i].turnaround_time =
            processes[i].completion_time -
            processes[i].arrival_time;

        processes[i].waiting_time =
            processes[i].turnaround_time -
            processes[i].burst_time;

        processes[i].remaining_time = 0;

        processes[i].state = TERMINATED;

        total_burst_time += processes[i].burst_time;

        final_completion_time = current_time;
    }

    /*
     * CPU Utilization.
     *
     * Total CPU busy time /
     * total elapsed scheduling time.
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

    /*
     * Display unified Gantt chart.
     */
    gantt_display(&chart);

    /*
     * Free Gantt chart memory.
     */
    gantt_free(&chart);
}
