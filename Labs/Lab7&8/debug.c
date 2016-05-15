#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
    Idea from: http://stackoverflow.com/questions/16884306/print-bytes-from-memory-in-c
*/
void print_bytes(void * to_print, uint32_t size){
    uint32_t i;

    printf("[ ");
    for(i = 0; i < size; i++)
    {
    printf("%02x ", ((unsigned char *) to_print)[i]);
    }
    printf("]\n");
}
