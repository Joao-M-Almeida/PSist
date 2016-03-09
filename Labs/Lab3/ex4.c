/*
gcc -Wall -pedantic -ansi -std=gnu99 ex4_gen_random.c -o ex4_gen_random.out
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


void error_and_die(char error[]) {
    printf("%s\n", error );
}

int main() {


    int fd = shm_open("gen_rand", O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1)
        error_and_die("shm_open");

    int r = ftruncate(fd, sizeof(int));
    if (r != 0)
        error_and_die("ftruncate");
    int * v_int = (int *) mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);



    srandom(getpid());
    random();

    exit(0);
}
