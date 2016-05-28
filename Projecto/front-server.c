#include "front-lib.h"

extern int end;
extern int stop;
extern int proper;
extern int ready;
extern int server;
extern int data_server_port;
extern struct arguments *args;

int main(int argc, char const *argv[]) {
    /* code */
    unsigned short port = FRONT_SERVER_PORT;

    /*Threads*/
    pthread_t call_tid, connection_tid, command_tid;

    /*Server can receive port number as argument*/
    if(argc>1) port = atoi(argv[1]);

    #ifdef DEBUG
        printf("(FRONT) Initializing Global Variables\n");
    #endif

    set_int_handler();

    /* Inicializar variaveis globais */
    connected = 0;
    proper = 0;
    end = 0;
    stop = 1;
    ready = 0;
    data_server_port = -1;

    /*Create arguments structure*/
    args = (struct arguments *) malloc(sizeof(struct arguments));

    #ifdef DEBUG
        printf("(FRONT) Calling Worker Threads\n");
    #endif
    /* Worker Threads */
    pthread_create(&command_tid, NULL, &command_handler, (void *) &args);
    pthread_create(&connection_tid, NULL, &connection_worker, (void *) &args);

    while(!end){
        #ifdef DEBUG
            printf("(FRONT) Waiting to be ready\n");
        #endif
        while(ready==0 || ready==2);
        if(ready==1){
            stop = 0;
            printf("Data Server Port: %d\n", data_server_port);
        } else if(ready==-1){
            stop = 1;
            end = 1;
        }
        printf("(FRONT) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
        ready = 0;

        if(!end){
            /*Bind all local inet adresses and port*/
            server = setup_server(port);

            int incoming;
            printf("Front Server (%d) Waiting for connections @ 127.0.0.1:%d\n", getpid(), port);
            printf("(FRONT) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
            while (!stop && !end) {
                printf("(FRONT) Waiting for a connection\n");
                incoming = TCPaccept(server);
                if(!stop && !end){
                    if (incoming<0){
                        /*perror("TCPaccept");*/
                        /*clean_up(-1);*/
                    } else {
                        /* Call Handler Thread */
                        args->sock_fd = incoming;
                        pthread_create(&call_tid, NULL, &answer_call, (void *) &args);
                    }
                }
            }
            if(server != -1)
                close(server);
        }
    }
    while(!proper);
    clean_up(0);

    return -1;
}
