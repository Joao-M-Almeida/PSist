#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "item.h"

#ifndef _HASH_LIB
#define _HASH_LIB

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
    uint32_t size;
} hash_table;

/*
    Create a hash table with space for size elements.
*/
hash_table * create_hash(uint32_t size);
void delete_hash(hash_table * hash, void (*delete_func) (Item));
uint32_t hash_function(uint32_t key, uint32_t size);
Item read_item(hash_table * hash, uint32_t key);
int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite, void (*delete_func) (Item));
bool delete_item(hash_table * hash, uint32_t key, void (*delete_func) (Item));
#endif
