#include <stdlib.h>
#include <stdio.h>
#include "phash-lib.h"
#include "debug.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
/*
    Create Hash table, allocating the memory for the lists and the list mutexes, initializing the mutexes
*/
hash_table * create_hash(uint32_t size){
    hash_table * hash = (hash_table *) malloc(sizeof(hash_table));
    hash->table = (hash_item**) calloc(size, sizeof( hash_item* ));
    hash->size = size;
    hash->locks = (pthread_rwlock_t**) malloc(sizeof(pthread_rwlock_t*)*size);
    for(unsigned int i = 0; i < size; i++){
        hash->locks[i] = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init(hash->locks[i],NULL);
    }
    return hash;
}

/*
    Create the hash_item and init the mutex
*/

hash_item *create_hitem(uint32_t key, Item item){
    hash_item *new_hitem;
    new_hitem = (hash_item*) malloc(sizeof(hash_item));
    new_hitem->key = key;
    pthread_rwlock_init(&new_hitem->lock, NULL); /*TODO: is this needed*/
    new_hitem->item = item;
    new_hitem->next = NULL;
    return new_hitem;
}

/*
    Delete the hash_item->item, the mutex and the hash_item
*/
void delete_hitem(hash_item *item, void (*delete_func) (Item)){
    /*TODO: do we need to lock on deletion?*/
    /*pthread_rwlock_wrlock(&item->lock);*/
    delete_func(item->item);
    /*pthread_rwlock_unlock(&item->lock);*/

    /*TODO: We have to delete the rwlock?*/
    pthread_rwlock_destroy(&item->lock);

    free(item);
    return;
}

/*
Delete the entire hash table.
First each item, then the locks then the hash
*/
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

/*
    Hash function
    TODO: turn this into a MACRO?
*/
uint hash_function(uint32_t key, uint32_t size){
    return (uint) (key%size);
}

/*
    Find the item and then if it exists return the pointer to it.
*/
Item read_item(hash_table * hash, uint32_t key,  Item (*copy_func) (Item)){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux_hitem;
    Item item;

    pthread_rwlock_rdlock(hash->locks[index]);
    for(aux_hitem = hash->table[index];
        aux_hitem != NULL && aux_hitem->key != key;
        aux_hitem = aux_hitem->next);

    if(aux_hitem != NULL){
        /* Return copy of item instead of a pointer to it */
        item = copy_func(aux_hitem->item);
        pthread_rwlock_unlock(hash->locks[index]);
        return item;
    }
    pthread_rwlock_unlock(hash->locks[index]);
    return NULL;
}

/*
    Find if item already exists, overwrite if specified.
    If it doesn't exist insert the new item
    TODO: optimize critical sections: for instance remove create item from critical section and item deletion (in overwrite)
*/
int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite, void (*delete_func) (Item)){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux;
    #ifdef DEBUG
        printf("Insert trying to lock\n");
    #endif
    pthread_rwlock_wrlock(hash->locks[index]);
    #ifdef DEBUG
        printf("Insert locked\n");
    #endif
    if(!hash->table[index]){
        /*No item in this index*/
        #ifdef DEBUG
            printf("Inserting Item at the begining of empty list\n");
        #endif
        hash->table[index] = create_hitem(key, item);
        pthread_rwlock_unlock(hash->locks[index]);
        #ifdef DEBUG
            printf("Insert unlocked\n");
        #endif
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
                    printf("Searching... Found Item with key: %d\n", aux->key);
                #endif
        }
        #ifdef DEBUG
            printf("Stopped at Item with key: %d\n", aux->key);
        #endif
        /* Check if found item or not*/
        if(aux->key != key){
            /*didn't find, so is at end of list*/
            aux->next = create_hitem(key, item);
            #ifdef DEBUG
                printf("Inserting Item at end of list\n");
            #endif
        } else {
            #ifdef DEBUG
                printf("Item with key %d already exists; overwrite: %d\n", key, overwrite);
            #endif
            if(overwrite){
                #ifdef DEBUG
                    printf("Overwriting item");
                #endif
                /*pthread_rwlock_wrlock(&aux->lock);*/
                /*TODO: check if correct*/
                delete_func(aux->item);
                aux->item = item;
                /*pthread_rwlock_unlock(&aux->lock);*/
            }else{
                pthread_rwlock_unlock(hash->locks[index]);
                /*Item already exists*/
                return 1;
            }
        }
        pthread_rwlock_unlock(hash->locks[index]);
    }
    return 0;
}

/*
    Find item, if it exists delete it
    TODO: optimize critical sections
*/
bool delete_item(hash_table * hash, uint32_t key, void (*delete_func) (Item)){
    uint32_t index = hash_function(key, hash->size);
    hash_item *curr, *next;

    pthread_rwlock_wrlock(hash->locks[index]);
    if(!hash->table[index]){
        /*No item on that index*/
        pthread_rwlock_unlock(hash->locks[index]);
        return false;
    }else if(hash->table[index]->key == key){
        next = hash->table[index]->next;
        delete_hitem(hash->table[index], delete_func);
        hash->table[index] = next;
    }else{
        for(curr = hash->table[index], next = curr->next;
            next != NULL && next->key != key;
            curr = next, next = curr->next );
        if(next != NULL){
            curr->next = next->next;
            delete_hitem(next, delete_func);
        }else{
            /*Item not present*/
            pthread_rwlock_unlock(hash->locks[index]);
            return false;
        }
    }
    pthread_rwlock_unlock(hash->locks[index]);
    return true;
}

/*First version of backup, inefficient*/
int backup_hash(hash_table * hash, char * path, char * (*to_str) (Item), uint32_t (*get_size) (Item) ){
    hash_item * aux;
    FILE * backup = fopen(path, "w");
    char * str_aux;
    if(backup == NULL){
        return -1;
    }
    #ifdef DEBUG
        printf("Starting Backup\n");
    #endif
    for(unsigned int i; i<hash->size; i++){
        if(hash->table[i] != NULL){
            aux = hash->table[i];
            do {
                str_aux = to_str(aux->item);
                fprintf(backup, "%u %u ", aux->key, get_size(aux->item));
                fwrite(str_aux,sizeof(char),get_size(aux->item),backup);
                #ifdef DEBUG
                    printf("K: %u, S:%u, V:", aux->key, get_size(aux->item));
                    print_bytes(str_aux,  get_size(aux->item));
                #endif
                free(str_aux);
                aux = aux->next;
            } while(aux!= NULL);
        }
    }
    #ifdef DEBUG
        printf("Finished Backup\n");
    #endif
    fclose(backup);
    return 0;
}


/*
    Create Hash table, allocating the memory for the lists and the list mutexes, initializing the mutexes
    and then initialize it from backup
*/
hash_table * create_hash_from_backup(uint32_t size, char * path, void * (*create_func) (unsigned int ,uint8_t *), void (*delete_func) (Item)){
    hash_table * hash = (hash_table *) malloc(sizeof(hash_table));
    hash->table = (hash_item**) calloc(size, sizeof( hash_item* ));
    hash->size = size;
    hash->locks = (pthread_rwlock_t**) malloc(sizeof(pthread_rwlock_t*)*size);
    for(unsigned int i = 0; i < size; i++){
        hash->locks[i] = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init(hash->locks[i],NULL);
    }

    int backup = open(path,0, "r");
    if(backup == -1){
        return NULL;
    }
    #ifdef DEBUG
        printf("Reading Backup\n");
    #endif

    char * buf = (char *) malloc(sizeof(char)*(1024+1));
    char * buf2 = (char *) malloc(sizeof(char)*(1024+1));
    char * aux;
    char * aux2;
    char * aux3;
    uint32_t key;
    uint32_t val_size;
    Item item_aux;
    int l;
    int k;
    k = read(backup, buf, 1024);
    buf[k]='\0'; /*Needed?*/
    aux = buf;
    while(1){
        while(sscanf(aux,"%u %u %n",&key, &val_size, &l)>=2){
            aux+=l;
            #ifdef DEBUG
                printf("K: %d, S: %d\n", key, val_size);
            #endif
            if(buf+1024-aux >= val_size){
                aux2 = (char *) malloc(sizeof(char)*(val_size+1));
                strncpy(aux2, aux, val_size);
                aux2[val_size]='\0';
                item_aux = create_func(val_size, (uint8_t *) aux2);
                insert_item(hash, item_aux, key, 0, delete_func);
                #ifdef DEBUG
                    printf("K: %d, V:", key);
                    print_bytes(aux2,  val_size);
                #endif
                aux+=val_size;
            }else{
                aux2 = (char *) malloc(sizeof(char)*(val_size+1));
                strncpy(aux2, aux, buf+1024-aux);
                aux3 = aux2 + (buf + 1024 - aux);
                read(backup, aux3, val_size-(buf+1024-aux));
                aux2[val_size]='\0';
                item_aux =create_func(val_size, (uint8_t *) aux2);
                insert_item(hash, item_aux, key, 0, delete_func);
                #ifdef DEBUG
                    printf("K: %d, V:", key);
                    print_bytes(aux2,  val_size);
                #endif
                k = read(backup, buf, 1024);
                buf[k]='\0'; /*Needed?*/
                aux = buf;
            }
        }
        /*Couldn't read a key and a size*/
        if(aux!=buf){
            /* there is more to read*/
            l=aux-buf;
            strncpy(buf2,aux,l);
            aux2 = buf2+l;
            k = read(backup, aux2, 1024-l);
            if(k==0){
                break;
            }
            aux3 = buf2;
            buf2 = buf;
            buf = aux3;

            buf[1024]='\0'; /*Needed?*/
            aux = buf;
        }else{
            /*if aux==buf then there is nothing else to read*/
            break;
        }
    }
    free(buf);
    free(buf2);
    #ifdef DEBUG
        printf("Finished Reading Backup\n");
    #endif
    close(backup);
    return hash;
}
