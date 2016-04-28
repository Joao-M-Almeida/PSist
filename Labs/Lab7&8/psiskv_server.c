#include "psiskv.h"
#include "hash-lib.h"
#include "psiskv_server.h"
#include "TCPlib.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>

int process_psiskv_prequest(int kv_descriptor, hash_table store, uint32_t size, pthread_mutex_t readlock, pthread_mutex_t writelock){

    kv_msg key_value;
    uint8_t * to_recv = (uint8_t *) &key_value;
    int result = TCPrecv(kv_descriptor, to_recv, sizeof(kv_msg));
    if( result < 0){
        return result;
    }
    
    #ifdef DEBUG
        printf("Request: \ntype: %s\nkey:%d\nValue_len:%d\n",
            msg_type_to_str(key_value.type), key_value.key, key_value.value_len);
    #endif

    switch (key_value.type) {
        case WRITE_REQ:
            #ifdef DEBUG
                printf("Received WRITE_REQ\n");
            #endif
            if(write_preq(store, kv_descriptor, key_value.key, key_value.value_len, size, readlock, writelock)<0){
                return -1;
            }
            break;
        case READ_REQ:
            #ifdef DEBUG
                printf("Received READ_REQ\n");
            #endif
            if(read_preq(store, kv_descriptor, key_value.key, size, readlock, writelock)<0){
                return -1;
            }
            break;
        case DELETE_REQ:
            #ifdef DEBUG
                printf("Received DELETE_REQ\n");
            #endif
            if(delete_preq(store, kv_descriptor, key_value.key, size, readlock, writelock)<0){
                return -1;
            }
            break;
        default:
            return -1;

        }

    return 0;
}

/* deve receber mais uma função para criação correcta do item */
/* deve receber mais uma função, que deve guardar como argumento,
    para a destruição do item*/
value_struct * create_struct( unsigned int size, uint8_t *value ){
    value_struct * vs;
    vs = (value_struct *) malloc(sizeof(value_struct));
    vs->size = size;
    vs->value = value;
    return vs;
}

int write_preq(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len, uint32_t size, pthread_mutex_t readlock, pthread_mutex_t writelock){

    /*Now read value_len bytes from the socket*/
    uint8_t * item =(uint8_t * ) malloc(value_len * sizeof(uint8_t));

    /*Receive the value from the socket*/
    if(TCPrecv(kv_descriptor, item, value_len)==-1){
        return -1;
    }
    #ifdef DEBUG
        printf("Value received: %s\n", (char *) item );
    #endif

    /*TODO: Define functions to create and delete the items in these structs */
    value_struct * to_store = create_struct( value_len, item );

    /* mete o readlock e o writelck on */
    pthread_mutex_lock(&writelock);
    pthread_mutex_lock(&readlock);

    /*Insert the item on the hash store*/
    if(!insert_item(store,to_store,key,size)){
        return -1;
    }

    /* tira o readlock e o writelock */
    pthread_mutex_unlock(&readlock);
    pthread_mutex_unlock(&writelock);

    /*Send the response*/
    kv_msg message;
    message.type = WRITE_RESP;
    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }

    return 0;
}

int read_preq(hash_table store, int kv_descriptor, uint32_t key, uint32_t size, pthread_mutex_t readlock, pthread_mutex_t writelock){

    value_struct * to_send;

    /* mete o writelock on */
    pthread_mutex_lock(&readlock);
    pthread_mutex_unlock(&readlock);
    pthread_mutex_lock(&writelock);

    to_send = (value_struct *) read_item(store, key, size);

    /*tira o writelock*/
    pthread_mutex_unlock(&writelock);

    if(to_send == NULL){
        printf("Value not found.\n");
        /*If the key is not present should send ERROR to client*/
        kv_msg message;
        message.type = ERROR;
        if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
            return -1;
        }
        return 0;
    }

    /* Send the response with the size of the item*/
    kv_msg read_response;
    read_response.type = READ_RESP;
    read_response.value_len = to_send->size;
    if(TCPsend(kv_descriptor, (uint8_t *) &read_response, sizeof(read_response))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("Value read: %s\n", (char *) to_send->value );
    #endif

    /*Send the item*/
    if(TCPsend(kv_descriptor, to_send->value, to_send->size)==-1){
        return -1;
    }

    return 0;
}

int delete_preq(hash_table store, int kv_descriptor, uint32_t key, uint32_t size, pthread_mutex_t readlock, pthread_mutex_t writelock){

    /* mete o readlock e o writelck on */
    pthread_mutex_lock(&writelock);
    pthread_mutex_lock(&readlock);

    /*TODO: Use correct delete function instead of free*/
    if (!delete_item(store, key, size, free)){
        return -1;
    }

    /* tira o readlock e o writelock */
    pthread_mutex_unlock(&readlock);
    pthread_mutex_unlock(&writelock);

    kv_msg message;
    message.type = DELETE_RESP;
    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }
    return 0;
}
