#include "psiskv.h"
#include "hash-lib.h"
#include "psiskv_server.h"
#include "TCPlib.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>


int process_psiskv_request(int kv_descriptor, hash_table store, uint32_t size){

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
            if(write_req(store, kv_descriptor, key_value.key, key_value.value_len, size)<0){
                return -1;
            }
            break;
        case READ_REQ:
            #ifdef DEBUG
                printf("Received READ_REQ\n");
            #endif
            if(read_req(store, kv_descriptor, key_value.key, size)<0){
                return -1;
            }
            break;
        case DELETE_REQ:
            #ifdef DEBUG
                printf("Received DELETE_REQ\n");
            #endif
            if(delete_req(store, kv_descriptor, key_value.key, size)<0){
                return -1;
            }
            break;
        default:
            return -1;

        }

    return 0;
}

/*
TODO: Take precautions for disconnected connections during the TCPrecv and TCPwrite
*/

int write_req(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len, uint32_t size){

    /*Now read value_len bytes from the socket*/
    uint8_t * item =(uint8_t * ) malloc(value_len * sizeof(uint8_t));

    /*Receive the value from the socket*/
    if(TCPrecv(kv_descriptor, item, value_len)==-1){
        return -1;
    }
    #ifdef DEBUG
        printf("Value received: %s\n", (char *) item );
    #endif

    /*TODO: Define functions to create and delete this structs */
    value_struct * to_store;
    to_store = (value_struct *) malloc(sizeof(value_struct));
    to_store->size = value_len;
    to_store->value = item;

    /*Insert the item on the hash store*/
    if(!insert_item(store,to_store,key,size)){
        return -1;
    }

    /*Send the response*/
    kv_msg message;
    message.type = WRITE_RESP;
    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }

    return 0;
}

int read_req(hash_table store, int kv_descriptor, uint32_t key, uint32_t size){

    value_struct * to_send;
    to_send = (value_struct *) read_item(store, key, size);

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

int delete_req(hash_table store, int kv_descriptor, uint32_t key, uint32_t size){
    /*TODO: Use correct delete function instead of free*/
    if (!delete_item(store , key, size, free)){
        return -1;
    }
    kv_msg message;
    message.type = DELETE_RESP;
    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }
    return 0;
}
