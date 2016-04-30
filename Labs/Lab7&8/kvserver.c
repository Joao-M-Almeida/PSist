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
#include <unistd.h>
#include <sys/types.h>

#define STORESIZE 200
#define DEFAULTPORT 9999
#define MAXCLIENTS 5

/*
Server to handle acess to the Key Value store. Only serves a client at a time.

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
hash_table kv_store;
struct arguments *args;

/*TODO: Implementar modos para saber o que limpar*/
void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    delete_hash(kv_store, STORESIZE, free);
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

    printf("\n\n#NEW THREAD\n\n");

    pthread_detach(pthread_self());
    printf("And awaaay we goo!!\n");

    printf("Sock_fd: %d\n\n\n", sock_fd);

    while (1) {
        int err = process_psiskv_prequest(sock_fd, kv_store,STORESIZE);

        if (err<0){
            if(err == -1){
                perror("Process Request");
                return(NULL);
            }else if (err == -2){
                printf("Connection Closed by peer.\n" );
                break;
            }
        }
    }

    close(sock_fd);

    printf("And I'm dead(done)!!\n");
    printf("#END OF THREAD\n\n");

    return(NULL);
}

int main(int argc, char const *argv[]) {
    /* code */

    unsigned short port = DEFAULTPORT;

    /*Threads*/
    pthread_t tid;
    pthread_mutex_t readlock, writelock;


    /* Capture CTRL-C to exit gracefuly */
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
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
