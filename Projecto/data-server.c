#include "data-lib.h"

/*retirar*/
#ifndef DEBUG
#define DEBUG
#endif

extern int end;
extern int proper;
extern int ready;
extern int server;
extern int port;
extern struct arguments *args;

int main(int argc, char const *argv[]) {
    int stop;

    /*Threads*/
    pthread_t call_id, connection_tid;

    set_int_handler();

    /*Server can receive port number as argument*/
    if(argc>1) port = atoi(argv[1]);

    setup_backup();

    #ifdef DEBUG
        printf("(DATA) Initializing global variables\n");
    #endif

    /* Inicializar variaveis globais */
    connected = 0;
    end = 0;
    proper = 0;
    ready = 0;
    port = -1;

    stop = 0;

    /*Create arguments structure*/
    args = (struct arguments *) malloc(sizeof(struct arguments));

    #ifdef DEBUG
        printf("(DATA) Calling Worker Thread\n");
    #endif
    pthread_create(&connection_tid, NULL, &connection_worker, (void *) &args);

    while(!end){
        #ifdef DEBUG
            printf("(DATA) Waiting to be ready\n");
        #endif
        while(ready==0 || ready==2);
        if(ready==1){
            stop = 0;
        } else if(ready==-1){
            stop = 1;
            end = 1;
        }
        printf("(DATA) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
        ready = 0;
        printf("I'm so fucking ready\n");
        if(!end){
            /*Bind all local inet adresses and port*/
            server = setup_server();

            int incoming;
            printf("Data Server (%d) Waiting for connections @ 127.0.0.1:%d\n", getpid(), port);
            printf("(DATA) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
            while (!end && !stop) {
                incoming = TCPaccept(server);
                if (incoming<0){
                    printf("(DATA) TCP Connection fell\n");
                    /*perror("TCPaccept");*/
                    /*clean_up(-1);*/
                } else {
                    /* Call Handler Thread */
                    args->sock_fd = incoming;
                    pthread_create(&call_id, NULL, &answer_call, (void *) &args);
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
