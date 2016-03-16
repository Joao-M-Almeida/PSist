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

    int fd = shm_open(memname,  O_RDWR, 0666);
    if (fd == -1)
        error_and_die("shm_open");

    int even_odd_file = open("even_odd.fifo",   O_RDWR);
    int sync_file = open("sync.fifo",   O_WRONLY);

    shm_region *ptr = (shm_region *) mmap(0, sizeof(shm_region), PROT_READ , MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
        error_and_die("mmap");
    close(fd);


    i = 0;
    int read_byte;
    int write_byte=1;
    int n_read;
    while(1){
        n_read = read(even_odd_file, &read_byte,sizeof(int));
        if(n_read>0){
            if(ptr->value%2 == 0){
                write(sync_file, &write_byte,sizeof(int));
                printf("%d %ld\n", i++, ptr->value);
            }else{
                write(even_odd_file, &write_byte,sizeof(int));
            }
        }

    }

}
