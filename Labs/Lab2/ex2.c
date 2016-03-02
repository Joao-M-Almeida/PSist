#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


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
            exit(sleep_time);
        }
    }

    /* Now wait for children*/
    int status;
    int id;
    int sum=0;

    for(int i = 0; i<10;i++){
        id = wait(&status);
        sum+=WEXITSTATUS(status);
        printf("Son %d exited with status: %d\n", id, WEXITSTATUS(status));
    }


    printf("\nParent dying, total sleep time: %d\n",sum);

    exit(0);
}
