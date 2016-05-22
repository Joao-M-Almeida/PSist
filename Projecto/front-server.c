#include "TCPlib.h"
#include "inetutils.h"
#include "item.h"
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
#define DEFAULTPORT 9999
#define MAXCLIENTS 5

/*
Server to handle acess to the Data Server.Serves plenty of clients at a time.

1. Bind TCP socket;
2. Wait for requests;
3. Handle request, go back to 2;
4. Close connections;
5. Exit;

*/

struct arguments {
    int sock_fd;
};

int server;
struct arguments *args;

/*TODO: Implementar modos para saber o que limpar*/
void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    free(args);
    TCPclose(server);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("Received signal: %d\n", signum);
    clean_up(0);
}

void * wakeup_data_server( void *args ){
    pthread_detach(pthread_self());
    execve("./data-server", args, NULL);
    return(NULL);
}

void * data_server_puller( void *args ){
    pthread_t tid;
    struct sockaddr_in address;
    int fd;
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
    address.sin_port = htons(data_server_port);

    if(connect(fd, (struct sockaddr*)&address, sizeof(address)) != -1){
        connected = 1;
        while(connected){
            if(TCPsend(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            if(TCPrecv(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        }
    }

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(front_server_port);
    if(bind(fd, (struct sockaddr*)&address, sizeof(address))==-1){ return(NULL); }
    if(listen(fd, MAXCLIENTS)){ return(NULL); }

    while(1){
        while(connected){
            if(TCPsend(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            if(TCPrecv(fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        }
        pthread_create(&tid, NULL, &wakeup_data_server, (void *) &args);
        if(TCPaccept(fd) != -1){ connected = 1; }
    }

    return(NULL);
}

void * answer_call( void *args ){
    char buffer[128];
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    sprintf(buffer, "127.0.0.1:9998");

    pthread_detach(pthread_self());

    /*printf("\tSock_fd: %d\n\n\n", sock_fd);*/

    TCPsend(sock_fd, (uint8_t *) buffer, strlen(buffer)*sizeof(char));

    close(sock_fd);

    return(NULL);
}

int main(int argc, char const *argv[]) {
    /* code */
    int stop = 0;
    unsigned short port = DEFAULTPORT;

    /*Threads*/
    pthread_t tid;


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
    printf("Server Waiting for connections @ 127.0.0.1:%d\n", port);
    while (!stop) {

        incoming = TCPaccept(server);

        if (incoming<0){
            perror("TCPaccept");
            clean_up(-1);
        }

        /*criar thread*/
        args->sock_fd = incoming;
        pthread_create(&tid, NULL, &answer_call, (void *) &args);

    }
    clean_up(0);

    return -1;
}
