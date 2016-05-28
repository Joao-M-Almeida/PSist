#include "front-lib.h"
/*extern int errno;*/

/*
Server to handle acess to the Data Server.
Serves plenty of clients at a time.

1. Bind TCP socket;
2. Wait for requests;
3. Handle request, go back to 2;
4. Close connections;
5. Exit;

*/

int port;
int server;
int connected;
/*int interrupt;*/
int end, stop, ready, proper;
int data_server_port;
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

/* Thread para acordar o data server,
mete o ready a 0 para não fazer binds antes do fork */
void wakeup_data_server(){
    char *args[] = {(char  *) DS_PATH,
                    NULL};
    if(ready==2 && connected==0){
        int id = fork();
        if(id!=0){
            /*child becomes ressurected server*/
            printf("(FRONT) launching data server %d\n", id);
            if(execv(DS_PATH, args) == -1)
                printf("Error: %d\n", errno);
            _Exit(-1);
        } else {
            ready = 0;
        }
    }
    return;
}

int setup_server( int port ){
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

    return server;
}

/*
    Fica à espera que alguem lhe diga para parar o servidor
    usage: exit, Exit, e, E
    TODO: should be only 'quit'
*/
void * command_handler(void *args){
    char buf[BUF_LEN];
    while(!end){
        fgets(buf, BUF_LEN, stdin);
        if(!strcmp(buf, "e\n") || !strcmp(buf, "E\n") || !strcmp(buf, "Exit\n") || !strcmp(buf, "exit\n")){
            /*interrupt = 1;*/
            stop = 1;
            end = 1;
        }
    }
    return(NULL);
}

/*
    Thread de comunicação com o data server,
    descobre o porto em que está
    e envia sinal para desligar, quando pedido.
*/
void * connection_worker(void *args){

    int local_fd, remote_fd;
    struct sockaddr_un local, remote;
    char send_tok[8], recv_tok[8];
    int len, t;
    int aux;

    remote_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    #ifdef DEBUG
        printf("(Front %d) started connection worker, attempting to be a client\n", getpid());
    #endif

    if (connect(remote_fd, (struct sockaddr *)&remote, len) != -1) {
        connected = 1;
        printf("front connected to data\n");
        while(connected || (end && !proper)){
            if(end==1){
                strcpy(send_tok, "EXIT\n");
            }else{
                strcpy(send_tok, "PING\n");
            }
            if(TCPsend(remote_fd, (uint8_t*) send_tok, strlen(send_tok)) == -1){
                connected = 0;
            }
            if(connected==1){
                if(TCPrecv(remote_fd, (uint8_t*) recv_tok, 8) == -1){
                    connected = 0;
                }
                if(connected==1){
                    if(end==1){
                        if(!strcmp(recv_tok,"OK\n")){
                            proper = 1;
                        }
                    } else {
                        if(!strcmp(recv_tok,"PING\n")){
                            sleep(1);
                        } else {
                            if(ready==0){
                                if(sscanf(recv_tok, "%d\n", &aux) != 1){
                                    ready = -1;
                                    close(remote_fd);
                                    return(NULL);
                                }
                                if(aux >= 10000 && aux < 11000){
                                    printf("Recv port: %d\n", aux);
                                    data_server_port = aux;
                                    ready = 1;
                                } else {
                                    ready = -1;
                                    close(remote_fd);
                                    return(NULL);
                                }
                                break;
                            } else {
                                ready = -1;
                                close(remote_fd);
                                return(NULL);
                            }
                        }
                    }

                }
            }
        }
    }
    close(remote_fd);

    #ifdef DEBUG
        printf("(Front %d) DS connection failed\n", getpid());
    #endif

    while(!end){
        ready = 2;
        #ifdef DEBUG
            printf("(Front %d) Wake Up Data\n", getpid());
        #endif
        wakeup_data_server();

        local_fd = socket(AF_UNIX, SOCK_STREAM, 0);

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCK_PATH);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);

        /* Para não copiar bind no fork */
        printf("(FRONT %d) Waiting for fork\n", getpid());
        while(ready==2);
        printf("(FRONT %d) Ready to go\n", getpid());

        if(bind(local_fd, (struct sockaddr *)&local, len) == -1){
            printf("Mega shit (front bind) #%d\n", errno);
            exit_gracefuly(3);
        }

        if(listen(local_fd, 5) == -1){
            printf("Mega shit (front listen) #%d\n", errno);
            exit_gracefuly(3);
        }

        t = sizeof(remote);
        printf("(FRONT %d) Waiting for connection\n", getpid());
        remote_fd = accept(local_fd, (struct sockaddr *)&remote, (socklen_t*) &t);
        printf("Accept: %d\n", remote_fd);
        if(remote_fd != -1){
            connected = 1;
        } else {
            printf("(FRONT %d) Unable to accept\n", getpid());
        }
        while(connected){
            printf("(FRONT %d) Waiting...\n", getpid());
            if(TCPrecv(remote_fd, (uint8_t*) recv_tok, 8) == -1){
                connected = 0;
                printf("(FRONT %d) Connection fell @ Recv\n", getpid());
            }
            printf("(FRONT) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
            if(connected==1){
                printf("(FRONT %d) Received a token: %s\n", getpid(), recv_tok);
                if(end==1){
                    if(!strcmp(recv_tok,"OK\n")){
                        proper = 1;
                    }
                } else {
                    if(!strcmp(recv_tok,"PING\n")){
                        sleep(1);
                    } else {
                        if(ready == 0){
                            if(sscanf(recv_tok, "%d\n", &aux) != 1){
                                ready = -1;
                                close(remote_fd);
                                return(NULL);
                            }
                            if(aux >= 10000 && aux < 11000){
                                data_server_port = aux;
                                ready = 1;
                            } else {
                                ready = -1;
                                close(remote_fd);
                                return(NULL);
                            }
                        } else {
                            ready = -1;
                            close(remote_fd);
                            return(NULL);
                        }
                    }
                }
                if(end==1){
                    strcpy(send_tok, "EXIT\n");
                } else {
                    strcpy(send_tok, "PING\n");
                }
                printf("(FRONT) Ready: %d; End: %d; Stop: %d\n", ready, end, stop);
                printf("(FRONT %d) Sending token: %s\n", getpid(), send_tok);
                if(TCPsend(remote_fd, (uint8_t*) send_tok, strlen(send_tok)) == -1){
                    connected = 0;
                    printf("(FRONT %d) Connection fell @ Send\n", getpid());
                }
            }
        }
        printf("Why am I here?\n");
        close(local_fd);
        stop = 1;
        close(server);
        server = -1;
    }

    return(NULL);
}

void * answer_call( void *args ){
    char buffer[128];
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    sprintf(buffer, "127.0.0.1:%d\n", data_server_port);

    pthread_detach(pthread_self());

    printf("\tSock_fd: %d\n\n\n", sock_fd);

    TCPsend(sock_fd, (uint8_t *) buffer, strlen(buffer)*sizeof(char));

    close(sock_fd);

    return(NULL);
}

/* Capture CTRL-C to exit gracefuly */
void set_int_handler(){
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
}
