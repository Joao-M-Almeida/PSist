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


    shm_region *ptr = (shm_region *) mmap(0, sizeof(shm_region), PROT_READ , MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
        error_and_die("mmap");
    close(fd);




    i = 0;
    while(1){
        sleep(1);
        if(ptr->value%2 == 0){
            printf("%d %ld\n", i++, ptr->value);
        }
    }

}
