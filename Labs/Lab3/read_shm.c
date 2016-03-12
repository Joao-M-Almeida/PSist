/*
gcc -Wall -pedantic -ansi -std=gnu99 read_shm.c -o ex4_read_shm.out -lrt
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


int count_even = 0;
int count_odd = 0;
unsigned int shared_mem_size = sizeof(int);
int fd;
int * shared;

void alarm_handler(int signum){
    printf("Received signal: %d\n", signum);
    printf("Stoped generating random numbers.\n\n");
    printf("Generated: \n%d odd numbers\n%d even numbers\n\nExiting...\n\n", count_even, count_odd);

    close(fd);
    munmap(shared, shared_mem_size);
    exit(0);

}

int main() {
    fd = shm_open("/rand_nums", O_CREAT | O_RDWR, 0666);
    if (fd == -1){
        printf("Unable to open\n");
        exit(-1);
    }
    shared = (int *) mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int read;
    int last_read=-1;

    struct sigaction action;
    action.sa_handler = alarm_handler;
    sigaction(SIGALRM, &action, NULL);
    alarm(20);



    while(1){
        read = (*shared);
        if(read!=last_read){
            if(read%2==0)
                count_even++;
            else
                count_odd++;
            last_read = read;
        }


    }


    close(fd);
    munmap(shared, shared_mem_size);
    exit(0);
}
