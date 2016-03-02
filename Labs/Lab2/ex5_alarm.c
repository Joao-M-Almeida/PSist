#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

/*struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};*/


    volatile int counter = 0;
    volatile int sleep_time;


    void alarm_handler(int signum){
        printf("Received signal: %d, Reseting Counter\n", signum);
        counter = 0;
        sleep_time = ((float) random())/RAND_MAX * 10;
        printf("Reseting again in: %d seconds\n", sleep_time);
        alarm(sleep_time);

    }

int main() {

    struct sigaction action;

    action.sa_handler = alarm_handler;

    sigaction(SIGALRM, &action, NULL);

    /* Or using signal

    signal(SIGALRM, alarm_handler);*/

    sleep_time = ((float) random())/RAND_MAX * 10;
    printf("Reseting in: %d seconds\n", sleep_time);
    alarm(sleep_time);


    while(1){
    counter++;
    printf("Counter is: %d\n", counter);
    sleep(1);
    }

    return 0;
}
