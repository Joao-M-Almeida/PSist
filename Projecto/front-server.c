#include "TCPlib.h"
#include "inetutils.h"
#include "item.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define STORESIZE 11
#define DEFAULTPORT 9999
#define MAXCLIENTS 5
#define SOCK_PATH "/home/ipc_sock"

extern int errno;

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
    unlink(SOCK_PATH);
    clean_up(0);
}

void wakeup_data_server(){
    char *args[] = {"./data_server",
                    NULL};
    int id = fork();
    if(id!=0){
      printf("Resing data server\n");
      if(execv("./data_server", args) == -1)
        printf("Error: %d\n", errno);
      printf("bye bye\n");
      _Exit(-1);
    }
    return;
}

void * data_server_puller( void *args ){

    unsigned int local_fd, remote_fd;
    struct sockaddr_un local, remote;
    char token = '\n';
    int len, t, connected = 0;

    remote_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    unlink(SOCK_PATH);

    if (connect(remote_fd, (struct sockaddr *)&remote, len) == -1) {
        connected = 1;
        printf("front connected to data\n");
        while(connected){
          if(TCPsend(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
          if(TCPrecv(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        }
    }
    close(remote_fd);
    local_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if(bind(local_fd, (struct sockaddr *)&local, len) == -1){
        printf("Mega shit (front bind)\n");
    }

    if(listen(local_fd, 5) == -1){
        printf("Mega shit (front listen)\n");
    }

    wakeup_data_server();

    while(1){
        t = sizeof(remote);
        remote_fd = accept(local_fd, (struct sockaddr *)&remote, (socklen_t*) &t);

        if(remote_fd != -1){
            printf("I DO (Front)\n");
            connected = 1;
        }
        while(connected){
            if(TCPsend(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            if(TCPrecv(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            sleep(1);
        }
    }

    while(1);
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
    pthread_t call_tid, pullup_tid;

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

    //definir o conteudo do args

    pthread_create(&pullup_tid, NULL, &data_server_puller, (void *) &args);

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
        pthread_create(&call_tid, NULL, &answer_call, (void *) &args);

    }
    clean_up(0);

    return -1;
}
