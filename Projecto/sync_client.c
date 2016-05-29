#include "TCPlib.h"
#include "inetutils.h"
#include "item.h"
#include "psiskv.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main() {
    char buf[25];
    int id;
    int i;
    for (i = 0; i < 1; i++) {
        id = fork();
        if(id==0){
            break;
        }
    }
    printf("I'm client with PID: %d\n",id);
    int kv = kv_connect( (char *) "127.0.0.1", 9999);
    if(kv<=0){
        exit(-1);
    }
    printf("PID: %d - Connected\n", id);
    sprintf(buf,"%d",id);
    printf("Writing:\n");
    int r;
    for (i = 0; i < 11; i++) {
        r = kv_write(kv, i , buf , strlen(buf)+1, 0); /* will not overwrite*/
        if(r<0){
            printf("PID: %d Write error\n",id);
            exit(-1);
        }
    }
    kv_close(kv);

    kv = kv_connect( (char *) "127.0.0.1", 9999);
    if(kv<=0){
        exit(-1);
    }
    for (i = 0; i < 11; i++) {
        r = kv_read(kv, i , buf,  25);
        if(r<0){
            printf("PID: %d Read error\n",id);
            exit(-1);
        }
        printf("PID: %d I=%d Read: %s\n",id, i, buf );
    }
    exit(0);
}
