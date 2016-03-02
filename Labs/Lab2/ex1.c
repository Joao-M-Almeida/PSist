#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main() {

    int myid;


    for(int i = 0; i<10;i++){
        myid = fork();
        if(myid == 0){
            myid = getpid();
            srandom(time(NULL) * myid);
            long sleep_time = random();
            sleep_time = ((float) sleep_time)/RAND_MAX * 10;
            printf("I am child PID: %d and I'm goint to  sleep for %ld seconds\n", myid, sleep_time);
            sleep(sleep_time);
            printf("I am child PID: %d and was assleep for %ld seconds\n", myid, sleep_time);
            exit(0);
        }
    }

    printf("\nParent dying\n");

    exit(0);
}
