#include <stdlib.h>
#include "hash-lib.h"

hash_table create_hash(uint32_t size){
    return (hash_item**) calloc(size, sizeof( hash_item* ));
}

hash_item *create_hitem(uint32_t key, Item item){
    hash_item *new_hitem;
    new_hitem = (hash_item*) malloc(sizeof(hash_item));
    new_hitem->key = key;
    new_hitem->item = item;
    new_hitem->next = NULL;
    return new_hitem;
}

void delete_hitem(hash_item *item, void (*delete_func) (Item)){
    delete_func(item->item);
    free(item);
    return;
}

void delete_hash(hash_table hash, uint32_t size, void (*delete_func) (Item)){
    uint32_t i;
    hash_item *curr, *next;
    for(i = 0; i < size; i++){
        if(hash[i]){
            curr = hash[i];
            while(curr){
                next = curr->next;
                delete_hitem(curr, delete_func);
                curr = next;
            }
        }
    }
    free(hash);
    return;
}

uint hash_function(uint32_t key, uint32_t size){
    return (uint) (key%size);
}

Item read_item(hash_table hash, uint32_t key, uint32_t size){
    uint32_t index = hash_function(key, size);
    hash_item *aux;
    for(aux = hash[index];
        aux != NULL && aux->key != key;
        aux = aux->next);
    return aux->item;
}

bool insert_item(hash_table hash, Item item, uint32_t key, uint32_t size){
    uint32_t index = hash_function(key, size);
    hash_item *aux;
    if(!hash[index]){
        hash[index] = create_hitem(key, item);
    } else {
        for(aux = hash[index];
            aux->next != NULL && aux->key != key;
            aux = aux->next);
        if(!aux){
            aux->next = create_hitem(key, item);
        } else {
            return false;
        }
    }
    return true;
}

bool delete_item(hash_table hash, uint32_t key, uint32_t size, void (*delete_func) (Item)){
    uint32_t index = hash_function(key, size);
    hash_item *curr, *next;
    if(!hash[index]){
        return false;
    }else if(hash[index]->key == key){
        next = hash[index]->next;
        delete_hitem(hash[index], delete_func);
        hash[index] = next;
    }else{
        for(curr = hash[index], next = curr->next;
            next != NULL && next->key != key;
            curr = next, next = curr->next );
        if(next != NULL){
            curr->next = next->next;
            delete_hitem(next, delete_func);
        }else{
            return false;
        }
    }
    return true;
}
