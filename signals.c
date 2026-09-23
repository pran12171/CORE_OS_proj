#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include "signals.h"

/*
 * Global flag modified by the signal handler.
 */
volatile sig_atomic_t timer_expired = 0;

/*
 * SIGALRM signal handler.
 */
static void handle_sigalrm(int signal_number)
{
    /*
     * Avoid unused parameter warning.
     */
    (void)signal_number;

    timer_expired = 1;

    printf("\n>>> SIGALRM received: Time quantum expired! <<<\n");
}

void demonstrate_timer_signal(void)
{
    struct itimerval timer;

    printf("\n========================================\n");
    printf("       SIGNAL + TIMER DEMONSTRATION\n");
    printf("========================================\n");

    /*
     * Register SIGALRM handler.
     */
    if (signal(SIGALRM, handle_sigalrm) == SIG_ERR)
    {
        perror("signal");
        return;
    }

    /*
     * Configure timer.
     *
     * First expiration: 2 seconds
     * Interval: 0 means one-shot timer.
     */
    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    printf("\nStarting timer for 2 seconds...\n");
    printf("Waiting for SIGALRM...\n");

    /*
     * Start timer.
     */
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        return;
    }

    /*
     * Wait until signal arrives.
     */
    while (!timer_expired)
    {
        pause();
    }

    printf("\nTimer demonstration completed.\n");
}
