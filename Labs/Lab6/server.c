#include "hash-lib.h"
#include "TCPlib.h"
#include "inetutils.h"
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#define STDIN 0
#define max(A,B) ((A) >= (B)?(A):(B))
#define INADDR_ANY ((unsigned long int) 0x00000000)

void errfunc( int errno ){
    return;
}

int main(int argc, char *argv[]){
    int server_port;
    /*char server_ip[32];*/
    int server_fd, maxfd, new_fd, afd;
    bool stop_server = false;
    int counter;
    fd_set rfds;
    char command[32];
    int state = 0;
    server_port = 9999;

    server_fd = TCPcreate(INADDR_ANY, server_port);
    afd = 0;

    while(!stop_server){
        FD_ZERO(&rfds);
        FD_SET(STDIN, &rfds);
        FD_SET(server_fd, &rfds);
        maxfd = max(server_fd, afd);
        counter = select(maxfd + 1, &rfds, (fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter <= 0) errfunc(1);
        if(FD_ISSET(STDIN, &rfds)){
            fgets(command, 128, stdin);
            if(!strcmp(command, "exit\n")){
                printf("Bye!\n");
                stop_server = true;
            }
        }
        if(FD_ISSET(server_fd, &rfds)){
            new_fd = TCPaccept(server_fd);
            switch(state){
                case 0:
                    afd = new_fd;
                    state = 1;
                    break;
                case 1:
                    TCPclose(new_fd);
                    break;
            }

        }

        if(FD_ISSET(afd, &rfds)){
            /*funções da API exposta*/
        }
    }
    return 0;
}
