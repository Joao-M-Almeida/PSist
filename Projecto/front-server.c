#include "front-lib.h"

extern int end;
extern int server;
extern int data_server_port;
extern struct arguments *args;

int main() {
    /* code */
    unsigned short port = FRONT_SERVER_PORT;
    int incoming;

    /*Threads*/
    pthread_t call_tid, connection_tid, command_tid;

    /* Interruption Handlers */
    set_int_handler();

    /* Inicializar variaveis globais */
    end = 0;
    server = -1;
    data_server_port = -1;

    /*Create arguments structure*/
    args = (struct arguments *) malloc(sizeof(struct arguments));

    #ifdef DEBUG
        printf("(FRONT) Calling Worker Threads\n");
    #endif
    /* Worker Threads */
    pthread_create(&command_tid, NULL, &command_handler, NULL);
    pthread_create(&connection_tid, NULL, &connection_worker, NULL);

    /*Bind all local inet adresses and port*/
    server = setup_server(port);
    if(server<0){
        clean_up(-2);
    }
    printf("Front Server (%d) Waiting for connections @ 127.0.0.1:%d\n", getpid(), port);

    while(!end){
        incoming = TCPaccept(server);
        #ifdef DEBUG
            printf("(FRONT) Accepted connection\n");
        #endif
        /* Verifica se houve */
        if(!end){
            if (incoming<0){
                perror("TCPaccept");
                clean_up(-1);
            } else {
                /* Call Handler Thread */
                args->sock_fd = incoming;
                pthread_create(&call_tid, NULL, &answer_call, (void *) &args);
            }
        }
    }
    pthread_join(connection_tid, NULL);

    clean_up(0);

    return -1;
}
