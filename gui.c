#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"
#include "fcfs.h"
#include "sjf.h"
#include "priority.h"
#include "round_robin.h"
#include "gantt.h"

#define MAX_PROCESSES 50

typedef struct
{
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
} GUIProcess;

typedef struct
{
    GUIProcess processes[MAX_PROCESSES];
    int count;

    GtkWidget *pid_entry;
    GtkWidget *arrival_entry;
    GtkWidget *burst_entry;
    GtkWidget *priority_entry;
    GtkWidget *quantum_entry;

    GtkWidget *algorithm_combo;
    GtkWidget *process_list;
    GtkWidget *result_text;

    GtkWidget *priority_label;
    GtkWidget *quantum_label;
    GtkWidget *gantt_area;

    Process *scheduled_processes;
    int scheduled_count;

    GanttChart gantt;
    int gantt_valid;

} AppData;

/* =========================================================
   COMMON HELPERS
   ========================================================= */

static GtkWidget *create_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_set_halign(
        label,
        GTK_ALIGN_START
    );

    return label;
}

static void show_message(
    GtkWindow *parent,
    GtkMessageType type,
    const char *message)
{
    GtkWidget *dialog =
        gtk_message_dialog_new(
            parent,
            GTK_DIALOG_MODAL,
            type,
            GTK_BUTTONS_OK,
            "%s",
            message
        );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* =========================================================
   CLEAR GANTT
   ========================================================= */

static void clear_gantt(AppData *data)
{
    if (data->gantt_valid)
    {
        gantt_free(&data->gantt);
        data->gantt_valid = 0;
    }
}

/* =========================================================
   SAVE SCHEDULED PROCESSES
   ========================================================= */

static void save_scheduled_processes(
    AppData *data,
    Process *processes)
{
    if (data->scheduled_processes != NULL)
    {
        free_processes(
            data->scheduled_processes
        );
    }

    data->scheduled_processes = processes;
    data->scheduled_count = data->count;
}

/* =========================================================
   BUILD GANTT FOR FCFS / SJF / PRIORITY
   ========================================================= */

static void build_nonpreemptive_gantt(AppData *data)
{
    /*
     * IMPORTANT:
     * clear_gantt expects AppData*.
     */
    clear_gantt(data);

    gantt_init(
        &data->gantt,
        data->count + 1
    );

    if (data->gantt.entries == NULL)
        return;

    int order[MAX_PROCESSES];

    for (int i = 0;
         i < data->scheduled_count;
         i++)
    {
        order[i] = i;
    }

    /*
     * Sort processes according to their
     * actual CPU start time.
     */
    for (int i = 0;
         i < data->scheduled_count - 1;
         i++)
    {
        for (int j = i + 1;
             j < data->scheduled_count;
             j++)
        {
            if (
                data->scheduled_processes[order[j]].start_time <
                data->scheduled_processes[order[i]].start_time
            )
            {
                int temp = order[i];

                order[i] = order[j];
                order[j] = temp;
            }
        }
    }

    int current_time = 0;

    for (int k = 0;
         k < data->scheduled_count;
         k++)
    {
        Process *p =
            &data->scheduled_processes[order[k]];

        /*
         * CPU idle period.
         */
        if (p->start_time > current_time)
        {
            gantt_add(
                &data->gantt,
                0,
                current_time,
                p->start_time
            );
        }

        /*
         * Process execution.
         */
        if (p->completion_time > p->start_time)
        {
            gantt_add(
                &data->gantt,
                p->pid,
                p->start_time,
                p->completion_time
            );

            current_time =
                p->completion_time;
        }
    }

    data->gantt_valid = 1;
}

/* =========================================================
   DRAW GANTT CHART
   ========================================================= */

static gboolean draw_gantt_chart(
    GtkWidget *widget,
    cairo_t *cr,
    gpointer user_data)
{
    AppData *data =
        (AppData *)user_data;

    GtkAllocation allocation;

    gtk_widget_get_allocation(
        widget,
        &allocation
    );

    double width =
        allocation.width;

    double height =
        allocation.height;

    /*
     * White background.
     */
    cairo_set_source_rgb(
        cr,
        1.0,
        1.0,
        1.0
    );

    cairo_paint(cr);

    /*
     * No chart available.
     */
    if (
        !data->gantt_valid ||
        data->gantt.count == 0
    )
    {
        cairo_set_source_rgb(
            cr,
            0.30,
            0.30,
            0.30
        );

        cairo_set_font_size(
            cr,
            16
        );

        cairo_move_to(
            cr,
            30,
            45
        );

        cairo_show_text(
            cr,
            "Run a scheduling algorithm to display the Gantt Chart"
        );

        return FALSE;
    }

    int start_time =
        data->gantt.entries[0].start_time;

    int end_time =
        data->gantt.entries[
            data->gantt.count - 1
        ].end_time;

    if (end_time <= start_time)
        return FALSE;

    int total_time =
        end_time - start_time;

    double left = 50.0;
    double right = 30.0;

    double chart_width =
        width - left - right;

    if (chart_width < 100.0)
        chart_width = 100.0;

    double y = 35.0;
    double chart_height = 70.0;

    if (height < 130.0)
        chart_height = 55.0;

    /*
     * Draw every Gantt entry.
     */
    for (
        int i = 0;
        i < data->gantt.count;
        i++
    )
    {
        GanttEntry *entry =
            &data->gantt.entries[i];

        double x =
            left +
            (
                (double)(
                    entry->start_time -
                    start_time
                ) /
                total_time
            ) *
            chart_width;

        double block_width =
            (
                (double)(
                    entry->end_time -
                    entry->start_time
                ) /
                total_time
            ) *
            chart_width;

        if (block_width < 2.0)
            block_width = 2.0;

        /*
         * PID 0 = IDLE.
         */
        if (entry->pid == 0)
        {
            cairo_set_source_rgb(
                cr,
                0.65,
                0.65,
                0.65
            );
        }
        else
        {
            cairo_set_source_rgb(
                cr,
                0.20,
                0.50,
                0.85
            );
        }

        cairo_rectangle(
            cr,
            x,
            y,
            block_width,
            chart_height
        );

        cairo_fill_preserve(cr);

        /*
         * Border.
         */
        cairo_set_source_rgb(
            cr,
            0.05,
            0.15,
            0.25
        );

        cairo_set_line_width(
            cr,
            2.0
        );

        cairo_stroke(cr);

        /*
         * Process label.
         */
        char label[32];

        if (entry->pid == 0)
        {
            snprintf(
                label,
                sizeof(label),
                "IDLE"
            );
        }
        else
        {
            snprintf(
                label,
                sizeof(label),
                "P%d",
                entry->pid
            );
        }

        cairo_set_source_rgb(
            cr,
            1.0,
            1.0,
            1.0
        );

        cairo_set_font_size(
            cr,
            15
        );

        cairo_text_extents_t extents;

        cairo_text_extents(
            cr,
            label,
            &extents
        );

        if (
            block_width >
            extents.width + 8
        )
        {
            cairo_move_to(
                cr,
                x +
                (
                    block_width -
                    extents.width
                ) / 2.0,

                y +
                (
                    chart_height +
                    extents.height
                ) / 2.0
            );

            cairo_show_text(
                cr,
                label
            );
        }

        /*
         * Start time.
         */
        char time_label[32];

        snprintf(
            time_label,
            sizeof(time_label),
            "%d",
            entry->start_time
        );

        cairo_set_source_rgb(
            cr,
            0.10,
            0.10,
            0.10
        );

        cairo_set_font_size(
            cr,
            13
        );

        cairo_move_to(
            cr,
            x - 5,
            y + chart_height + 24
        );

        cairo_show_text(
            cr,
            time_label
        );

        /*
         * Final time.
         */
        if (
            i ==
            data->gantt.count - 1
        )
        {
            snprintf(
                time_label,
                sizeof(time_label),
                "%d",
                entry->end_time
            );

            cairo_move_to(
                cr,
                x + block_width - 5,
                y + chart_height + 24
            );

            cairo_show_text(
                cr,
                time_label
            );
        }
    }

    return FALSE;
}

/* =========================================================
   SHOW / HIDE ALGORITHM FIELDS
   ========================================================= */

static void update_algorithm_fields(
    AppData *data)
{
    gchar *algorithm =
        gtk_combo_box_text_get_active_text(
            GTK_COMBO_BOX_TEXT(
                data->algorithm_combo
            )
        );

    if (algorithm == NULL)
        return;

    /*
     * Priority field.
     */
    if (
        strcmp(
            algorithm,
            "Priority"
        ) == 0
    )
    {
        gtk_widget_show(
            data->priority_label
        );

        gtk_widget_show(
            data->priority_entry
        );
    }
    else
    {
        gtk_widget_hide(
            data->priority_label
        );

        gtk_widget_hide(
            data->priority_entry
        );
    }

    /*
     * Round Robin quantum.
     */
    if (
        strcmp(
            algorithm,
            "Round Robin"
        ) == 0
    )
    {
        gtk_widget_show(
            data->quantum_label
        );

        gtk_widget_show(
            data->quantum_entry
        );
    }
    else
    {
        gtk_widget_hide(
            data->quantum_label
        );

        gtk_widget_hide(
            data->quantum_entry
        );
    }

    g_free(algorithm);
}

/* =========================================================
   PROCESS LIST
   ========================================================= */

static void refresh_process_list(
    AppData *data)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->process_list
            )
        );

    char text[8192];

    int offset = 0;

    offset += snprintf(
        text + offset,
        sizeof(text) - offset,

        "PROCESS LIST\n\n"

        "----------------------------------------------------------\n"

        "Process\tArrival Time\tBurst Time\tPriority\n"

        "----------------------------------------------------------\n"
    );

    for (
        int i = 0;
        i < data->count;
        i++
    )
    {
        offset += snprintf(
            text + offset,
            sizeof(text) - offset,

            "P%d\t%d\t\t%d\t\t%d\n",

            data->processes[i].pid,

            data->processes[i].arrival_time,

            data->processes[i].burst_time,

            data->processes[i].priority
        );
    }

    if (data->count == 0)
    {
        offset += snprintf(
            text + offset,
            sizeof(text) - offset,

            "\nNo processes added yet."
        );
    }

    gtk_text_buffer_set_text(
        buffer,
        text,
        -1
    );
}

/* =========================================================
   ADD PROCESS
   ========================================================= */

static void add_process(
    GtkButton *button,
    gpointer user_data)
{
    AppData *data =
        (AppData *)user_data;

    (void)button;

    if (
        data->count >=
        MAX_PROCESSES
    )
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    GTK_WIDGET(button)
                )
            ),
            GTK_MESSAGE_ERROR,
            "Maximum of 50 processes reached."
        );

        return;
    }

    const char *arrival_text =
        gtk_entry_get_text(
            GTK_ENTRY(
                data->arrival_entry
            )
        );

    const char *burst_text =
        gtk_entry_get_text(
            GTK_ENTRY(
                data->burst_entry
            )
        );

    gchar *algorithm =
        gtk_combo_box_text_get_active_text(
            GTK_COMBO_BOX_TEXT(
                data->algorithm_combo
            )
        );

    if (
        strlen(arrival_text) == 0 ||
        strlen(burst_text) == 0
    )
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    GTK_WIDGET(button)
                )
            ),
            GTK_MESSAGE_WARNING,
            "Please enter Arrival Time and Burst Time."
        );

        g_free(algorithm);

        return;
    }

    int arrival =
        atoi(arrival_text);

    int burst =
        atoi(burst_text);

    int priority = 0;

    if (
        arrival < 0 ||
        burst <= 0
    )
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    GTK_WIDGET(button)
                )
            ),
            GTK_MESSAGE_WARNING,
            "Arrival Time must be >= 0\n"
            "Burst Time must be > 0."
        );

        g_free(algorithm);

        return;
    }

    /*
     * Priority is required only
     * for Priority Scheduling.
     */
    if (
        algorithm != NULL &&
        strcmp(
            algorithm,
            "Priority"
        ) == 0
    )
    {
        const char *priority_text =
            gtk_entry_get_text(
                GTK_ENTRY(
                    data->priority_entry
                )
            );

        if (
            strlen(priority_text) == 0
        )
        {
            show_message(
                GTK_WINDOW(
                    gtk_widget_get_toplevel(
                        GTK_WIDGET(button)
                    )
                ),
                GTK_MESSAGE_WARNING,
                "Please enter Priority."
            );

            g_free(algorithm);

            return;
        }

        priority =
            atoi(priority_text);

        if (priority <= 0)
        {
            show_message(
                GTK_WINDOW(
                    gtk_widget_get_toplevel(
                        GTK_WIDGET(button)
                    )
                ),
                GTK_MESSAGE_WARNING,
                "Priority must be greater than 0."
            );

            g_free(algorithm);

            return;
        }
    }

    g_free(algorithm);

    /*
     * Store process.
     */
    data->processes[data->count].pid =
        data->count + 1;

    data->processes[data->count].arrival_time =
        arrival;

    data->processes[data->count].burst_time =
        burst;

    data->processes[data->count].priority =
        priority;

    data->count++;

    refresh_process_list(data);

    /*
     * Next PID.
     */
    char next_pid[20];

    snprintf(
        next_pid,
        sizeof(next_pid),
        "P%d",
        data->count + 1
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->pid_entry
        ),
        next_pid
    );

    /*
     * Clear input fields.
     */
    gtk_entry_set_text(
        GTK_ENTRY(
            data->arrival_entry
        ),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->burst_entry
        ),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->priority_entry
        ),
        ""
    );

    /*
     * New input means old result/Gantt
     * is no longer current.
     */
    clear_gantt(data);

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->result_text
            )
        ),

        "RESULTS\n\n"
        "Process list updated.\n"
        "Click RUN SIMULATION.",

        -1
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   CLEAR PROCESSES
   ========================================================= */

static void clear_processes(
    GtkButton *button,
    gpointer user_data)
{
    AppData *data =
        (AppData *)user_data;

    (void)button;

    data->count = 0;

    if (
        data->scheduled_processes != NULL
    )
    {
        free_processes(
            data->scheduled_processes
        );

        data->scheduled_processes =
            NULL;
    }

    data->scheduled_count = 0;

    clear_gantt(data);

    gtk_entry_set_text(
        GTK_ENTRY(
            data->pid_entry
        ),
        "P1"
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->arrival_entry
        ),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->burst_entry
        ),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->priority_entry
        ),
        ""
    );

    gtk_entry_set_text(
        GTK_ENTRY(
            data->quantum_entry
        ),
        ""
    );

    refresh_process_list(data);

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->result_text
            )
        ),

        "RESULTS\n\n"
        "Add processes and select an algorithm.",

        -1
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   CREATE SCHEDULER PROCESSES
   ========================================================= */

static Process *create_scheduler_processes(
    AppData *data)
{
    Process *processes =
        create_processes(
            data->count
        );

    if (processes == NULL)
        return NULL;

    for (
        int i = 0;
        i < data->count;
        i++
    )
    {
        initialize_process(
            &processes[i],

            data->processes[i].pid,

            data->processes[i].arrival_time,

            data->processes[i].burst_time,

            data->processes[i].priority
        );
    }

    return processes;
}

/* =========================================================
   DISPLAY STANDARD RESULTS
   ========================================================= */

static void display_standard_results(
    AppData *data,
    const char *title,
    int show_priority)
{
    char results[12000];

    int offset = 0;

    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    if (show_priority)
    {
        offset += snprintf(
            results + offset,
            sizeof(results) - offset,

            "%s\n\n"

            "Lower priority number = Higher priority\n\n"

            "--------------------------------------------------------------------------\n"

            "Process\tAT\tBT\tPriority\tCT\tWT\tTAT\tRT\n"

            "--------------------------------------------------------------------------\n",

            title
        );
    }
    else
    {
        offset += snprintf(
            results + offset,
            sizeof(results) - offset,

            "%s\n\n"

            "--------------------------------------------------------------\n"

            "Process\tAT\tBT\tCT\tWT\tTAT\tRT\n"

            "--------------------------------------------------------------\n",

            title
        );
    }

    for (
        int i = 0;
        i < data->scheduled_count;
        i++
    )
    {
        Process *p =
            &data->scheduled_processes[i];

        if (show_priority)
        {
            offset += snprintf(
                results + offset,
                sizeof(results) - offset,

                "P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d\n",

                p->pid,

                p->arrival_time,

                p->burst_time,

                p->priority,

                p->completion_time,

                p->waiting_time,

                p->turnaround_time,

                p->response_time
            );
        }
        else
        {
            offset += snprintf(
                results + offset,
                sizeof(results) - offset,

                "P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",

                p->pid,

                p->arrival_time,

                p->burst_time,

                p->completion_time,

                p->waiting_time,

                p->turnaround_time,

                p->response_time
            );
        }

        total_waiting +=
            p->waiting_time;

        total_turnaround +=
            p->turnaround_time;

        total_response +=
            p->response_time;
    }

    if (
        data->scheduled_count > 0
    )
    {
        offset += snprintf(
            results + offset,
            sizeof(results) - offset,

            "\n--------------------------------------------------------------\n"

            "Average Waiting Time    : %.2f\n"

            "Average Turnaround Time : %.2f\n"

            "Average Response Time   : %.2f\n"

            "--------------------------------------------------------------\n",

            total_waiting /
            data->scheduled_count,

            total_turnaround /
            data->scheduled_count,

            total_response /
            data->scheduled_count
        );
    }

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->result_text
            )
        ),

        results,

        -1
    );
}

/* =========================================================
   RUN FCFS
   ========================================================= */

static void run_fcfs(AppData *data)
{
    Process *processes =
        create_scheduler_processes(
            data
        );

    if (processes == NULL)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_ERROR,
            "Unable to allocate process memory."
        );

        return;
    }

    fcfs(
        processes,
        data->count
    );

    save_scheduled_processes(
        data,
        processes
    );

    build_nonpreemptive_gantt(
        data
    );

    display_standard_results(
        data,
        "FCFS SCHEDULING RESULTS",
        0
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   RUN SJF
   ========================================================= */

static void run_sjf(AppData *data)
{
    Process *processes =
        create_scheduler_processes(
            data
        );

    if (processes == NULL)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_ERROR,
            "Unable to allocate process memory."
        );

        return;
    }

    sjf(
        processes,
        data->count
    );

    save_scheduled_processes(
        data,
        processes
    );

    build_nonpreemptive_gantt(
        data
    );

    display_standard_results(
        data,
        "SJF SCHEDULING RESULTS",
        0
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   RUN PRIORITY
   ========================================================= */

static void run_priority(AppData *data)
{
    Process *processes =
        create_scheduler_processes(
            data
        );

    if (processes == NULL)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_ERROR,
            "Unable to allocate process memory."
        );

        return;
    }

    priority_scheduling(
        processes,
        data->count
    );

    save_scheduled_processes(
        data,
        processes
    );

    build_nonpreemptive_gantt(
        data
    );

    display_standard_results(
        data,
        "PRIORITY SCHEDULING RESULTS",
        1
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   RUN ROUND ROBIN
   ========================================================= */

static void run_round_robin(
    AppData *data)
{
    const char *quantum_text =
        gtk_entry_get_text(
            GTK_ENTRY(
                data->quantum_entry
            )
        );

    if (
        strlen(quantum_text) == 0
    )
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_WARNING,
            "Please enter Time Quantum."
        );

        return;
    }

    int quantum =
        atoi(quantum_text);

    if (quantum <= 0)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_WARNING,
            "Time Quantum must be greater than 0."
        );

        return;
    }

    Process *processes =
        create_scheduler_processes(
            data
        );

    if (processes == NULL)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_ERROR,
            "Unable to allocate process memory."
        );

        return;
    }

    /*
     * RR creates one Gantt entry
     * for every CPU time slice.
     */
    clear_gantt(data);

    gantt_init(
        &data->gantt,
        data->count + 2
    );

    if (
        data->gantt.entries == NULL
    )
    {
        free_processes(processes);

        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    data->gantt_area
                )
            ),
            GTK_MESSAGE_ERROR,
            "Unable to allocate Gantt chart memory."
        );

        return;
    }

    round_robin_with_gantt(
        processes,
        data->count,
        quantum,
        &data->gantt
    );

    data->gantt_valid = 1;

    save_scheduled_processes(
        data,
        processes
    );

    char results[12000];

    int offset = 0;

    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    offset += snprintf(
        results + offset,
        sizeof(results) - offset,

        "ROUND ROBIN SCHEDULING RESULTS\n\n"

        "Time Quantum: %d\n\n"

        "--------------------------------------------------------------\n"

        "Process\tAT\tBT\tCT\tWT\tTAT\tRT\n"

        "--------------------------------------------------------------\n",

        quantum
    );

    for (
        int i = 0;
        i < data->scheduled_count;
        i++
    )
    {
        Process *p =
            &data->scheduled_processes[i];

        offset += snprintf(
            results + offset,
            sizeof(results) - offset,

            "P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",

            p->pid,

            p->arrival_time,

            p->burst_time,

            p->completion_time,

            p->waiting_time,

            p->turnaround_time,

            p->response_time
        );

        total_waiting +=
            p->waiting_time;

        total_turnaround +=
            p->turnaround_time;

        total_response +=
            p->response_time;
    }

    offset += snprintf(
        results + offset,
        sizeof(results) - offset,

        "\n--------------------------------------------------------------\n"

        "Average Waiting Time    : %.2f\n"

        "Average Turnaround Time : %.2f\n"

        "Average Response Time   : %.2f\n"

        "--------------------------------------------------------------\n",

        total_waiting /
        data->scheduled_count,

        total_turnaround /
        data->scheduled_count,

        total_response /
        data->scheduled_count
    );

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->result_text
            )
        ),

        results,

        -1
    );

    gtk_widget_queue_draw(
        data->gantt_area
    );
}

/* =========================================================
   RUN SIMULATION
   ========================================================= */

static void run_simulation(
    GtkButton *button,
    gpointer user_data)
{
    AppData *data =
        (AppData *)user_data;

    (void)button;

    if (data->count == 0)
    {
        show_message(
            GTK_WINDOW(
                gtk_widget_get_toplevel(
                    GTK_WIDGET(button)
                )
            ),
            GTK_MESSAGE_WARNING,
            "Please add at least one process."
        );

        return;
    }

    gchar *algorithm =
        gtk_combo_box_text_get_active_text(
            GTK_COMBO_BOX_TEXT(
                data->algorithm_combo
            )
        );

    if (algorithm == NULL)
        return;

    if (
        strcmp(
            algorithm,
            "FCFS"
        ) == 0
    )
    {
        run_fcfs(data);
    }
    else if (
        strcmp(
            algorithm,
            "SJF"
        ) == 0
    )
    {
        run_sjf(data);
    }
    else if (
        strcmp(
            algorithm,
            "Priority"
        ) == 0
    )
    {
        run_priority(data);
    }
    else if (
        strcmp(
            algorithm,
            "Round Robin"
        ) == 0
    )
    {
        run_round_robin(data);
    }

    g_free(algorithm);
}

/* =========================================================
   ALGORITHM CHANGED
   ========================================================= */

static void algorithm_changed(
    GtkComboBox *combo,
    gpointer user_data)
{
    AppData *data =
        (AppData *)user_data;

    (void)combo;

    update_algorithm_fields(data);
}

/* =========================================================
   ACTIVATE GUI
   ========================================================= */

static void activate(
    GtkApplication *app,
    gpointer user_data)
{
    (void)user_data;

    GtkWidget *window;

    GtkWidget *main_box;
    GtkWidget *header_box;
    GtkWidget *content_box;

    GtkWidget *input_frame;
    GtkWidget *control_frame;
    GtkWidget *process_frame;
    GtkWidget *result_frame;
    GtkWidget *gantt_frame;

    GtkWidget *input_grid;
    GtkWidget *control_grid;

    GtkWidget *title;
    GtkWidget *subtitle;

    GtkWidget *add_button;
    GtkWidget *clear_button;
    GtkWidget *run_button;

    AppData *data =
        g_malloc0(
            sizeof(AppData)
        );

    /* =====================================================
       WINDOW
       ===================================================== */

    window =
        gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "CPU Scheduling Simulation System"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        1150,
        900
    );

    gtk_window_set_position(
        GTK_WINDOW(window),
        GTK_WIN_POS_CENTER
    );

    /* =====================================================
       MAIN BOX
       ===================================================== */

    main_box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            10
        );

    gtk_widget_set_margin_top(
        main_box,
        15
    );

    gtk_widget_set_margin_bottom(
        main_box,
        15
    );

    gtk_widget_set_margin_start(
        main_box,
        20
    );

    gtk_widget_set_margin_end(
        main_box,
        20
    );

    /* =====================================================
       HEADER
       ===================================================== */

    header_box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            5
        );

    title =
        gtk_label_new(
            "CPU SCHEDULING SIMULATION SYSTEM"
        );

    subtitle =
        gtk_label_new(
            "CORE_OSSP | Operating Systems & Systems Programming"
        );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_widget_set_halign(
        subtitle,
        GTK_ALIGN_CENTER
    );

    gtk_box_pack_start(
        GTK_BOX(header_box),
        title,
        FALSE,
        FALSE,
        5
    );

    gtk_box_pack_start(
        GTK_BOX(header_box),
        subtitle,
        FALSE,
        FALSE,
        5
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        header_box,
        FALSE,
        FALSE,
        5
    );

    /* =====================================================
       INPUT + CONTROL
       ===================================================== */

    content_box =
        gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL,
            15
        );

    /* =====================================================
       PROCESS INPUT
       ===================================================== */

    input_frame =
        gtk_frame_new(
            "PROCESS INPUT"
        );

    input_grid =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(input_grid),
        10
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(input_grid),
        10
    );

    gtk_widget_set_margin_top(
        input_grid,
        15
    );

    gtk_widget_set_margin_bottom(
        input_grid,
        15
    );

    gtk_widget_set_margin_start(
        input_grid,
        15
    );

    gtk_widget_set_margin_end(
        input_grid,
        15
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        create_label("Process ID"),
        0, 0, 1, 1
    );

    data->pid_entry =
        gtk_entry_new();

    gtk_entry_set_text(
        GTK_ENTRY(
            data->pid_entry
        ),
        "P1"
    );

    gtk_widget_set_sensitive(
        data->pid_entry,
        FALSE
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        data->pid_entry,
        1, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        create_label("Arrival Time"),
        0, 1, 1, 1
    );

    data->arrival_entry =
        gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(
            data->arrival_entry
        ),
        "0"
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        data->arrival_entry,
        1, 1, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        create_label("Burst Time"),
        0, 2, 1, 1
    );

    data->burst_entry =
        gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(
            data->burst_entry
        ),
        "5"
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        data->burst_entry,
        1, 2, 1, 1
    );

    data->priority_label =
        create_label("Priority");

    gtk_grid_attach(
        GTK_GRID(input_grid),
        data->priority_label,
        0, 3, 1, 1
    );

    data->priority_entry =
        gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(
            data->priority_entry
        ),
        "1"
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        data->priority_entry,
        1, 3, 1, 1
    );

    add_button =
        gtk_button_new_with_label(
            "ADD PROCESS"
        );

    clear_button =
        gtk_button_new_with_label(
            "CLEAR"
        );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        add_button,
        0, 4, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        clear_button,
        1, 4, 1, 1
    );

    gtk_container_add(
        GTK_CONTAINER(input_frame),
        input_grid
    );

    gtk_box_pack_start(
        GTK_BOX(content_box),
        input_frame,
        TRUE,
        TRUE,
        0
    );

    /* =====================================================
       SCHEDULING CONTROL
       ===================================================== */

    control_frame =
        gtk_frame_new(
            "SCHEDULING CONTROL"
        );

    control_grid =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(control_grid),
        12
    );

    gtk_grid_set_column_spacing(
        GTK_GRID(control_grid),
        10
    );

    gtk_widget_set_margin_top(
        control_grid,
        15
    );

    gtk_widget_set_margin_bottom(
        control_grid,
        15
    );

    gtk_widget_set_margin_start(
        control_grid,
        15
    );

    gtk_widget_set_margin_end(
        control_grid,
        15
    );

    gtk_grid_attach(
        GTK_GRID(control_grid),
        create_label("Algorithm"),
        0, 0, 1, 1
    );

    data->algorithm_combo =
        gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(
            data->algorithm_combo
        ),
        "FCFS"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(
            data->algorithm_combo
        ),
        "SJF"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(
            data->algorithm_combo
        ),
        "Priority"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(
            data->algorithm_combo
        ),
        "Round Robin"
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(
            data->algorithm_combo
        ),
        0
    );

    gtk_grid_attach(
        GTK_GRID(control_grid),
        data->algorithm_combo,
        1, 0, 1, 1
    );

    data->quantum_label =
        create_label(
            "Time Quantum"
        );

    gtk_grid_attach(
        GTK_GRID(control_grid),
        data->quantum_label,
        0, 1, 1, 1
    );

    data->quantum_entry =
        gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(
            data->quantum_entry
        ),
        "For Round Robin"
    );

    gtk_grid_attach(
        GTK_GRID(control_grid),
        data->quantum_entry,
        1, 1, 1, 1
    );

    run_button =
        gtk_button_new_with_label(
            "RUN SIMULATION"
        );

    gtk_widget_set_size_request(
        run_button,
        180,
        45
    );

    gtk_grid_attach(
        GTK_GRID(control_grid),
        run_button,
        0, 2, 2, 1
    );

    gtk_container_add(
        GTK_CONTAINER(control_frame),
        control_grid
    );

    gtk_box_pack_start(
        GTK_BOX(content_box),
        control_frame,
        TRUE,
        TRUE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        content_box,
        FALSE,
        FALSE,
        5
    );

    /* =====================================================
       PROCESS LIST
       ===================================================== */

    data->process_list =
        gtk_text_view_new();

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(
            data->process_list
        ),
        FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(
            data->process_list
        ),
        FALSE
    );

    process_frame =
        gtk_frame_new(
            "PROCESSES"
        );

    gtk_container_add(
        GTK_CONTAINER(process_frame),
        data->process_list
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        process_frame,
        TRUE,
        TRUE,
        0
    );

    /* =====================================================
       RESULTS
       ===================================================== */

    data->result_text =
        gtk_text_view_new();

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(
            data->result_text
        ),
        FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(
            data->result_text
        ),
        FALSE
    );

    result_frame =
        gtk_frame_new(
            "SCHEDULING RESULTS"
        );

    gtk_container_add(
        GTK_CONTAINER(result_frame),
        data->result_text
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        result_frame,
        TRUE,
        TRUE,
        0
    );

    /* =====================================================
       GANTT CHART
       ===================================================== */

    gantt_frame =
        gtk_frame_new(
            "GANTT CHART"
        );

    data->gantt_area =
        gtk_drawing_area_new();

    gtk_widget_set_size_request(
        data->gantt_area,
        -1,
        150
    );

    gtk_container_add(
        GTK_CONTAINER(gantt_frame),
        data->gantt_area
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        gantt_frame,
        TRUE,
        TRUE,
        0
    );

    /* =====================================================
       INITIAL CONTENT
       ===================================================== */

    refresh_process_list(data);

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(
                data->result_text
            )
        ),

        "RESULTS\n\n"
        "Add processes and select an algorithm.\n"
        "Click RUN SIMULATION.",

        -1
    );

    /* =====================================================
       SIGNAL CONNECTIONS
       ===================================================== */

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(add_process),
        data
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(clear_processes),
        data
    );

    g_signal_connect(
        run_button,
        "clicked",
        G_CALLBACK(run_simulation),
        data
    );

    g_signal_connect(
        data->algorithm_combo,
        "changed",
        G_CALLBACK(algorithm_changed),
        data
    );

    g_signal_connect(
        data->gantt_area,
        "draw",
        G_CALLBACK(draw_gantt_chart),
        data
    );

    /* =====================================================
       SHOW WINDOW
       ===================================================== */

    gtk_container_add(
        GTK_CONTAINER(window),
        main_box
    );

    gtk_widget_show_all(window);

    /*
     * Hide Priority and Quantum initially.
     */
    update_algorithm_fields(data);
}

/* =========================================================
   MAIN
   ========================================================= */

int main(
    int argc,
    char **argv)
{
    GtkApplication *app;

    int status;

    app =
        gtk_application_new(
            "com.coreosspscheduler.app",
            G_APPLICATION_DEFAULT_FLAGS
        );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    status =
        g_application_run(
            G_APPLICATION(app),
            argc,
            argv
        );

    g_object_unref(app);

    return status;
}
