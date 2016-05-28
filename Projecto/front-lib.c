#include "front-lib.h"
#include "debug.h"
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
int ipc_server;
int end;
int data_server_port;
struct arguments *args;

/*TODO: Implementar modos para saber o que limpar
has to wait for all threads
*/
void clean_up(int exit_val){
    printf("(Front %d) Cleaning UP... \n", getpid());
    free(args);
    TCPclose(server);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("(Front %d) Received signal: %d\n",getpid(), signum);
    /*unlink(SOCK_PATH);
    clean_up(0);*/
    end =1;
}

/* Thread para acordar o data server,
mete o ready a 0 para não fazer binds antes do fork */
void wakeup_data_server(){
    char *args[] = {(char  *) DS_PATH,
                    NULL};
    int id = fork();
    if(id==0){
        if(server != -1) { close(server); }
        if(ipc_server != -1) { close(ipc_server); }
        printf("(FRONT) launching data server %d\n", id);
        if(execv(DS_PATH, args) == -1){ /* ERRO */ }
        _Exit(-1);
    }
    return;
}

int setup_server( int port ){
    server = TCPcreate(INADDR_ANY, port);

    if (server<0){
        return -1;
    }
    if(listen(server, MAXCLIENTS)<0){
        return -1;
    }

    return server;
}

/*
    Fica à espera que alguem lhe diga para parar o servidor
    usage: exit, Exit, e, E
    TODO: should be only 'quit'
*/
void * command_handler(void *args){
    if (args == NULL){}

    char buf[BUF_LEN];
    while(!end){
        fgets(buf, BUF_LEN, stdin);
        if(!strcmp(buf, "quit\n")){
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
    if (args == NULL){}

    int ipc_client;
    struct sockaddr_un local, remote;
    char send_tok[8], recv_tok[8];
    int len, t;
    int connected;
    int aux;

    ipc_client = socket(AF_UNIX, SOCK_STREAM, 0);

    connected = 0;
    ipc_server = -1;

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    #ifdef DEBUG
        printf("(Front %d) started connection worker, attempting to be a client\n", getpid());
    #endif

    if (connect(ipc_client, (struct sockaddr *)&remote, len) != -1) {
        connected = 1;
        printf("(Front %d): front connected to data\n", getpid());
        while(connected){
            if(end==1){
                strcpy(send_tok, "EXIT\n");
                printf("(FRONT %d) Sending end token: %s\n", getpid(), send_tok);
            }
            else { strcpy(send_tok, "PING\n"); }
            /*printf("(FRONT %d) Sending token: %s\n", getpid(), send_tok);*/
            if(TCPsend(ipc_client, (uint8_t*) send_tok, strlen(send_tok)+1) == -1){ connected = 0; }
            if(connected==1){
                if(TCPrecv(ipc_client, (uint8_t*) recv_tok, 8)<0){ connected = 0; }
                /*printf("(FRONT %d) Received a token: %s\n", getpid(), recv_tok);*/
                if(connected==1){
                    if(!end){
                        if(!strcmp(recv_tok,"PING\n")){
                            sleep(1);
                        }else{
                            printf("(FRONT %d) Received a token: %s\n", getpid(), recv_tok);
                            if(sscanf(recv_tok, "%d\n", &aux) == 1){
                                if(aux >= 10000 && aux < 11000){
                                    data_server_port = aux;
                                }else{
                                 /* TODO ERRO */
                                }
                            }else{
                                /* TODO ERRO */
                            }
                        }
                    }else{
                        printf("(FRONT %d) Received a token: %s\n", getpid(), recv_tok);
                        /*implementar um counter*/
                        if(!strcmp(recv_tok,"OK\n")){
                            break;
                        }
                    }
                }
            }
        }
    }
    close(ipc_client);

    #ifdef DEBUG
        printf("(Front %d) DS connection failed\n", getpid());
    #endif

    ipc_server = socket(AF_UNIX, SOCK_STREAM, 0);

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if(bind(ipc_server, (struct sockaddr *)&local, len) == -1){ exit_gracefuly(3); }
    if(listen(ipc_server, 5) == -1){ exit_gracefuly(3); }

    while(!end){
        wakeup_data_server();

        t = sizeof(remote);
        ipc_client = accept(ipc_server, (struct sockaddr *)&remote, (socklen_t*) &t);
        if(ipc_client != -1){ connected = 1; }
        else { printf("(FRONT %d) Unable to accept\n", getpid()); }

        while(connected){
            if(TCPrecv(ipc_client, (uint8_t*) recv_tok, 8) <0){ connected = 0; }
            if(connected==1){
                /*printf("(FRONT %d) Received a token: %s\n", getpid(), recv_tok);*/
                if(!end){
                    if(!strcmp(recv_tok,"PING\n")){
                        sleep(1);
                    }else{
                        printf("(FRONT %d) Not a PING, Received a token: %s\n", getpid(), recv_tok);
                        if(sscanf(recv_tok, "%d\n", &aux) == 1){
                            if(aux >= 10000 && aux < 11000){
                                data_server_port = aux;
                            }else { /* ERRO */ }
                        } else { /* ERRO */ }
                    }
                }
                if(end==1){
                    printf("(FRONT %d) END and received a token: %s\n", getpid(), recv_tok);
                    strcpy(send_tok, "EXIT\n");
                    printf("(FRONT %d) sending: %s\n", getpid(),send_tok);
                }
                else { strcpy(send_tok, "PING\n"); }

                /*printf("(FRONT %d) Sending token: %s\n", getpid(), send_tok);*/
                if(TCPsend(ipc_client, (uint8_t*) send_tok, strlen(send_tok)+1) == -1){ connected = 0; }
            }
        }
    }
    clean_up(0);
    pthread_exit(NULL);
}

void * answer_call( void *args ){
    char buffer[128];
    struct arguments *_args = *((struct arguments **) args);
    int sock_fd = _args->sock_fd;

    sprintf(buffer, "127.0.0.1:%d\n", data_server_port);

    #ifdef DEBUG
        printf("(FRONT %d) Received connection, answering %s\n", getpid(),buffer);
    #endif

    pthread_detach(pthread_self());

    printf("\tSock_fd: %d\n\n\n", sock_fd);

    TCPsend(sock_fd, (uint8_t *) buffer, (strlen(buffer)+1)*sizeof(char));

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
