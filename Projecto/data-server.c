#include "data-lib.h"

/*retirar*/
#ifndef DEBUG
#define DEBUG
#endif

extern int end;
extern int server;
extern int port;
extern struct arguments *args;

int main(int argc, char const *argv[]) {
    int incoming;

    /*Threads*/
    pthread_t call_id, connection_tid;

    set_int_handler();

    /*Server can receive port number as argument*/
    if(argc>1) port = atoi(argv[1]);

    setup_backup();

    /* Inicializar variaveis globais */
    end = 0;
    server = -1;
    port = -1;

    /*Create arguments structure*/
    args = (struct arguments *) malloc(sizeof(struct arguments));

    #ifdef DEBUG
        printf("(DATA) Calling Worker Thread\n");
    #endif
    /* Worker Thread */
    pthread_create(&connection_tid, NULL, &connection_worker, NULL);

    /*Bind all local inet adresses and port*/
    server = setup_server();
    if(server<0){
        clean_up(-1);
    }
    printf("Data Server (%d) Waiting for connections @ 127.0.0.1:%d\n", getpid(), port);

    while(!end){
        incoming = TCPaccept(server);
        if (incoming<0){
            perror("TCPaccept");
            clean_up(-1);
        } else {
            /* Call Handler Thread */
            args->sock_fd = incoming;
            pthread_create(&call_id, NULL, &answer_call, (void *) &args);
        }
    }
    pthread_join(connection_tid, NULL);

    clean_up(0);

    return -1;
}
