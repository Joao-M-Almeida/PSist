#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "item.h"
#include <stdint.h>

/*
Connects IP and port to the socket referred to by the return value.
IP is in host byte order.
When failing to create a socket returns -1;
When failing to connect returns -2.
Sets errno in both cases.
*/

int TCPconnect(unsigned long IP, unsigned short port)
{
    int fd;
    struct sockaddr_in address;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1)
    {
        return -1;
    }

    /*memset() not necessary according to
    http://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html */
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(IP);
    address.sin_port = htons(port);

    /* struct sockaddr_in is the same size as struct sockaddr */
    if(connect(fd, (struct sockaddr*)&address, sizeof(address))==-1)
    {
        return -2;
    }

    return fd;
}

/*
Sends message to the socket referred to by fd.
Sends only first len bytes. returns 0 on successful send.
Returns -1 on error and sets errno.
*/
int TCPsend(int fd, uint8_t * message, unsigned int len)
{
    uint8_t * ptr = message;
    int nwritten, nleft;

    /* TODO remember to protect write() against SIGPIPE

    If you call write on a lost connection, write would return -1,
    errno will be set to EPIPE, but the system would raise a
    SIGPIPE signal and, by default, that would kill your process.

    idea: ignore SIGPIPE and then reactivate it after all writes */

    nleft = len;
    while(nleft>0)
    {
        nwritten = write(fd, ptr, nleft);
        if(nwritten<=0)
            return -1;
        nleft -= nwritten;
        ptr += nwritten;
    }

    return 0;
}

/*
Reads available content from fd and writes at most len bytes to the location starting at str.
Retuns the number of bytes written to str.
Returns -1 on unsuccessful read. retuns -2 if connection is closed by peer.
*/

int TCPrecv(int fd, uint8_t *str, unsigned int len)
{
    uint8_t *ptr = str;
    int nread;

    nread = read(fd, ptr, len);
    if(nread==-1)
        return -1;    /* error while reading */
    else if(nread==0)
        return -2;    /* connection closed by peer */

    return nread;
}

int TCPnrecv(int fd, uint8_t *str, unsigned int len){
    uint8_t *ptr = str;
    int recv, n = len;
    while(n > 0){
        recv = TCPrecv(fd, ptr, n);
        if(recv==-1){
            return -1;
        }else if(recv == -2){
            return -2;
        }
        n -= recv;
        ptr += recv;
    }
    return 0;
}

/*
Closes the file descriptor fd.
Retuns 0 on success;
Returns -1 on error and sets errno.
*/
int TCPclose(int fd)
{
    return close(fd);
}

/*
Binds IP and port to the returned fd.
When failing to create a socket returns -1;
When failing to bind IP and port to a fd returns -2.
Sets errno in both cases.
*/
int TCPcreate(unsigned long IP, unsigned short port)
{
    int fd;
    struct sockaddr_in address;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1)
    {
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(IP);
    address.sin_port = htons(port);

    /* struct sockaddr_in is the same size as struct sockaddr */
    if(bind(fd, (struct sockaddr*)&address, sizeof(address))==-1)
    {
        return -2;
    }

    return fd;
}

/*
Accepts an incoming connection on the listening port associated to fd.
In case of success returns the fd associated to the incoming connection.
Returns -1 otherwise
*/

int TCPaccept(int fd)
{
    int fd_accept;
    struct sockaddr address;
    socklen_t addrlen = sizeof(address);

    fd_accept = accept(fd, &address, &addrlen);
    if(fd_accept == -1)
    {
        return -1;
    }

    return fd_accept;
}
