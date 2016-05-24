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
#include <sys/un.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define STORESIZE 11
#define DATA_SERVER_PORT 9998
#define MAXCLIENTS 5
#define SOCK_PATH "./ipc_sock"
#define JESUS_POWER 1
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

void wakeup_front_server(){
    char *args[] = {"./front_server",
                    NULL};
    int id = fork();
    if(id!=0){
        printf("(DATA) Resing front server %d\n", id);
        execv("./front_server", args);
        _Exit(-1);
    }
    return;
}

void front_server_puller(){

    unsigned int local_fd, remote_fd;
    struct sockaddr_un local, remote;
    char token = '\n';
    int len, t, connected = 0;

    remote_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    if (connect(remote_fd, (struct sockaddr *)&remote, len) != -1) {
        connected = 1;
        printf("data connected to front\n");
        printf("Will you marry me?\n");
        while(connected){
            if(TCPsend(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            if(TCPrecv(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            /*printf("(DATA) ping: %c", token);*/
            sleep(1);
        }
    }
    printf("But you promissed ):\n");
    close(remote_fd);

    local_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if(bind(local_fd, (struct sockaddr *)&local, len) == -1){
      printf("Mega shit (data bind)\n");
      exit_gracefuly(3);
    }

    if(listen(local_fd, 5) == -1){
        printf("Mega shit (data listen)\n");
        exit_gracefuly(3);
    }

    wakeup_front_server();

    while(1){
        t = sizeof(remote);
        remote_fd = accept(local_fd, (struct sockaddr *)&remote, (socklen_t*) &t);

        if(remote_fd != -1){
          printf("I DO (Data)\n");
          connected = 1;
        }
        while(connected){
            if(TCPrecv(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
            /*printf("(DATA) ping: %c", token);*/
            sleep(1);
            if(TCPsend(remote_fd, (uint8_t*) &token, sizeof(char)) == -1){ connected = 0; }
        }
    }

    return;
}

void * answer_call( void *args ){
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    printf("\n\n\t#NEW THREAD\n");

    pthread_detach(pthread_self());
    printf("\tSock_fd: %d\n\n\n", sock_fd);

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
    printf("\t#END OF THREAD\n\n");

    return(NULL);
}

int main(int argc, char const *argv[]) {
    /* code */

    unsigned short port = DATA_SERVER_PORT;

    /*Threads*/
    pthread_t tid, pullup_pid;

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

    //definir o conteudo do args
    if(JESUS_POWER){
        pullup_pid = fork();
        if(pullup_pid!=0){
            front_server_puller();
            _Exit(-1);
        }
    }

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
    while (1) {
        printf("Waiting\n");

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
