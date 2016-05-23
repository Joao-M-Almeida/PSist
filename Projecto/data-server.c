#include "TCPlib.h"
#include "phash-lib.h"
#include "inetutils.h"
#include "item.h"
#include "psiskv.h"
#include "psiskv_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define STORESIZE 11
#define DEFAULTPORT 9998
#define MAXCLIENTS 5

/*
Server to handle acess to the Key Value store. Also  serves plenty of clients at a time.

1. Create Hash store;
2. Bind TCP socket;
3. Wait for requests;
4. Handle request, go back to 3;
5. Close connections;
6. Free Hash store;
7. Exit;

*/

struct arguments {
    int sock_fd;
};

int server;
hash_table * kv_store;
struct arguments *args;

void * wakeup_front_server( void *args ){
    pthread_detach(pthread_self());
    printf("PRE execve\n");
    execve("./front_server", args, NULL);
    printf("POS execve\n");
    return(NULL);
}

/*int fd;
int connected = 0;
int front_server_port = 10100;
int data_server_port = 10101;
char token = '\n';
pthread_detach(pthread_self());

fd = socket(AF_UNIX, SOCK_STREAM, 0);
if(fd==-1){ return(NULL); }

memset(&address, 0, sizeof(address));
address.sin_family = AF_UNIX;
address.sin_addr.s_addr = htonl(atoh("0.0.0.0"));
address.sin_port = htons(front_server_port);

if(connect(fd, (struct sockaddr*)&address, sizeof(address)) != -1){
    connected = 1;
    while(connected){
        if(TCPsend(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        if(TCPrecv(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
    }
}

fd = socket(AF_UNIX, SOCK_STREAM, 0);
address.sin_addr.s_addr = htonl(INADDR_ANY);
address.sin_port = htons(data_server_port);
if(bind(fd, (struct sockaddr*)&address, sizeof(address))==-1){ return(NULL); }
if(listen(fd, MAXCLIENTS)){ return(NULL); }

while(1){
    while(connected){
        if(TCPsend(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        if(TCPrecv(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
    }
    pthread_create(&tid, NULL, &wakeup_front_server, (void *) &args);
    if(TCPaccept(fd) != -1){ connected = 1; }
}
*/

void * front_server_puller( void *args ){
    /*
    pthread_t tid;
    unsigned int s, s2;
    struct sockaddr_un local, remote;
    int len;
    */
    printf("Hello from inside front server puller\n");

    printf("Byeub\n");
    return(NULL);
}

/*TODO: Implementar modos para saber o que limpar*/
void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    delete_hash(kv_store, destroy_struct);
    free(args);
    TCPclose(server);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("Received signal: %d\n", signum);
    clean_up(0);
}

void * answer_call( void *args ){
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    /*printf("\n\n\t#NEW THREAD\n");*/

    pthread_detach(pthread_self());
    /*printf("\tSock_fd: %d\n\n\n", sock_fd);*/

    while (1) {
        int err = process_psiskv_prequest(sock_fd,kv_store);

        if (err<0){
            if(err == -1){
                printf("Error while processing request.\n" );
                /*perror("Process Request");*/
                return(NULL);
            }else if (err == -2){
                printf("Connection Closed by peer.\n" );
                break;
            }
        }
    }

    close(sock_fd);
    /*printf("\t#END OF THREAD\n\n");*/

    return(NULL);
}

int main(int argc, char const *argv[]) {
    /* code */

    unsigned short port = DEFAULTPORT;

    /*Threads*/
    pthread_t tid;

    write(1,"INSIDIOUS\n",11);

    /* Capture CTRL-C to exit gracefuly */
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    /*Server can receive port number as argument*/
    if(argc>1){
        port = atoi(argv[1]);
    }

    /*Create Hash Table*/
    kv_store =  create_hash(STORESIZE);

    /*Create arguments structure*/
    args = (struct arguments *) malloc(sizeof(struct arguments));

    /*Bind all local inet adresses and port*/
    server = TCPcreate(INADDR_ANY, port);
    if (server<0){
        perror("TCPcreate");
        clean_up(-1);
    }

    /*Set socket as passive*/
    int err = listen(server, MAXCLIENTS);
    if (err<0){
        perror("Listen Error: Port occupied?");
        clean_up(-1);
    }

    int incoming;
    while (1) {
        printf("Server Waiting for connection\n");

        incoming = TCPaccept(server);

        if (incoming<0){
            perror("TCPaccept");
            clean_up(-1);
        }

        /*criar thread*/
        printf("Creating a thread\n");
        args->sock_fd = incoming;
        pthread_create(&tid, NULL, &answer_call, (void *) &args);

    }
    clean_up(0);
}
