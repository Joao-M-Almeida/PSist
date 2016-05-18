#ifndef _HASH_LIB
#define _HASH_LIB

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "item.h"
#include "log.h"

typedef struct _hash_item {
    struct _hash_item *next;
    uint32_t key;
    /*Lock controls acesses to the Item only*/
    pthread_rwlock_t lock;
    Item item;
} hash_item;

/*typedef hash_item** hash_table;*/

typedef struct _hash_table{
    hash_item** table;
    pthread_rwlock_t** locks;
    pthread_mutex_t** log_locks;
    uint32_t size;
    kv_log * log;
} hash_table;

/*
    Create a hash table with space for size elements.
*/
hash_table * create_hash(uint32_t size, char * log_path);
void delete_hash(hash_table * hash, void (*delete_func) (Item));
uint32_t hash_function(uint32_t key, uint32_t size);
Item read_item(hash_table * hash, uint32_t key, Item (*copy_func) (Item));
int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite,
        void (*delete_func) (Item), char * (*to_byte_array) (Item),
        uint32_t (*get_size) (Item));
bool delete_item(hash_table * hash, uint32_t key, void (*delete_func) (Item));

int backup_hash(hash_table * hash, char * path , char * (*to_byte_array) (Item),
        uint32_t (*get_size) (Item));
hash_table * create_hash_from_backup(uint32_t size, char * path, char * log_path, void * (*create_func) (unsigned int ,uint8_t *), void (*delete_func) (Item), char * (*to_byte_array) (Item), uint32_t (*get_size) (Item));

#endif
