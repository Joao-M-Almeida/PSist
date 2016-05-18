#include "log.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

kv_log * create_log(char * log_path){
    #ifdef DEBUG
        printf("Creating Log\n");
    #endif
    kv_log * new_log = (kv_log *) malloc(sizeof(kv_log));
    new_log->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    new_log->log_path = (char *) malloc(sizeof(char)*(strlen(log_path)+1));
    strncpy(new_log->log_path, log_path, strlen(log_path)+1);
    pthread_mutex_init(new_log->lock, NULL);

    new_log->log_fd = fopen(log_path, "w");
    if(new_log->log_fd == NULL){
        return NULL;
    }

    return new_log;
}

/* Log should be locked */
int log_insert(kv_log * log, uint32_t key, void * to_store, char * (*to_byte_array) (Item), uint32_t (*get_size) (Item)){
    fprintf(log->log_fd, "I %u %u ", key, get_size(to_store));
    fwrite(to_byte_array(to_store), sizeof(char), get_size(to_store), log->log_fd);
    #ifdef DEBUG
        printf("Logging insert K: %u S: %u V: ", key, get_size(to_store));
        print_bytes(to_byte_array(to_store), get_size(to_store));
    #endif
    return 0;
}

/* Log should be locked */
int log_delete(kv_log * log, uint32_t key){
    fprintf(log->log_fd, "D %u", key);
    #ifdef DEBUG
        printf("Logging Delete K: %u\n", key);
    #endif
    return 0;
}

int log_lock(kv_log * log){
    return pthread_mutex_lock(log->lock);
}

int log_unlock(kv_log * log){
    return pthread_mutex_unlock(log->lock);
}

int delete_log(kv_log * log){
    fclose(log->log_fd);
    pthread_mutex_destroy(log->lock);
    remove(log->log_path);
    free(log);
    return 0;
}
