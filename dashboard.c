#include <stdio.h>
#include <stdlib.h>

#include "dashboard.h"

void clear_screen()
{
    system("clear");
}

void pause_screen()
{
    printf("\nPress Enter to continue...");
    getchar();
    getchar();
}

void display_section_header(const char *title)
{
    printf("\n");
    printf("============================================================\n");
    printf("                    %s\n", title);
    printf("============================================================\n");
}

void display_dashboard()
{
    clear_screen();

    printf("\n");
    printf("============================================================\n");
    printf("              CPU SCHEDULING SIMULATION SYSTEM\n");
    printf("                       CORE_OSSP\n");
    printf("============================================================\n");

    printf("\n");
    printf("  PROCESS MANAGEMENT\n");
    printf("  ----------------------------------------------------------\n");
    printf("   1. Add Processes\n");
    printf("   2. View Process Table\n");
    printf("   3. Process States\n");

    printf("\n");
    printf("  CPU SCHEDULING\n");
    printf("  ----------------------------------------------------------\n");
    printf("   4. FCFS Scheduling\n");
    printf("   5. SJF Scheduling\n");
    printf("   6. Priority Scheduling\n");
    printf("   7. Round Robin Scheduling\n");

    printf("\n");
    printf("  ANALYSIS\n");
    printf("  ----------------------------------------------------------\n");
    printf("   8. Gantt Chart\n");
    printf("   9. Scheduling Metrics\n");
    printf("  10. Compare Algorithms\n");

    printf("\n");
    printf("  OS DEMONSTRATIONS\n");
    printf("  ----------------------------------------------------------\n");
    printf("  11. Process Creation (fork/wait)\n");
    printf("  12. Pipe IPC\n");
    printf("  13. FIFO IPC\n");
    printf("  14. Signal + Timer\n");

    printf("\n");
    printf("  ----------------------------------------------------------\n");
    printf("   0. Exit\n");
    printf("============================================================\n");
}
