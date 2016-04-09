#include <stdbool.h>
#include <stdint.h>
#include "item.h"

typedef struct _hash_item {
    struct _hash_item *next;
    uint32_t key;
    Item item;
} hash_item;

typedef hash_item** hash_table;

hash_table create_hash(uint32_t size);
void delete_hash(hash_table hash, uint32_t size, void (*delete_func) (Item));
uint32_t hash_function(uint32_t key, uint32_t size);
Item read_item(hash_table hash, uint32_t key, uint32_t size);
bool insert_item(hash_table hash, Item item, uint32_t key, uint32_t size);
bool delete_item(hash_table hash, uint32_t key, uint32_t size, void (*delete_func) (Item));
