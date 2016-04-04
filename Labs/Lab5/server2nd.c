/*    gcc -Wall -pedantic -ansi -std=gnu99 server2nd.c -o server.out    */

#include "storyserver.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

int sock;
char * story;

void clean_up(int exit_val){
    printf("Cleaning UP... \n");
    free(story);
    close(sock);
    unlink(MY_SOCK_PATH);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("Received signal: %d\n", signum);
    clean_up(0);
}

int main(){
    story = strdup("");
    char * read_msg;
     /* create socket  */


    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    sigaction(SIGINT, &action, NULL);

    if((sock = socket(AF_UNIX, SOCK_DGRAM,0)) == -1){
        perror("Socket Error");
        clean_up(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MY_SOCK_PATH, sizeof(addr.sun_path) - 1);

    unsigned int len_addr = sizeof(addr);

    if(bind(sock, (const struct sockaddr *) &addr,len_addr)==-1){
        perror("Socket Error");
        clean_up(-1);
    }

    int len_rcvd;

    struct sockaddr_un sender_addr;

    unsigned int len_sender_addr = sizeof(sender_addr);

    while(1){
        /* read message */

        len_rcvd = recvfromwrapped(&read_msg, (struct sockaddr *) &sender_addr, &len_sender_addr, sock);
        if(len_rcvd == -1){
            perror("Read Error");
            clean_up(-1);
        }

        printf("Received %s\n in %d bytes\n", read_msg, len_rcvd);
        /* process message */

        story = (char *) realloc(story, strlen(story) + strlen(read_msg) +1);
        strcat(story, read_msg);

        free(read_msg);

        printf("Story: %s\n", story);
        if(sendtowrapped(story,strlen(story),(const struct sockaddr *)&sender_addr,len_sender_addr,sock)==-1){
            perror("Send Error:");
            clean_up(-1);
        }
        printf("Sent Story:\n");

    }
    printf("OK\n");
    clean_up(0);
}
