/*    gcc -Wall -pedantic -ansi -std=gnu99 client2nd.c -o client.out    */


#include "storyserver.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>


int sock;
char my_path[100];

void clean_up(int exit_val) {
    printf("Cleaning UP... \n");
    close(sock);
    unlink(my_path);
    exit(exit_val);
}

void exit_gracefuly(int signum){
    printf("Received signal: %d\n", signum);
    clean_up(0);
}

int main(){
    char temp_msg[300];

    int my_id = 0;
    struct sigaction action;
    action.sa_handler = exit_gracefuly;
    sigaction(SIGINT, &action, NULL);

    if((sock = socket(AF_UNIX, SOCK_DGRAM,0)) == -1){
        perror("Socket Error");
        exit(-1);
    }

    struct sockaddr_un addr;
    int bind_sucess = 0;
    unsigned int len_addr;
    do{
        sprintf(my_path, "%s%d",CLIENT_SOCK,my_id);
        memset(&addr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, my_path, sizeof(addr.sun_path) - 1);
        len_addr = sizeof(addr);
        if(bind(sock, (const struct sockaddr *) &addr,len_addr)==-1){
            if(errno == EADDRINUSE){
                my_id++;
            }else{
                perror("Bind Error");
                printf("Errno = %d \n", errno);
                clean_up(-1);
            }
        }else{
            bind_sucess = 1;
        }
    }while (!bind_sucess);

    printf("My path is: (%s) send message: ",my_path);
    fgets(temp_msg, 300, stdin);

    struct sockaddr_un dest_addr;
    memset(&dest_addr, 0, sizeof(struct sockaddr_un)); /* Clear structure */
    dest_addr.sun_family = AF_UNIX;
    strncpy(dest_addr.sun_path, MY_SOCK_PATH, sizeof(dest_addr.sun_path) - 1);
    len_addr = sizeof(dest_addr);


    /* write message */
    /*int l = sendto(sock, &m, sizeof(m), 0, (const struct sockaddr *) &dest_addr, len_addr);*/
    int l = sendtowrapped(temp_msg, strlen(temp_msg),  (const struct sockaddr *) &dest_addr, len_addr, sock);
    if (l ==-1){
        perror("Send Error");
        clean_up(-1);
    }
    printf("Sent %d bytes\n", l);

    /* receive story */
    int len_rcvd;
    char * read_msg;
    struct sockaddr_un sender_addr;
    unsigned int len_sender_addr = sizeof(sender_addr);

    len_rcvd = recvfromwrapped(&read_msg, (struct sockaddr *) &sender_addr, &len_sender_addr, sock);
    if(len_rcvd == -1){
        perror("Read Error");
        clean_up(-1);
    }
    printf("Story: %s\n",read_msg );
    free(read_msg);
    printf("OK\n");
    clean_up(0);

}
