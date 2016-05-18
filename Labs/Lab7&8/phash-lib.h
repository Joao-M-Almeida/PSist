#ifndef _HASH_LIB
#define _HASH_LIB

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "item.h"
#include "log.h"

typedef void * (*create_func) (unsigned int ,uint8_t *);
typedef void (*delete_func) (Item);
typedef char * (*to_byte_array) (Item);
typedef uint32_t (*get_size) (Item);


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
    create_func item_create;
    delete_func item_delete;
    to_byte_array item_to_byte_array;
    get_size item_get_size;
} hash_table;

/*
    Create a hash table with space for size elements.
*/
hash_table * create_hash(uint32_t size, char * log_path, create_func new_create_func, delete_func new_delete_func, to_byte_array new_to_byte_array, get_size new_get_size);
void delete_hash(hash_table * hash);
uint32_t hash_function(uint32_t key, uint32_t size);
Item read_item(hash_table * hash, uint32_t key);
int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite);
bool delete_item(hash_table * hash, uint32_t key);

int backup_hash(hash_table * hash, char * path);
hash_table * create_hash_from_backup(uint32_t size, char * path, char * log_path, create_func new_create_func, delete_func new_delete_func, to_byte_array new_to_byte_array, get_size new_get_size);

#endif
