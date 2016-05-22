#ifndef _LOG_H_
#define _LOG_H_

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "item.h"

typedef struct _kv_log {
    FILE * log_fd;
    /*pthread_mutex_t * lock;*/
    char * log_path;
} kv_log;

kv_log * create_log(char *);
int log_insert(kv_log * log, uint32_t key, void * to_store, char * (*to_byte_array) (Item), uint32_t (*get_size) (Item));
int log_delete(kv_log * log, uint32_t key);
/*int log_lock(kv_log * log);
int log_unlock(kv_log * log);*/
int delete_log(kv_log * log);
int rename_log(kv_log * log, char * new_path);

#endif
