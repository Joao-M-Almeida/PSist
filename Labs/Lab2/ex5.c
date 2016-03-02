#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main() {

    int counter = 0;
    long start_time = time(NULL);
    int sleep_time = ((float) random())/RAND_MAX * 10;
    long end_time = start_time + sleep_time;
    printf("Reseting in: %d seconds\n", sleep_time);

    while(1){

    if(time(NULL)>= end_time){
        printf("Reseting Counter\n");
        counter = 0;
        start_time =  time(NULL);
        sleep_time = ((float) random())/RAND_MAX * 10;
        end_time = start_time + sleep_time;
        printf("Reseting again in: %d seconds\n", sleep_time);

    }


    counter++;
    printf("Counter is: %d\n", counter);
    sleep(1);
    }

    return 0;
}
