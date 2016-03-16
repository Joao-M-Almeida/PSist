/* gcc -Wall -pedantic -ansi -std=gnu99 ex1_fifo.c -o ex1_fifo.out -lrt
*/

#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lab4-1.h"

void error_and_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(){

    char const *memname = "odd_even";
    int i;


    shm_unlink(memname);
    int fd = shm_open(memname, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
        error_and_die("shm_open");

    int r = ftruncate(fd, sizeof(shm_region));
    if (r != 0)
        error_and_die("ftruncate");

    shm_region *ptr =  ( shm_region*) mmap(0, sizeof(shm_region), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
        error_and_die("mmap");
    close(fd);

    int even_odd_fifo = mkfifo("even_odd.fifo",0666);
    if(even_odd_fifo !=0)
        error_and_die("Unable to create FIFO");
    int even_odd_file = open("even_odd.fifo",  O_WRONLY);
    if(even_odd_file ==-1)
        error_and_die("Unable to open even_odd FIFO");


    int sync_fifo = mkfifo("sync.fifo",0600);
    if(sync_fifo !=0)
        error_and_die("Unable to create FIFO");
    int sync_file = open("sync.fifo",   O_RDONLY|O_NONBLOCK);
    if(sync_file ==-1)
        error_and_die("Unable to open sync FIFO");

    int fifo_ready = 0;

    int num_written = 1;

    int n_written = 0;

    ptr->value = random();
    printf("%d: %ld\n",0,ptr->value);
    n_written = write(even_odd_file,&num_written,sizeof(int));
    if(n_written==-1)
        error_and_die("Write problem");
    for(i=1; i < 10; ){
        if(read(sync_file, &fifo_ready, sizeof(int))!=0){
            ptr->value = random();
            printf("%d: %ld\n",i,ptr->value);
            n_written = write(even_odd_file,&num_written,sizeof(int));
            i++;
            sleep(1);
        }
    }

    shm_unlink(memname);
    close(sync_file);
    close(even_odd_file);

}
