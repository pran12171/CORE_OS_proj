#ifndef GANTT_H
#define GANTT_H

typedef struct
{
    int pid;
    int start_time;
    int end_time;
} GanttEntry;

typedef struct
{
    GanttEntry *entries;
    int count;
    int capacity;
} GanttChart;

void gantt_init(GanttChart *chart, int capacity);

void gantt_add(
    GanttChart *chart,
    int pid,
    int start_time,
    int end_time
);

void gantt_display(const GanttChart *chart);

void gantt_free(GanttChart *chart);

#endif
