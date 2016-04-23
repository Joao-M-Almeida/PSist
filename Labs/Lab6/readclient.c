#include "TCPlib.h"
/*#include "hash-lib.h"*/
#include "inetutils.h"
#include "item.h"
#include "psiskv.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>


#define DEFAULTPORT 9999
int connection;


void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    kv_close(connection);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("Received signal: %d\n", signum);
    clean_up(0);
}


int main(int argc, char const *argv[]) {

    unsigned short port = DEFAULTPORT;

    /* Capture CTRL-C to exit gracefuly */
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    sigaction(SIGINT, &action, NULL);

    /*Client can receive port number as argument*/
    if(argc>1){
        port = atoi(argv[1]);
    }

    connection = kv_connect( (char *) "127.0.0.1", port);
    if (connection<0){
        perror("KVconnect");
        clean_up(-1);
    }

    char value[BUF_LEN];
    char key_char[BUF_LEN];
    printf("Connected.\n Key:\n");
    fgets(key_char, BUF_LEN, stdin);
    uint32_t key = atoi(key_char);


    int result = kv_read(connection, key, value, BUF_LEN);
    if (result<0){
        perror("KVRead");
        clean_up(-1);
    }
    printf("Read value: %s \nExiting...", value);

    clean_up(0);
}
