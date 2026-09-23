#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include "round_robin.h"

volatile sig_atomic_t quantum_expired = 0;

static void handle_quantum_expiration(int signal_number)
{
    (void)signal_number;
    quantum_expired = 1;
}

/* ---------- Circular Queue ---------- */

typedef struct
{
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} ReadyQueue;

static int queue_init(ReadyQueue *q, int capacity)
{
    q->data = malloc(capacity * sizeof(int));

    if (q->data == NULL)
        return 0;

    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;

    return 1;
}

static int queue_empty(ReadyQueue *q)
{
    return q->size == 0;
}

static int queue_full(ReadyQueue *q)
{
    return q->size == q->capacity;
}

static int queue_push(ReadyQueue *q, int value)
{
    if (queue_full(q))
        return 0;

    q->data[q->rear] = value;

    q->rear = (q->rear + 1) % q->capacity;

    q->size++;

    return 1;
}

static int queue_pop(ReadyQueue *q)
{
    if (queue_empty(q))
        return -1;

    int value = q->data[q->front];

    q->front = (q->front + 1) % q->capacity;

    q->size--;

    return value;
}

static void queue_destroy(ReadyQueue *q)
{
    free(q->data);
}

/* ---------- Timer ---------- */

static void start_timer(int seconds)
{
    struct itimerval timer;

    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer");
    }
}

static void stop_timer(void)
{
    struct itimerval timer = {0};

    setitimer(ITIMER_REAL, &timer, NULL);
}

/* ---------- Round Robin ---------- */

void round_robin(Process *processes, int n, int quantum)
{
    int current_time = 0;
    int completed = 0;

    int total_burst_time = 0;
    int first_start_time = -1;
    int final_completion_time = 0;

    ReadyQueue queue;

    /*
     * We use n + 1 slots so that the queue
     * always has enough space for rotation.
     */
    if (!queue_init(&queue, n + 1))
    {
        printf("Ready queue memory allocation failed.\n");
        return;
    }

    if (signal(SIGALRM, handle_quantum_expiration) == SIG_ERR)
    {
        perror("signal");
        queue_destroy(&queue);
        return;
    }

    /*
     * Reset all scheduling values.
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
    printf("       ROUND ROBIN SCHEDULING\n");
    printf("       SIGNAL + TIMER ENABLED\n");
    printf("========================================\n");

    printf("\nTime Quantum: %d second(s)\n", quantum);

    /*
     * Add processes already available at time 0.
     */
    for (int i = 0; i < n; i++)
    {
        if (processes[i].arrival_time <= current_time)
        {
            queue_push(&queue, i);
        }
    }

    while (completed < n)
    {
        /*
         * If there is nothing ready,
         * advance to the next process arrival.
         */
        if (queue_empty(&queue))
        {
            int next_arrival = -1;

            for (int i = 0; i < n; i++)
            {
                if (processes[i].state != TERMINATED &&
                    processes[i].remaining_time > 0)
                {
                    if (next_arrival == -1 ||
                        processes[i].arrival_time < next_arrival)
                    {
                        next_arrival =
                            processes[i].arrival_time;
                    }
                }
            }

            if (next_arrival > current_time)
            {
                printf("\nCPU Idle: %d -> %d\n",
                       current_time,
                       next_arrival);

                current_time = next_arrival;
            }

            /*
             * Add newly arrived processes.
             */
            for (int i = 0; i < n; i++)
            {
                if (processes[i].state != TERMINATED &&
                    processes[i].remaining_time > 0 &&
                    processes[i].arrival_time <= current_time)
                {
                    queue_push(&queue, i);
                }
            }

            continue;
        }

        /*
         * Select next process.
         */
        int index = queue_pop(&queue);

        if (index == -1)
            continue;

        /*
         * Record first response time.
         */
        if (processes[index].start_time == -1)
        {
            processes[index].start_time = current_time;

            processes[index].response_time =
                current_time -
                processes[index].arrival_time;
        }

        processes[index].state = RUNNING;

        /*
         * Determine this process's time slice.
         */
        int slice = processes[index].remaining_time;

        if (slice > quantum)
        {
            slice = quantum;
        }

        printf("\nP%d starts at time %d\n",
               processes[index].pid,
               current_time);

        printf("Starting timer for %d second(s)...\n",
               slice);

        quantum_expired = 0;

        /*
         * Start Linux timer.
         */
        start_timer(slice);

        /*
         * Wait for SIGALRM.
         */
        while (!quantum_expired)
        {
            pause();
        }

        stop_timer();

        printf("SIGALRM received: time slice expired.\n");

        /*
         * Advance simulated CPU time.
         */
        current_time += slice;

        processes[index].remaining_time -= slice;

        printf("P%d executed: %d -> %d\n",
               processes[index].pid,
               current_time - slice,
               current_time);

        /*
         * Add processes that arrived during
         * this time slice.
         */
        for (int i = 0; i < n; i++)
        {
            if (i != index &&
                processes[i].state != TERMINATED &&
                processes[i].remaining_time > 0 &&
                processes[i].arrival_time <= current_time)
            {
                /*
                 * A simple duplicate check.
                 */
                int already_present = 0;

                for (int j = 0; j < queue.size; j++)
                {
                    int position =
                        (queue.front + j) %
                        queue.capacity;

                    if (queue.data[position] == i)
                    {
                        already_present = 1;
                        break;
                    }
                }

                if (!already_present)
                {
                    queue_push(&queue, i);
                }
            }
        }

        /*
         * Process completed.
         */
        if (processes[index].remaining_time == 0)
        {
            processes[index].completion_time =
                current_time;

            processes[index].turnaround_time =
                processes[index].completion_time -
                processes[index].arrival_time;

            processes[index].waiting_time =
                processes[index].turnaround_time -
                processes[index].burst_time;

            processes[index].state = TERMINATED;

            completed++;

            final_completion_time = current_time;

            printf("P%d TERMINATED.\n",
                   processes[index].pid);
        }
        else
        {
            /*
             * Process still has work.
             * Put it at the end of the queue.
             */
            processes[index].state = READY;

            queue_push(&queue, index);

            printf("P%d moved back to READY queue.\n",
                   processes[index].pid);
        }
    }

    /*
     * CPU utilization.
     */
    if (final_completion_time > first_start_time)
    {
        double cpu_utilization =
            ((double)total_burst_time /
             (final_completion_time -
              first_start_time)) * 100.0;

        printf("\nCPU Utilization: %.2f%%\n",
               cpu_utilization);
    }

    stop_timer();

    queue_destroy(&queue);

    printf("\nRound Robin scheduling completed successfully.\n");
}

