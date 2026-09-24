#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "fcfs.h"
#include "sjf.h"
#include "priority.h"
#include "round_robin.h"
#include "process_manager.h"
#include "ipc_pipe.h"
#include "fifo.h"
#include "signals.h"
#include "dashboard.h"


/*
 * Display the current process table.
 */
void display_process_table(Process *processes, int n)
{
    display_section_header("PROCESS TABLE");

    if (processes == NULL || n <= 0)
    {
        printf("\nNo processes have been added yet.\n");
        return;
    }

    printf("\n");
    printf("PID\tAT\tBT\tPriority\tState\n");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        char *state;

        switch (processes[i].state)
        {
            case NEW:
                state = "NEW";
                break;

            case READY:
                state = "READY";
                break;

            case RUNNING:
                state = "RUNNING";
                break;

            case WAITING:
                state = "WAITING";
                break;

            case TERMINATED:
                state = "TERMINATED";
                break;

            default:
                state = "UNKNOWN";
                break;
        }

        printf(
            "P%d\t%d\t%d\t%d\t\t%s\n",
            processes[i].pid,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].priority,
            state
        );
    }
}


/*
 * Display process states.
 */
void display_process_states(Process *processes, int n)
{
    display_section_header("PROCESS STATES");

    if (processes == NULL || n <= 0)
    {
        printf("\nNo processes have been added yet.\n");
        return;
    }

    printf("\n");
    printf("PID\tCurrent State\n");
    printf("-------------------------\n");

    for (int i = 0; i < n; i++)
    {
        char *state;

        switch (processes[i].state)
        {
            case NEW:
                state = "NEW";
                break;

            case READY:
                state = "READY";
                break;

            case RUNNING:
                state = "RUNNING";
                break;

            case WAITING:
                state = "WAITING";
                break;

            case TERMINATED:
                state = "TERMINATED";
                break;

            default:
                state = "UNKNOWN";
                break;
        }

        printf(
            "P%d\t%s\n",
            processes[i].pid,
            state
        );
    }
}


/*
 * Display scheduling metrics.
 */
void display_metrics(Process *processes, int n)
{
    display_section_header("SCHEDULING METRICS");

    if (processes == NULL || n <= 0)
    {
        printf("\nNo processes have been added yet.\n");
        return;
    }

    double total_waiting = 0;
    double total_turnaround = 0;
    double total_response = 0;

    printf("\n");
    printf("PID\tWT\tTAT\tRT\n");
    printf("--------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        printf(
            "P%d\t%d\t%d\t%d\n",
            processes[i].pid,
            processes[i].waiting_time,
            processes[i].turnaround_time,
            processes[i].response_time
        );

        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
        total_response += processes[i].response_time;
    }

    printf("--------------------------------\n");

    printf(
        "\nAverage Waiting Time    : %.2f\n",
        total_waiting / n
    );

    printf(
        "Average Turnaround Time : %.2f\n",
        total_turnaround / n
    );

    printf(
        "Average Response Time   : %.2f\n",
        total_response / n
    );
}


int main()
{
    int n = 0;
    int choice;

    Process *processes = NULL;


    /*
     * MAIN DASHBOARD LOOP
     */
    while (1)
    {
        display_dashboard();

        printf("\nEnter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input. Please enter a number.\n");

            while (getchar() != '\n')
            {
                /* Clear input buffer */
            }

            pause_screen();
            continue;
        }


        switch (choice)
        {

            /*
             * ==================================================
             * 1. ADD PROCESSES
             * ==================================================
             */
            case 1:
            {
                /*
                 * If processes already exist,
                 * free the old process list.
                 */
                if (processes != NULL)
                {
                    free_processes(processes);
                    processes = NULL;
                    n = 0;
                }


                printf("\n");
                printf("============================================================\n");
                printf("                     ADD PROCESSES\n");
                printf("============================================================\n");


                printf("\nEnter number of processes: ");

                if (scanf("%d", &n) != 1)
                {
                    printf("\nInvalid input.\n");

                    while (getchar() != '\n')
                    {
                        /* Clear buffer */
                    }

                    n = 0;
                    pause_screen();
                    break;
                }


                if (n <= 0)
                {
                    printf("\nInvalid number of processes.\n");

                    n = 0;

                    pause_screen();
                    break;
                }


                /*
                 * Dynamically create processes.
                 */
                processes = create_processes(n);


                if (processes == NULL)
                {
                    printf("\nMemory allocation failed.\n");

                    n = 0;

                    pause_screen();
                    break;
                }


                /*
                 * Input only Arrival Time and Burst Time.
                 *
                 * Priority is NOT entered here.
                 * It will be requested only when
                 * Priority Scheduling is selected.
                 */
                for (int i = 0; i < n; i++)
                {
                    int arrival;
                    int burst;


                    printf("\n");
                    printf("-----------------------------\n");
                    printf("          Process P%d\n", i + 1);
                    printf("-----------------------------\n");


                    printf("Arrival Time: ");
                    scanf("%d", &arrival);


                    printf("Burst Time: ");
                    scanf("%d", &burst);


                    /*
                     * Priority is initially set to 0.
                     * It will be updated later if the user
                     * selects Priority Scheduling.
                     */
                    initialize_process(
                        &processes[i],
                        i + 1,
                        arrival,
                        burst,
                        0
                    );


                    processes[i].state = READY;
                }


                printf("\n");
                printf("Processes added successfully.\n");


                pause_screen();

                break;
            }


            /*
             * ==================================================
             * 2. VIEW PROCESS TABLE
             * ==================================================
             */
            case 2:

                display_process_table(processes, n);

                pause_screen();

                break;


            /*
             * ==================================================
             * 3. PROCESS STATES
             * ==================================================
             */
            case 3:

                display_process_states(processes, n);

                pause_screen();

                break;


            /*
             * ==================================================
             * 4. FCFS
             * ==================================================
             */
            case 4:

                if (processes == NULL)
                {
                    printf("\nPlease add processes first.\n");

                    pause_screen();

                    break;
                }


                display_section_header("FCFS SCHEDULING");


                /*
                 * No priority is required for FCFS.
                 */
                fcfs(processes, n);


                pause_screen();

                break;


            /*
             * ==================================================
             * 5. SJF
             * ==================================================
             */
            case 5:

                if (processes == NULL)
                {
                    printf("\nPlease add processes first.\n");

                    pause_screen();

                    break;
                }


                display_section_header("SJF SCHEDULING");


                /*
                 * No priority is required for SJF.
                 */
                sjf(processes, n);


                pause_screen();

                break;


            /*
             * ==================================================
             * 6. PRIORITY SCHEDULING
             * ==================================================
             */
            case 6:

                if (processes == NULL)
                {
                    printf("\nPlease add processes first.\n");

                    pause_screen();

                    break;
                }


                display_section_header("PRIORITY SCHEDULING");


                /*
                 * Priority is requested ONLY here.
                 */
                printf("\nEnter Priority for each process:\n");


                for (int i = 0; i < n; i++)
                {
                    printf(
                        "Priority for P%d: ",
                        processes[i].pid
                    );

                    scanf(
                        "%d",
                        &processes[i].priority
                    );
                }


                printf("\nRunning Priority Scheduling...\n");


                priority_scheduling(processes, n);


                pause_screen();

                break;


            /*
             * ==================================================
             * 7. ROUND ROBIN
             * ==================================================
             */
            case 7:
            {
                int quantum;


                if (processes == NULL)
                {
                    printf("\nPlease add processes first.\n");

                    pause_screen();

                    break;
                }


                display_section_header(
                    "ROUND ROBIN SCHEDULING"
                );


                /*
                 * Round Robin requires Time Quantum.
                 */
                printf("\nEnter Time Quantum: ");

                scanf("%d", &quantum);


                if (quantum <= 0)
                {
                    printf(
                        "\nInvalid time quantum.\n"
                    );

                    pause_screen();

                    break;
                }


                round_robin(
                    processes,
                    n,
                    quantum
                );


                pause_screen();

                break;
            }


            /*
             * ==================================================
             * 8. GANTT CHART
             * ==================================================
             */
            case 8:

                display_section_header("GANTT CHART");


                printf("\n");
                printf(
                    "The Gantt Chart is generated during\n"
                );

                printf(
                    "FCFS, SJF, Priority, and Round Robin\n"
                );

                printf(
                    "scheduling execution.\n"
                );


                printf("\nRun a scheduling algorithm first.\n");


                pause_screen();

                break;


            /*
             * ==================================================
             * 9. SCHEDULING METRICS
             * ==================================================
             */
            case 9:

                display_metrics(processes, n);

                pause_screen();

                break;


            /*
             * ==================================================
             * 10. COMPARE ALGORITHMS
             * ==================================================
             */
            case 10:

                display_section_header(
                    "ALGORITHM COMPARISON"
                );


                printf("\n");
                printf(
                    "Algorithm Comparison Dashboard\n"
                );

                printf(
                    "--------------------------------------------\n"
                );

                printf(
                    "\nFCFS\n"
                );

                printf(
                    "First Come First Served\n"
                );


                printf(
                    "\nSJF\n"
                );

                printf(
                    "Shortest Job First\n"
                );


                printf(
                    "\nPriority Scheduling\n"
                );

                printf(
                    "Processes are selected according to priority.\n"
                );


                printf(
                    "\nRound Robin\n"
                );

                printf(
                    "Processes receive CPU time according to a\n"
                );

                printf(
                    "fixed time quantum.\n"
                );


                printf(
                    "\nFull numerical comparison will be connected\n"
                );

                printf(
                    "in the next dashboard phase.\n"
                );


                pause_screen();

                break;


            /*
             * ==================================================
             * 11. PROCESS CREATION
             * ==================================================
             */
            case 11:

                if (processes == NULL)
                {
                    printf(
                        "\nPlease add processes first.\n"
                    );

                    pause_screen();

                    break;
                }


                display_section_header(
                    "PROCESS CREATION - FORK/WAIT"
                );


                demonstrate_process_creation(
                    &processes[0]
                );


                pause_screen();

                break;


            /*
             * ==================================================
             * 12. PIPE IPC
             * ==================================================
             */
            case 12:

                if (processes == NULL)
                {
                    printf(
                        "\nPlease add processes first.\n"
                    );

                    pause_screen();

                    break;
                }


                display_section_header(
                    "PIPE IPC"
                );


                demonstrate_pipe(
                    &processes[0]
                );


                pause_screen();

                break;


            /*
             * ==================================================
             * 13. FIFO IPC
             * ==================================================
             */
            case 13:

                if (processes == NULL)
                {
                    printf(
                        "\nPlease add processes first.\n"
                    );

                    pause_screen();

                    break;
                }


                display_section_header(
                    "FIFO IPC"
                );


                demonstrate_fifo(
                    &processes[0]
                );


                pause_screen();

                break;


            /*
             * ==================================================
             * 14. SIGNAL + TIMER
             * ==================================================
             */
            case 14:

                display_section_header(
                    "SIGNAL + TIMER DEMONSTRATION"
                );


                demonstrate_timer_signal();


                pause_screen();

                break;


            /*
             * ==================================================
             * 0. EXIT
             * ==================================================
             */
            case 0:

                if (processes != NULL)
                {
                    free_processes(processes);
                }


                printf("\n");
                printf(
                    "============================================================\n"
                );

                printf(
                    "     Thank you for using CPU Scheduling Simulation System\n"
                );

                printf(
                    "============================================================\n"
                );

                printf("\nExiting...\n");


                return 0;


            /*
             * ==================================================
             * INVALID OPTION
             * ==================================================
             */
            default:

                printf(
                    "\nInvalid choice. Please select "
                    "a valid option.\n"
                );


                pause_screen();

                break;
        }
    }


    return 0;
}
