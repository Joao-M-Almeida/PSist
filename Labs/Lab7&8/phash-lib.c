#include <stdlib.h>
#include <stdio.h>
#include "phash-lib.h"

hash_table create_hash(uint32_t size){
    return (hash_item**) calloc(size, sizeof( hash_item* ));
}

hash_item *create_hitem(uint32_t key, Item item){
    hash_item *new_hitem;
    new_hitem = (hash_item*) malloc(sizeof(hash_item));
    new_hitem->key = key;
    pthread_rwlock_init(&new_hitem->lock, NULL);
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
    hash_item *aux_hitem;
    Item item;

    for(aux_hitem = hash[index];
        aux_hitem != NULL && aux_hitem->key != key;
        aux_hitem = aux_hitem->next);
    if(aux_hitem != NULL){
        pthread_rwlock_rdlock(&aux_hitem->lock);
        item = aux_hitem->item; /* TODO: função para copiar */
        pthread_rwlock_unlock(&aux_hitem->lock);
        return item;
    }
    return NULL;
}

int insert_item(hash_table hash, Item item, uint32_t key, uint32_t size, int overwrite){
    uint32_t index = hash_function(key, size);
    hash_item *aux;
    if(!hash[index]){
        printf("Insert in the beggining\n");
        hash[index] = create_hitem(key, item);
    } else {
        if( hash[index]->next == NULL){
            printf("item in list with key: %d\n", hash[index]->key);
        }
        for(aux = hash[index];
            aux->next != NULL && aux->key != key;
            aux = aux->next){
                printf("item in list with key: %d\n", aux->key);
        }
        if(!aux->next){
            aux->next = create_hitem(key, item);
        } else {
            printf("Item with key %d already exists with value %s; overwrite: %d\n", key, (char *)aux->item, overwrite);
            if(overwrite){
                pthread_rwlock_wrlock(&aux->lock);
                /* TODO: delete e rewrite */
                pthread_rwlock_unlock(&aux->lock);
            }else{
                return -2;
            }
        }
    }
    return 0;
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
