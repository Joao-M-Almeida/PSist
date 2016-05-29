#include "TCPlib.h"
#include "phash-lib.h"
#include "inetutils.h"
#include "item.h"
#include "psiskv.h"
#include "psiskv_server.h"
#include "log.h"
#include "debug.h"
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
#define DATA_SERVER_PORT 9998
#define MAXCLIENTS 5
#define SOCK_PATH "./ipc_sock"
#define JESUS_POWER 1
#define FS_PATH "./front_server.out"
#define BACKUP_PATH "backup.data"
#define LOG_PATH "log.data"

struct arguments {
    int sock_fd;
};

extern int port;
extern int server;
extern int connected;
extern int end, ready, proper;
extern hash_table * kv_store;
extern struct arguments *args;

/* Utils */
void clean_up(int exit_val);
void exit_gracefuly(int signum);
int setup_server();
void wakeup_front_server();

/* Thread Handlers */
void * connection_worker(void *args);
void * answer_call(void *args);

/* Backup Setup */
void setup_backup();

/* SIG Handlers */
void set_int_handler();
