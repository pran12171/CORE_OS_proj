#include <stdio.h>
#include <stdlib.h>

#include "gantt.h"

void gantt_init(GanttChart *chart, int capacity)
{
    chart->entries = malloc(capacity * sizeof(GanttEntry));

    if (chart->entries == NULL)
    {
        printf("Gantt chart memory allocation failed.\n");

        chart->count = 0;
        chart->capacity = 0;

        return;
    }

    chart->count = 0;
    chart->capacity = capacity;
}

void gantt_add(
    GanttChart *chart,
    int pid,
    int start_time,
    int end_time)
{
    /*
     * Expand the array if necessary.
     */
    if (chart->count >= chart->capacity)
    {
        int new_capacity;

        if (chart->capacity == 0)
        {
            new_capacity = 10;
        }
        else
        {
            new_capacity = chart->capacity * 2;
        }

        GanttEntry *temp =
            realloc(
                chart->entries,
                new_capacity * sizeof(GanttEntry)
            );

        if (temp == NULL)
        {
            printf("Unable to expand Gantt chart.\n");
            return;
        }

        chart->entries = temp;
        chart->capacity = new_capacity;
    }

    chart->entries[chart->count].pid = pid;
    chart->entries[chart->count].start_time = start_time;
    chart->entries[chart->count].end_time = end_time;

    chart->count++;
}

void gantt_display(const GanttChart *chart)
{
    if (chart == NULL || chart->count == 0)
    {
        printf("\nNo Gantt chart data available.\n");
        return;
    }

    printf("\n");
    printf("============================================================\n");
    printf("                      GANTT CHART\n");
    printf("============================================================\n\n");

    /*
     * Top border.
     */
    printf(" ");

    for (int i = 0; i < chart->count; i++)
    {
        printf("--------");
    }

    printf("\n|");

    /*
     * Process names.
     * PID 0 = CPU IDLE.
     */
    for (int i = 0; i < chart->count; i++)
    {
        if (chart->entries[i].pid == 0)
        {
            printf(" IDLE   |");
        }
        else
        {
            printf(" P%-5d |",
                   chart->entries[i].pid);
        }
    }

    printf("\n ");

    /*
     * Bottom border.
     */
    for (int i = 0; i < chart->count; i++)
    {
        printf("--------");
    }

    printf("\n");

    /*
     * Time values.
     */
    printf("%d",
           chart->entries[0].start_time);

    for (int i = 0; i < chart->count; i++)
    {
        printf("\t%d",
               chart->entries[i].end_time);
    }

    printf("\n");
}

void gantt_free(GanttChart *chart)
{
    if (chart != NULL)
    {
        free(chart->entries);

        chart->entries = NULL;
        chart->count = 0;
        chart->capacity = 0;
    }
}
