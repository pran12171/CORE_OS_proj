#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "fifo.h"

#define FIFO_PATH "/tmp/cpu_scheduler_fifo"

void demonstrate_fifo(Process *process)
{
    pid_t pid;

    printf("\n========================================\n");
    printf("        FIFO IPC DEMONSTRATION\n");
    printf("========================================\n");

    /*
     * Remove an old FIFO if it already exists.
     */
    unlink(FIFO_PATH);

    /*
     * Create named pipe.
     *
     * 0666 = read/write permissions.
     */
    if (mkfifo(FIFO_PATH, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            return;
        }
    }

    printf("\nFIFO created at:\n");
    printf("%s\n", FIFO_PATH);

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        unlink(FIFO_PATH);
        return;
    }

    if (pid == 0)
    {
        /*
         * CHILD = FIFO READER
         */

        printf("\n[CHILD - READER]\n");
        printf("Child PID: %d\n", getpid());

        int fd = open(FIFO_PATH, O_RDONLY);

        if (fd == -1)
        {
            perror("open FIFO for reading");
            exit(EXIT_FAILURE);
        }

        Process received_process;

        ssize_t bytes_read =
            read(fd,
                 &received_process,
                 sizeof(Process));

        if (bytes_read == -1)
        {
            perror("read");
            close(fd);
            exit(EXIT_FAILURE);
        }

        printf("\nReceived process information through FIFO:\n");

        printf("PID: %d\n",
               received_process.pid);

        printf("Arrival Time: %d\n",
               received_process.arrival_time);

        printf("Burst Time: %d\n",
               received_process.burst_time);

        printf("Priority: %d\n",
               received_process.priority);

        printf("Remaining Time: %d\n",
               received_process.remaining_time);

        close(fd);

        printf("\nFIFO reader completed.\n");

        exit(EXIT_SUCCESS);
    }
    else
    {
        /*
         * PARENT = FIFO WRITER
         */

        printf("\n[PARENT - WRITER]\n");
        printf("Parent PID: %d\n", getpid());

        /*
         * Give the child time to open FIFO.
         */
        sleep(1);

        int fd = open(FIFO_PATH, O_WRONLY);

        if (fd == -1)
        {
            perror("open FIFO for writing");
            waitpid(pid, NULL, 0);
            unlink(FIFO_PATH);
            return;
        }

        printf("Sending process P%d through FIFO...\n",
               process->pid);

        ssize_t bytes_written =
            write(fd,
                  process,
                  sizeof(Process));

        if (bytes_written == -1)
        {
            perror("write");
            close(fd);
            waitpid(pid, NULL, 0);
            unlink(FIFO_PATH);
            return;
        }

        printf("Process information sent successfully.\n");

        close(fd);

        /*
         * Wait for child.
         */
        waitpid(pid, NULL, 0);

        printf("\nChild completed reading FIFO.\n");
    }

    /*
     * Remove named pipe.
     */
    unlink(FIFO_PATH);

    printf("\nFIFO IPC demonstration completed.\n");
}
