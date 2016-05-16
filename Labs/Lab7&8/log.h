#include "psiskv.h"
#include "phash-lib.h"
#include "psiskv_server.h"

#include <stdio.h>

#ifndef _LOG_H_
#define _LOG_H_

typedef struct _kv_log {
    FILE * log_fd;
    pthread_mutex_t * lock;
} kv_log;

kv_log * create_log(char *);
int log_insert(kv_log * log, uint32_t key, value_struct * to_store);
int log_delete(kv_log * log, uint32_t key);
int log_lock(kv_log * log);
int log_unlock(kv_log * log);
int close_log(kv_log * log);


#endif
