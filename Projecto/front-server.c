#include "TCPlib.h"
#include "inetutils.h"
#include "item.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
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

int create_ds_stub() {
    return TCPconnect(getIPbyname((char *) "127.0.0.1"), 9998);
}

int proxy_handler(int sock_fd){
    kv_msg key_value;
    uint8_t * to_recv = (uint8_t *) &key_value;
    bool stop = false;
    int ds_fd = create_ds_stub();

    while(!stop){
        int cmd = TCPrecv(sock_fd, to_recv, sizeof(kv_msg));
        if( cmd < 0 ){
            return result;
        }

        #ifdef DEBUG
            printf("Request: \n\ttype: %s\n\tkey:%d\n\tValue_len:%d\n",
                msg_type_to_str(key_value.type), key_value.key, key_value.value_len);
        #endif

        switch(key_value.type){
            case WRITE_REQ:
                #ifdef DEBUG
                    printf("Received WRITE_REQ\n");
                #endif
                /*TODO: enviar pedido de escrita sem overwrite */
                TCPsend(ds_fd, to_recv, sizeof(kv_msg));
                break;
            case WRITE_REQ_OW:
                #ifdef DEBUG
                    printf("Received WRITE_REQ_OW\n");
                #endif
                /*TODO: enviar pedido de escrita com overwrite */
                TCPsend(ds_fd, to_recv, sizeof(kv_msg));
                break;
            case READ_REQ:
                #ifdef DEBUG
                    printf("Received READ_REQ\n");
                #endif
                /*TODO: enviar pedido de leitura */
                TCPsend(ds_fd, to_recv, sizeof(kv_msg));
                break;
            case DELETE_REQ:
                #ifdef DEBUG
                    printf("Received DELETE_REQ\n");
                #endif
                /*TODO: enviar pedido de remoção */
                TCPsend(ds_fd, to_recv, sizeof(kv_msg));
                break;
            default:
                return -1;
        }
    }
}

void * answer_call( void *args ){
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    printf("\n\n\t#NEW THREAD\n");

    pthread_detach(pthread_self());
    printf("\tSock_fd: %d\n\n\n", sock_fd);

    while (1) {
        int err = create_ds_stub(sock_fd);

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
