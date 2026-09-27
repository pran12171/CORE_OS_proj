
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

static int queue_empty(const ReadyQueue *q)
{
    return q->size == 0;
}

static int queue_full(const ReadyQueue *q)
{
    return q->size == q->capacity;
}

static int queue_contains(const ReadyQueue *q, int value)
{
    for (int j = 0; j < q->size; j++)
    {
        int position =
            (q->front + j) % q->capacity;

        if (q->data[position] == value)
            return 1;
    }

    return 0;
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

    q->front =
        (q->front + 1) % q->capacity;

    q->size--;

    return value;
}

static void queue_destroy(ReadyQueue *q)
{
    free(q->data);
    q->data = NULL;
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = 0;
}

/* ---------- Timer ---------- */

static void start_timer(int seconds)
{
    struct itimerval timer = {0};

    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
        perror("setitimer");
}

static void stop_timer(void)
{
    struct itimerval timer = {0};
    setitimer(ITIMER_REAL, &timer, NULL);
}

/* ---------- Add newly arrived processes ---------- */

static void enqueue_arrived_processes(
    Process *processes,
    int n,
    int current_time,
    int current_index,
    ReadyQueue *queue)
{
    for (int i = 0; i < n; i++)
    {
        if (i == current_index)
            continue;

        if (processes[i].state == TERMINATED)
            continue;

        if (processes[i].remaining_time <= 0)
            continue;

        if (processes[i].arrival_time > current_time)
            continue;

        if (!queue_contains(queue, i))
            queue_push(queue, i);
    }
}

/* =========================================================
   ROUND ROBIN WITH GANTT
   ========================================================= */

void round_robin_with_gantt(
    Process *processes,
    int n,
    int quantum,
    GanttChart *chart)
{
    if (processes == NULL || n <= 0 || quantum <= 0)
        return;

    int current_time = 0;
    int completed = 0;
    int total_burst_time = 0;
    int first_start_time = -1;
    int final_completion_time = 0;

    ReadyQueue queue;

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
     * Initial processes available at time 0.
     */
    enqueue_arrived_processes(
        processes,
        n,
        current_time,
        -1,
        &queue
    );

    while (completed < n)
    {
        /*
         * CPU idle until the next arrival.
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
                printf(
                    "\nCPU Idle: %d -> %d\n",
                    current_time,
                    next_arrival
                );

                if (chart != NULL)
                {
                    gantt_add(
                        chart,
                        0,
                        current_time,
                        next_arrival
                    );
                }

                current_time = next_arrival;
            }

            enqueue_arrived_processes(
                processes,
                n,
                current_time,
                -1,
                &queue
            );

            continue;
        }

        int index = queue_pop(&queue);

        if (index == -1)
            continue;

        if (processes[index].remaining_time <= 0)
            continue;

        if (processes[index].start_time == -1)
        {
            processes[index].start_time =
                current_time;

            processes[index].response_time =
                current_time -
                processes[index].arrival_time;

            if (first_start_time == -1)
                first_start_time = current_time;
        }

        processes[index].state = RUNNING;

        int slice =
            processes[index].remaining_time;

        if (slice > quantum)
            slice = quantum;

        int slice_start = current_time;

        printf(
            "\nP%d starts at time %d\n",
            processes[index].pid,
            current_time
        );

        printf(
            "Starting timer for %d second(s)...\n",
            slice
        );

        quantum_expired = 0;

        start_timer(slice);

        while (!quantum_expired)
            pause();

        stop_timer();

        printf(
            "SIGALRM received: time slice expired.\n"
        );

        current_time += slice;

        processes[index].remaining_time -= slice;

        printf(
            "P%d executed: %d -> %d\n",
            processes[index].pid,
            slice_start,
            current_time
        );

        /*
         * Every Round Robin time slice becomes
         * one Gantt entry.
         */
        if (chart != NULL)
        {
            gantt_add(
                chart,
                processes[index].pid,
                slice_start,
                current_time
            );
        }

        /*
         * Add processes that arrived during the slice.
         */
        enqueue_arrived_processes(
            processes,
            n,
            current_time,
            index,
            &queue
        );

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

            printf(
                "P%d TERMINATED.\n",
                processes[index].pid
            );
        }
        else
        {
            processes[index].state = READY;

            /*
             * Current process goes to the back of
             * the ready queue after its quantum.
             */
            if (!queue_contains(&queue, index))
                queue_push(&queue, index);

            printf(
                "P%d moved back to READY queue.\n",
                processes[index].pid
            );
        }
    }

    if (first_start_time >= 0 &&
        final_completion_time > first_start_time)
    {
        double cpu_utilization =
            ((double)total_burst_time /
             (final_completion_time -
              first_start_time)) * 100.0;

        printf(
            "\nCPU Utilization: %.2f%%\n",
            cpu_utilization
        );
    }

    stop_timer();
    queue_destroy(&queue);

    printf(
        "\nRound Robin scheduling completed successfully.\n"
    );
}

/* =========================================================
   ORIGINAL CLI INTERFACE
   ========================================================= */

void round_robin(
    Process *processes,
    int n,
    int quantum)
{
    GanttChart chart;

    gantt_init(&chart, n + 2);

    round_robin_with_gantt(
        processes,
        n,
        quantum,
        &chart
    );

    gantt_display(&chart);
    gantt_free(&chart);
}
