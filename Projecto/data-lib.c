#include "data-lib.h"

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

int port;
int server;
int ipc_server;
int end;
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

void wakeup_front_server(){
    char *args[] = { (char *) FS_PATH,
                    NULL};
    int id = fork();
    if(id==0){
        if(server != -1) { close(server); }
        if(ipc_server != -1) { close(ipc_server); }
        printf("(DATA) Launching Front server %d\n", id);
        execv(FS_PATH, args);
        _Exit(-1);
    }
    return;
}

int setup_server(){
    port = 9999;

    do {
        port++;
        server = TCPcreate(INADDR_ANY, port);
    } while(server<0 && port < 11000);
    if(port==11000){
        clean_up(-1);
    }

    if (server < 0 || listen(server, MAXCLIENTS) < 0) { clean_up(-1); }

    return server;
}

void * connection_worker( void *args ){
    if(args==NULL){}

    int ipc_client;
    struct sockaddr_un local, remote;
    char send_tok[8], recv_tok[8];
    int len, t;
    int connected, resend;

    connected = 0;
    resend = 1;

    ipc_client = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    #ifdef DEBUG
        printf("(Data %d) started FS Puller, attempting to be a client\n", getpid());
    #endif

    if (connect(ipc_client, (struct sockaddr *)&remote, len) != -1) {
        connected = 1;
        printf("data connected to front\n");
        while(connected){
            if(end==1){ strcpy(send_tok, "OK\n"); }
            else {
                if(resend==1){
                    while(port==-1);
                    sprintf(send_tok, "%d\n", port);
                    resend = 0;
                } else { strcpy(send_tok, "PING\n"); }
            }
            /*printf("(DATA %d) Sending a token: %s\n", getpid(), send_tok);*/
            if(TCPsend(ipc_client, (uint8_t*) send_tok, strlen(send_tok)) == -1){ connected = 0; }
            if(connected==1){
                if(end || TCPrecv(ipc_client, (uint8_t*) recv_tok, 8) == -1){ connected = 0; }
                /*printf("(DATA %d) Received a token: %s\n", getpid(), recv_tok);*/
                if(connected==1){
                    if(!strcmp(recv_tok,"PING\n")){ sleep(1); }
                    else if(!strcmp(recv_tok,"EXIT\n")){ end = 1; }
                    else { /* ERRO */ }
                }
            }
        }
    }
    close(ipc_client);

    #ifdef DEBUG
        printf("(Data %d) FS connection failed\n", getpid());
    #endif

    ipc_server = socket(AF_UNIX, SOCK_STREAM, 0);

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);

    if(bind(ipc_server, (struct sockaddr *)&local, len) == -1){ exit_gracefuly(3); }
    if(listen(ipc_server, 5) == -1){ exit_gracefuly(3); }

    while(!end){
        resend = 1;
        wakeup_front_server();

        t = sizeof(remote);
        ipc_client = accept(ipc_server, (struct sockaddr *)&remote, (socklen_t*) &t);

        if(ipc_client != -1){ connected = 1;
        } else { printf("(DATA %d) Unable to accept\n", getpid()); }

        while(connected){
            if(TCPrecv(ipc_client, (uint8_t*) recv_tok, 8) == -1){ connected = 0; }
            if(connected==1){
                /*printf("(DATA %d) Received token: %s\n", getpid(), recv_tok);*/
                if(!strcmp(recv_tok,"PING\n")){ sleep(1); }
                else if(!strcmp(recv_tok,"EXIT\n")){ end = 1; }
                else { /* ERRO */ }

                if(end==1){ strcpy(send_tok, "OK\n"); }
                else {
                    if(resend == 1){
                        while(port==-1);
                        sprintf(send_tok, "%d\n", port);
                        resend = 0;
                    } else { strcpy(send_tok, "PING\n"); }
                }
                /*printf("(DATA %d) Sending a token: %s\n", getpid(), send_tok);*/
                if(TCPsend(ipc_client, (uint8_t*) send_tok, strlen(send_tok)) == -1){ connected = 0; }
            }
        }
    }

    pthread_exit(NULL);
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

void setup_backup() {
    FILE * aux = fopen(BACKUP_PATH, "r");
    /*Check if Backup exists and Create Hash Table */
    if(aux==NULL){
        printf("No Backup found... Starting from scratch\n");
        aux = fopen(LOG_PATH, "r");
        if(aux==NULL){
            printf("No Log found...\n");
            kv_store = create_hash(STORESIZE, (char *) LOG_PATH, create_struct, destroy_struct, struct_to_str, struct_get_size);
        } else {
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
    } else {
        fclose(aux);
        /*backup exists, check if log exists*/
        aux = fopen(LOG_PATH, "r");
        if(aux==NULL){
            printf("Backup found and no Log found...\n");
            /*Log doesn't exist*/
            kv_store = create_hash_from_backup(STORESIZE, (char *) BACKUP_PATH, (char *) LOG_PATH, create_struct, destroy_struct, struct_to_str, struct_get_size);
        } else {
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
        //criar erro para isto
        clean_up(0);
    }
    return;
}

/* Capture CTRL-C to exit gracefuly */
void set_int_handler(){
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
}
