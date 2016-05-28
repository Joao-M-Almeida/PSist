#include "TCPlib.h"
#include "inetutils.h"
#include "item.h"
#include "debug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define STORESIZE 11
#define FRONT_SERVER_PORT 9999
#define MAXCLIENTS 5
#define SOCK_PATH "./ipc_sock"
#define JESUS_POWER 1
#define DS_PATH "./data_server.out"
#define BUF_LEN 32

struct arguments {
    int sock_fd;
};

extern int port;
extern int server;
extern int connected;
/*extern int interrupt;*/
extern int end, stop, ready, proper;
extern int data_server_port;
extern struct arguments *args;

/* Utils */
void clean_up(int exit_val);
void exit_gracefuly(int signum);
void wakeup_data_server();
int setup_server(int port);

/* Thread Handlers */
void * command_handler(void *args);
void * connection_worker(void *args);
void * answer_call(void *args);

/* SIG Handlers
On INT exits gracefuly */
void set_int_handler();
