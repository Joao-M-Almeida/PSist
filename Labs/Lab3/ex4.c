/*
gcc -Wall -pedantic -ansi -std=gnu99 ex4.c -o ex4_gen_random.out -lrt
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>


int fd;
int * shared;
unsigned int shared_mem_size ;
int count_even = 0;
int count_odd = 0;

void error_and_die(char const error[]) {
    printf("%s\n", error );
    exit(-1);
}


void alarm_handler(int signum){
    printf("Received signal: %d\n", signum);
    printf("Stoped generating random numbers.\n\n");
    printf("Generated: \n%d odd numbers\n%d even numbers\n\nExiting...\n\n", count_even, count_odd);

    close(fd);
    munmap(shared, shared_mem_size);
    exit(0);
}


int main() {

    int shared_mem_size = sizeof(int);
    srandom(getpid());


    fd = shm_open("/rand_nums", O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1)
        error_and_die("shm_open");
    int r = ftruncate(fd, shared_mem_size);
    if (r != 0)
        error_and_die("ftruncate");
    shared = (int *) mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct sigaction action;
    action.sa_handler = alarm_handler;
    sigaction(SIGALRM, &action, NULL);
    alarm(10);

    int num;


    while(1){
        num = random();
        (*shared) = num;
        if (num%2==0)
            count_even++;
        else
            count_odd++;

    }
}
