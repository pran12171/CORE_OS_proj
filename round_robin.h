#ifndef ROUND_ROBIN_H
#define ROUND_ROBIN_H

#include "process.h"
#include "gantt.h"

void round_robin(Process *processes, int n, int quantum);

/*
 * GUI-capable version.
 * Records every Round Robin CPU time slice in the supplied GanttChart.
 */
void round_robin_with_gantt(
    Process *processes,
    int n,
    int quantum,
    GanttChart *chart
);

#endif
