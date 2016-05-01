#include <stdlib.h>
#include <stdio.h>
#include "phash-lib.h"

/*hash_table create_hash(uint32_t size){
    return (hash_item**) calloc(size, sizeof( hash_item* ));
}*/

hash_table * create_hash(uint32_t size){
    hash_table * hash = (hash_table *) malloc(sizeof(hash_table));
    hash->table = (hash_item**) calloc(size, sizeof( hash_item* ));
    hash->size = size;
    hash->locks =   (pthread_rwlock_t**) malloc(sizeof(pthread_rwlock_t*)*size);
    for(unsigned int i = 0; i < size; i++){
        hash->locks[i] = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init(hash->locks[i],NULL);
    }
    return hash;
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
    /*TODO: do we need to lock on deletion?*/
    pthread_rwlock_wrlock(&item->lock);
    delete_func(item->item);
    pthread_rwlock_unlock(&item->lock);

    /*TODO: We have to delete the rwlock?*/

    free(item);
    return;
}

void delete_hash(hash_table * hash, void (*delete_func) (Item)){
    uint32_t i;
    hash_item *curr, *next;
    for(i = 0; i < hash->size; i++){
        if(hash->table[i]){
            curr = hash->table[i];
            /*TODO: lock hash and delete lock?*/
            while(curr){
                next = curr->next;
                delete_hitem(curr, delete_func);
                curr = next;
            }

        }
        pthread_rwlock_destroy(hash->locks[i]);
        free(hash->locks[i]);
    }
    free(hash->locks);
    free(hash->table);
    free(hash);
    return;
}

uint hash_function(uint32_t key, uint32_t size){
    return (uint) (key%size);
}

Item read_item(hash_table * hash, uint32_t key){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux_hitem;
    Item item;

    /*TODO: lock for read hashlock*/
    for(aux_hitem = hash->table[index];
        aux_hitem != NULL && aux_hitem->key != key;
        aux_hitem = aux_hitem->next);

    if(aux_hitem != NULL){
        pthread_rwlock_rdlock(&aux_hitem->lock);
        /* TODO: função para copiar
            Return copy of item instead of a pointer to it
        */
        item = aux_hitem->item;
        pthread_rwlock_unlock(&aux_hitem->lock);
        return item;
    }
    return NULL;
}

int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite, void (*delete_func) (Item)){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux;
    if(!hash->table[index]){
        /*No item in this index*/
        #ifdef DEBUG
            printf("Inserting Item at the begining of empty list\n");
        #endif
        hash->table[index] = create_hitem(key, item);
    } else {
        if( hash->table[index]->next == NULL){
            /*Useless if, list has only one element*/
            #ifdef DEBUG
                printf("Only item in list with key: %d\n", hash->table[index]->key);
            #endif
        }
        /*TODO: lock for write hashlock*/
        for(aux = hash->table[index];
            aux->next != NULL && aux->key != key;
            aux = aux->next){
                /*search list untill end or untill finding key*/
                #ifdef DEBUG
                    printf("item in list with key: %d\n", aux->key);
                #endif
        }
        if(!aux->next){
            /*End of list reached insert item at the end*/
            aux->next = create_hitem(key, item);
            #ifdef DEBUG
                printf("Inserting Item at end of list");
            #endif
        } else {
            #ifdef DEBUG
                printf("Item with key %d already exists with value %s; overwrite: %d\n", key, (char *)aux->item, overwrite);
            #endif
            if(overwrite){
                #ifdef DEBUG
                    printf("Overwriting item");
                #endif
                pthread_rwlock_wrlock(&aux->lock);
                /*TODO: check if correct*/
                delete_func(aux->item);
                aux->item = item;
                pthread_rwlock_unlock(&aux->lock);
            }else{
                return -2;
            }
        }
    }
    return 0;
}

bool delete_item(hash_table * hash, uint32_t key, void (*delete_func) (Item)){
    uint32_t index = hash_function(key, hash->size);
    hash_item *curr, *next;
    if(!hash->table[index]){
        /*No item on that index*/
        return false;
    }else if(hash->table[index]->key == key){
        next = hash->table[index]->next;
        delete_hitem(hash->table[index], delete_func);
        hash->table[index] = next;
    }else{
        /*TODO: lock for write hashlock*/
        for(curr = hash->table[index], next = curr->next;
            next != NULL && next->key != key;
            curr = next, next = curr->next );
        if(next != NULL){
            curr->next = next->next;
            delete_hitem(next, delete_func);
        }else{
            /*Item not present*/
            return false;
        }
    }
    return true;
}
