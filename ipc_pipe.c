#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "ipc_pipe.h"

void demonstrate_pipe(Process *process)
{
    int fd[2];

    /*
     * fd[0] = read end
     * fd[1] = write end
     */
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return;
    }

    printf("\n========================================\n");
    printf("       PIPE IPC DEMONSTRATION\n");
    printf("========================================\n");

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return;
    }

    if (pid == 0)
    {
        /*
         * CHILD PROCESS
         */

        close(fd[1]);

        Process received_process;

        ssize_t bytes_read =
            read(fd[0],
                 &received_process,
                 sizeof(Process));

        if (bytes_read == -1)
        {
            perror("read");
            close(fd[0]);
            exit(EXIT_FAILURE);
        }

        printf("\n[CHILD]\n");
        printf("Child PID: %d\n", getpid());

        printf("\nReceived process information through pipe:\n");

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

        close(fd[0]);

        exit(EXIT_SUCCESS);
    }
    else
    {
        /*
         * PARENT PROCESS
         */

        close(fd[0]);

        printf("\n[PARENT]\n");
        printf("Parent PID: %d\n",
               getpid());

        printf("Sending process P%d through pipe...\n",
               process->pid);

        ssize_t bytes_written =
            write(fd[1],
                  process,
                  sizeof(Process));

        if (bytes_written == -1)
        {
            perror("write");
            close(fd[1]);
            waitpid(pid, NULL, 0);
            return;
        }

        printf("Process information sent successfully.\n");

        close(fd[1]);

        /*
         * Wait for child.
         */
        waitpid(pid, NULL, 0);

        printf("\nParent received confirmation that "
               "child completed.\n");
    }

    printf("\nPipe IPC demonstration completed.\n");
}
