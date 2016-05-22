#include "TCPlib.h"
#include "phash-lib.h"
#include "inetutils.h"
#include "item.h"
#include "psiskv.h"
#include "psiskv_server.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define STORESIZE 11
#define DEFAULTPORT 9999
#define MAXCLIENTS 5
#define BACKUP_PATH "backup.data"
#define LOG_PATH "log.data"
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
hash_table * kv_store;
struct arguments *args;

/*TODO: Implementar modos para saber o que limpar*/
void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    backup_hash(kv_store, (char *) BACKUP_PATH);
    delete_hash(kv_store);
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
    FILE * aux = fopen(BACKUP_PATH, "r");
    /*Check if Backup exists and Create Hash Table */
    if(aux==NULL){
        printf("No Backup found... Starting from scratch\n");
        aux = fopen(LOG_PATH, "r");
        if(aux==NULL){
            printf("No Log found...\n");
            kv_store =  create_hash(STORESIZE, (char *) LOG_PATH, create_struct, destroy_struct, struct_to_str, struct_get_size);
        }else{
            fclose(aux);
            printf("Log found\n");
            char temp_log[1024];
            strcpy(temp_log, LOG_PATH);
            strcat(temp_log, ".temp");
            kv_store = create_hash(STORESIZE, temp_log, create_struct, destroy_struct, struct_to_str, struct_get_size);
            /*process old log and rename temp*/
            if(process_hash_log(kv_store, (char *) LOG_PATH)<0){
                printf("Error reading Log... Ignoring\n");
            }
            rename_log(kv_store->log, (char *) LOG_PATH);
        }
    }else{
        fclose(aux);
        /*backup exists, check if log exists*/
        aux = fopen(LOG_PATH, "r");
        if(aux==NULL){
            printf("Backup found and no Log found...\n");
            /*Log doesn't exist*/
            kv_store = create_hash_from_backup(STORESIZE, (char *) BACKUP_PATH, (char *) LOG_PATH, create_struct, destroy_struct, struct_to_str, struct_get_size);
        }else{
            fclose(aux);
            printf("Backup and Log found...\n");
            char temp_log[1024];
            strcpy(temp_log, LOG_PATH);
            strcat(temp_log, ".temp");
            kv_store = create_hash_from_backup(STORESIZE, (char *) BACKUP_PATH, temp_log, create_struct, destroy_struct, struct_to_str, struct_get_size);
            /*process old log and rename temp*/
            if(process_hash_log(kv_store, (char *) LOG_PATH)<0){
                printf("Error reading Log... Ignoring\n");
            }
            rename_log(kv_store->log, (char *) LOG_PATH);
        }
    }
    if( kv_store == NULL){
        exit(-1);
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
