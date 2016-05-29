#include "phash-lib.h"
#include "debug.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/*
    Create Hash table, allocating the memory for the lists and the list mutexes, initializing the mutexes
*/

hash_table * create_hash(uint32_t size, char * log_path, create_func new_create_func, delete_func new_delete_func, to_byte_array new_to_byte_array, get_size new_get_size){
    hash_table * hash = (hash_table *) malloc(sizeof(hash_table));
    hash->table = (hash_item**) calloc(size, sizeof( hash_item* ));
    hash->size = size;

    hash->item_create = new_create_func;
    hash->item_delete = new_delete_func;
    hash->item_to_byte_array = new_to_byte_array;
    hash->item_get_size = new_get_size;

    hash->locks = (pthread_rwlock_t**) malloc(sizeof(pthread_rwlock_t*)*size);
    for(unsigned int i = 0; i < size; i++){
        hash->locks[i] = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init(hash->locks[i],NULL);
    }
    hash->log_locks = (pthread_mutex_t**) malloc(sizeof(pthread_mutex_t*)*size);
    for(unsigned int i = 0; i < size; i++){
        hash->log_locks[i] = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(hash->log_locks[i],NULL);
    }
    hash->log = create_log(log_path);
    return hash;
}

/*
    Create the hash_item and init the mutex
*/

hash_item *create_hitem(uint32_t key, Item item){
    hash_item *new_hitem;
    new_hitem = (hash_item*) malloc(sizeof(hash_item));
    new_hitem->key = key;
    new_hitem->item = item;
    new_hitem->next = NULL;
    return new_hitem;
}

/*
    Delete the hash_item->item, the mutex and the hash_item
*/
void delete_hitem(hash_item *item, void (*delete_func) (Item)){
    delete_func(item->item);
    free(item);
    return;
}

/*
Delete the entire hash table.
First each item, then the locks then the hash
*/
void delete_hash(hash_table * hash){
    uint32_t i;
    hash_item *curr, *next;
    for(i = 0; i < hash->size; i++){
        if(hash->table[i]){
            curr = hash->table[i];
            /*TODO: lock hash and delete lock?*/
            while(curr){
                next = curr->next;
                delete_hitem(curr, hash->item_delete);
                curr = next;
            }
        }
        pthread_rwlock_destroy(hash->locks[i]);
        pthread_mutex_destroy(hash->log_locks[i]);
        free(hash->log_locks[i]);
        free(hash->locks[i]);
    }
    delete_log(hash->log);
    free(hash->locks);
    free(hash->log_locks);
    free(hash->table);
    free(hash);
    return;
}

/*
    Hash function
*/
uint hash_function(uint32_t key, uint32_t size){
    return (uint) (key%size);
}

/*
    Find the item and then if it exists return the pointer to it.
*/
Item read_item(hash_table * hash, uint32_t key){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux_hitem;
    Item item;

    pthread_rwlock_rdlock(hash->locks[index]);
    for(aux_hitem = hash->table[index];
        aux_hitem != NULL && aux_hitem->key != key;
        aux_hitem = aux_hitem->next);

    if(aux_hitem != NULL){
        /* Return copy of item instead of a pointer to it */
        /*TODO: is doing an extra copy of the value,
        should have a specific function to get pointer*/
        item = hash->item_create(hash->item_get_size(aux_hitem->item), (uint8_t *) hash->item_to_byte_array(aux_hitem->item));
        pthread_rwlock_unlock(hash->locks[index]);
        return item;
    }
    pthread_rwlock_unlock(hash->locks[index]);
    return NULL;
}

/*
    Find if item already exists, overwrite if specified.
    If it doesn't exist insert the new item
*/
int insert_item(hash_table * hash, Item item, uint32_t key, int overwrite, int logging){
    uint32_t index = hash_function(key, hash->size);
    hash_item *aux;
    hash_item * to_insert = create_hitem(key, item);
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
        hash->table[index] = to_insert;
        if(logging){
            pthread_mutex_lock(hash->log_locks[index]);
        }
        pthread_rwlock_unlock(hash->locks[index]);
        if(logging){
            log_insert(hash->log,key, item, hash->item_to_byte_array, hash->item_get_size);
            pthread_mutex_unlock(hash->log_locks[index]);
        }
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
            aux->next = to_insert;
            #ifdef DEBUG
                printf("Inserting Item at end of list\n");
            #endif

            if(logging){
                pthread_mutex_lock(hash->log_locks[index]);
            }
            pthread_rwlock_unlock(hash->locks[index]);
            if(logging){
                log_insert(hash->log,key, item, hash->item_to_byte_array, hash->item_get_size);
                pthread_mutex_unlock(hash->log_locks[index]);
            }

        } else {
            #ifdef DEBUG
                printf("Item with key %d already exists; overwrite: %d\n", key, overwrite);
            #endif
            if(overwrite){
                #ifdef DEBUG
                    printf("Overwriting item");
                #endif
                Item to_delete = aux->item;
                /*hash->item_delete(aux->item);*/
                aux->item = item;

                if(logging){
                    pthread_mutex_lock(hash->log_locks[index]);
                }
                pthread_rwlock_unlock(hash->locks[index]);
                if(logging){
                    log_insert(hash->log,key, item, hash->item_to_byte_array, hash->item_get_size);
                    pthread_mutex_unlock(hash->log_locks[index]);
                }
                hash->item_delete(to_delete);
            }else{
                pthread_rwlock_unlock(hash->locks[index]);
                /*Item already exists*/
                return 1;
            }
        delete_hitem(to_insert, hash->item_delete);
        }
    }
    return 0;
}

/*
    Find item, if it exists delete it
    TODO: optimize critical sections
*/
bool delete_item(hash_table * hash, uint32_t key, int logging){
    uint32_t index = hash_function(key, hash->size);
    hash_item *curr, *next, *to_delete;

    pthread_rwlock_wrlock(hash->locks[index]);

    if(!hash->table[index]){
        /*No item on that index*/
        pthread_rwlock_unlock(hash->locks[index]);
        return false;
    }else if(hash->table[index]->key == key){
        next = hash->table[index]->next;
        to_delete = hash->table[index];
        hash->table[index] = next;
        if(logging){
            pthread_mutex_lock(hash->log_locks[index]);
        }
        pthread_rwlock_unlock(hash->locks[index]);
        if(logging){
            log_delete(hash->log, key);
            pthread_mutex_unlock(hash->log_locks[index]);
        }
    }else{
        for(curr = hash->table[index], next = curr->next;
            next != NULL && next->key != key;
            curr = next, next = curr->next );
        if(next != NULL){
            curr->next = next->next;
            to_delete = next;
            if(logging){
                pthread_mutex_lock(hash->log_locks[index]);
            }
            pthread_rwlock_unlock(hash->locks[index]);
            if(logging){
                log_delete(hash->log, key);
                pthread_mutex_unlock(hash->log_locks[index]);
            }
        }else{
            /*Item not present*/
            pthread_rwlock_unlock(hash->locks[index]);
            return false;
        }
    }
    delete_hitem(to_delete, hash->item_delete);
    return true;
}

int backup_hash(hash_table * hash, char * path){
    hash_item * aux;
    char temp_path[1024];
    strcpy(temp_path, path);
    strcat(temp_path, ".temp");
    FILE * backup = fopen(temp_path, "w");
    char * str_aux;
    if(backup == NULL){
        return -1;
    }
    #ifdef DEBUG
        printf("Starting Backup\n");
    #endif
    for(unsigned int i = 0; i<hash->size; i++){
        if(hash->table[i] != NULL){
            aux = hash->table[i];
            do {
                str_aux = hash->item_to_byte_array(aux->item);
                fprintf(backup, "%u %u ", aux->key, hash->item_get_size(aux->item));
                fwrite(str_aux,sizeof(char),hash->item_get_size(aux->item),backup);
                #ifdef DEBUG
                    printf("K: %u, S:%u, V:", aux->key, hash->item_get_size(aux->item));
                    print_bytes(str_aux,  hash->item_get_size(aux->item));
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
    /*Check if backup exists and replace it*/
    FILE * old_backup;
    old_backup = fopen(path,"r");
    if(old_backup != NULL){
        fclose(old_backup);
        remove(path);
    }
    rename(temp_path, path);

    return 0;
}


/*
    Create Hash table, allocating the memory for the lists and the list mutexes, initializing the mutexes
    and then initialize it from backup
*/
hash_table * create_hash_from_backup(uint32_t size, char * path, char * log_path,
    create_func new_create_func, delete_func new_delete_func, to_byte_array new_to_byte_array, get_size new_get_size){

    hash_table * hash = create_hash(size,log_path, new_create_func, new_delete_func, new_to_byte_array, new_get_size);

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
    #ifdef DEBUG
        printf("Buffer:");
        print_bytes(buf,  k);
    #endif
    aux = buf;
    char space;
    while(1){
        /*TODO: replace ' ' with %c*/
        while(sscanf(aux,"%u %u%c%n",&key, &val_size, &space, &l)>=3){
            if(space!=' '){
                break;
                #ifdef DEBUG
                    printf("Error");
                #endif
            }
            aux+=l;
            #ifdef DEBUG
                printf("K: %d, S: %d\n", key, val_size);
            #endif
            if(buf+1024-aux >= val_size){
                aux2 = (char *) malloc(sizeof(char)*(val_size+1));
                memcpy(aux2, aux, val_size);
                aux2[val_size]='\0';
                item_aux = hash->item_create(val_size, (uint8_t *) aux2);
                insert_item(hash, item_aux, key, 0,0);
                #ifdef DEBUG
                    printf("K: %d, V:", key);
                    print_bytes(aux2,  val_size);
                #endif
                aux+=val_size;
            }else{
                aux2 = (char *) malloc(sizeof(char)*(val_size+1));
                memcpy(aux2, aux, buf+1024-aux);
                aux3 = aux2 + (buf + 1024 - aux);
                read(backup, aux3, val_size-(buf+1024-aux));
                aux2[val_size]='\0';
                item_aux =hash->item_create(val_size, (uint8_t *) aux2);
                insert_item(hash, item_aux, key, 0,0);
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
            /*TODO: test this case*/
            /* there is more to read*/
            l=aux-buf;
            memcpy(buf2,aux,l);
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

int process_hash_log(hash_table * hash, char * log_path){
    int log_file = open(log_path,0, "r");
    if(log_file == -1){
        return -1;
    }
    #ifdef DEBUG
        printf("Reading Log File...\n");
    #endif

    const int bufsize = 100;

    char * buf = (char *) malloc(sizeof(char)*(bufsize+1));
    char * buf2 = (char *) malloc(sizeof(char)*(bufsize+1));
    int k;
    k = read(log_file, buf, bufsize);
    if(k!=0){
        buf[k]='\0'; /*Needed?*/
        #ifdef DEBUG
            printf("Buffer:");
            print_bytes(buf,  k+1);
        #endif
        char log_type;
        char space;
        char * ptr, * ptr2, * ptr3;
        Item item_aux;
        int l;
        uint32_t key, val_size;
        ptr = buf;
        int buf_left = k;
        while(1){
            /*Get log type and space*/
            if(sscanf(buf,"%c%c",&log_type, &space)!=2){
                #ifdef DEBUG
                    printf("log_type errror\n");
                #endif
                return -1;
            }
            if(space != ' '){
                #ifdef DEBUG
                    printf("No ' ' found\n");
                #endif
                return -1;
            }
            if(log_type == 'I'){
                /*it's an insert*/
                #ifdef DEBUG
                    printf("Processing Insert\n");
                #endif
                ptr = buf+2;
                buf_left-=2;
                if(sscanf(ptr,"%u %u%c%n",&key, &val_size ,&space, &l)>=3){
                    if(space != ' '){
                        #ifdef DEBUG
                            printf("No ' ' found\n");
                        #endif
                        return -1;
                    }
                    /*Got key and size*/
                    ptr+=l;
                    buf_left-=l;
                    if(buf+bufsize-ptr >= val_size){
                        /*There is still room in the buffer for the value*/
                        ptr2 = (char *) malloc(sizeof(char)*(val_size+1));
                        memcpy(ptr2, ptr, val_size);
                        ptr2[val_size]='\0';
                        item_aux = hash->item_create(val_size, (uint8_t *) ptr2);
                        insert_item(hash, item_aux, key, 1, 0);
                        #ifdef DEBUG
                            printf("K: %d, V:", key);
                            print_bytes(ptr2,  val_size);
                        #endif
                        buf_left-=val_size;
                        ptr+=val_size;
                    }else{
                        /*Copy the rest of the buffer to a new buffer and
                        read the rest of the value*/
                        ptr2 = (char *) malloc(sizeof(char)*(val_size+1));
                        memcpy(ptr2, ptr, buf+bufsize-ptr);
                        ptr3 = ptr2 + (buf+bufsize-ptr);
                        read(log_file, ptr3, val_size-(buf+bufsize-ptr));
                        ptr2[val_size]='\0';
                        item_aux =hash->item_create(val_size, (uint8_t *) ptr2);
                        insert_item(hash, item_aux, key, 1, 0);
                        #ifdef DEBUG
                            printf("K: %d, V:", key);
                            print_bytes(ptr2,  val_size);
                        #endif
                        ptr = buf + bufsize;
                        buf_left=0;

                    }
                }else{
                    #ifdef DEBUG
                        printf("Error scanf Insert\n");
                    #endif
                    return -1;
                }
            }else if (log_type == 'D'){
                #ifdef DEBUG
                    printf("Processing Delete\n");
                #endif
                /*It's a delete*/
                buf_left-=2;
                ptr = buf+2;
                if(sscanf(ptr,"%u%n",&key, &l)>=1){
                    delete_item(hash, key, 0);
                    ptr+=l;
                    buf_left-=l;
                }else{
                    return -1;
                }
            }else{
                #ifdef DEBUG
                    printf("Unkown Log type\n");
                #endif
            }
            #ifdef DEBUG

            #endif
            /*buf_left = buf+bufsize-ptr;*/
            #ifdef DEBUG
                printf("Ptr str: %s \n Bytes: ", ptr);
                print_bytes(ptr,  buf_left);
                printf("Buffer still has %d bytes left\n", buf_left);
            #endif
            memcpy(buf2, ptr, buf_left);
            ptr=buf2+buf_left;
            #ifdef DEBUG
                printf("Reading more %d bytes\n",  bufsize-buf_left);
            #endif
            k = read(log_file, ptr, bufsize-buf_left);
            #ifdef DEBUG
                printf("Read %d bytes\n",  k);
            #endif
            buf_left+=k;
            if(k==0 && buf_left == 0){
                break;
            }
            ptr[k]='\0';
            ptr3 = buf2;
            buf2 = buf;
            buf = ptr3;
            ptr = buf;
            #ifdef DEBUG
                printf("New Buffer:%s\n", buf);
            #endif
        }
    }else{
        #ifdef DEBUG
            printf("Log File empty\n");
        #endif
    }
    #ifdef DEBUG
        printf("Finished Processing Log File\n");
    #endif
    close(log_file);
    free(buf);
    free(buf2);
    return 0;
}
