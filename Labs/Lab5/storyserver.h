
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "stdio.h"
#include "stdlib.h"

#define MESSAGE_LEN 100

typedef struct message{
    char buffer[MESSAGE_LEN];
    int last;
} message;

#define MY_SOCK_PATH "somepath"

#define CLIENT_SOCK  "client_N"

/*

Using the struct message sends a message of abitrary lenght _len_ to _to_addr_ through the DGRAM socket sock
Returns -1 on failure and the total number of bytes sent on success.

*/

int sendtowrapped(char * msg, int len,   const struct sockaddr * to_addr, int len_addr, int sock ){
    printf("Sending message: %s of %d chars\n",msg,len);
    if(len < MESSAGE_LEN){
        message m;
        strncpy(m.buffer,msg,len);
        m.last=1;
        return sendto(sock, &m, sizeof(m), 0,  (const struct sockaddr *) to_addr, len_addr);
    }else{
        char * ptr = msg;
        int len_to_send;
        message m;
        m.last = 0;
        int sent_bytes = 0;
        do{
            if(strlen(ptr)<MESSAGE_LEN){
                m.last = 1;
                len_to_send = strlen(ptr)+1;
            } else {
                m.last = 0;
                len_to_send =  MESSAGE_LEN;
            }
            strncpy(m.buffer,ptr,len_to_send);
            sent_bytes += sizeof(m);
            if(sendto(sock, &m, sizeof(m), 0,  (const struct sockaddr *) to_addr, len_addr)==-1){
                return -1;
            }
            ptr = ptr + len_to_send;
        }while(m.last!=1);
        return sent_bytes;
    }
    return -1;
}

int recvfromwrapped(char ** msg, struct sockaddr * sender_addr, unsigned int * len_sender_addr, int sock){
    message m;
    int total_recvd = sizeof(m);
    (*msg) = (char * ) malloc(sizeof(char));
    (**msg) = '\0';
    do{
        if(recvfrom(sock, &m ,sizeof(m) , 0, (struct sockaddr *) sender_addr, len_sender_addr) ==-1){
            return -1;
        }
        (*msg) = (char * ) realloc((*msg), strlen(*msg) + strlen(m.buffer) +1);
        strcat((*msg), m.buffer);
        total_recvd += sizeof(m);
    }while(m.last == 0);
    return total_recvd;
}
