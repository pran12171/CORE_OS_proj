#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "process_manager.h"

void demonstrate_process_creation(Process *process)
{
    pid_t pid;

    printf("\n========================================\n");
    printf("      PROCESS CREATION DEMONSTRATION\n");
    printf("========================================\n");

    printf("\nParent process PID: %d\n", getpid());

    printf("Creating simulated process P%d using fork()...\n",
           process->pid);

    pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        /*
         * Child process
         */
        printf("\n[CHILD]\n");
        printf("Child PID: %d\n", getpid());
        printf("Parent PID: %d\n", getppid());

        printf("Simulating execution of P%d...\n",
               process->pid);

        process->state = RUNNING;

        sleep(1);

        process->state = TERMINATED;

        printf("P%d execution completed.\n",
               process->pid);

        /*
         * Child exits.
         */
        exit(0);
    }
    else
    {
        /*
         * Parent process
         */
        printf("\n[PARENT]\n");
        printf("Parent created child with PID: %d\n",
               pid);

        printf("Parent waiting for child...\n");

        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return;
        }

        if (WIFEXITED(status))
        {
            printf("Child process terminated normally.\n");
            printf("Child exit status: %d\n",
                   WEXITSTATUS(status));
        }

        printf("Parent resumes execution.\n");
    }

    printf("\nProcess creation demonstration completed.\n");
}
