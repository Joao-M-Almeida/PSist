#include "psiskv.h"
#include "hash-lib.h"
#include "psiskv_server.h"
#include "TCPlib.h"

#include <stdlib.h>


int process_psiskv_request(int kv_descriptor, hash_table store, uint32_t size){


    kv_msg key_value;
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    switch (key_value.type) {
        case WRITE_REQ:
            if(write_req(store, kv_descriptor, key_value.key, key_value.value_len, size)<0){
                return -1;
            }
            break;
        case READ_REQ:
            if(read_req(store, kv_descriptor, key_value.key, size)<0){
                return -1;
            }
            break;
        case DELETE_REQ:
            if(delete_req(store, kv_descriptor, key_value.key, size)<0){
                return -1;
            }
            break;
        default:
            return -1;

        }

    return 0;
}


int write_req(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len, uint32_t size){

    /*Now read value_len bytes from the socket*/
    uint8_t * item =(uint8_t * ) malloc(value_len * sizeof(uint8_t));

    /*Receive the value from the socket*/
    if(TCPrecv(kv_descriptor, item, value_len)==-1){
        return -1;
    }

    /*Insert the item on the hash store*/
    if(!insert_item(store,item,key,size)){
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

    uint8_t * item = (uint8_t*) read_item(store, key, size);

    if(item == NULL){
        /*If the key is not present should send ERROR to client*/
        kv_msg message;
        message.type = ERROR;
        if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
            return -1;
        }
    }

    /* Send the response with the size of the item*/
    kv_msg read_response;
    read_response.type = READ_RESP;
    read_response.value_len = sizeof(item);
    if(TCPsend(kv_descriptor, (uint8_t *) &read_response, sizeof(read_response))==-1){
        return -1;
    }


    /*Send the item*/
    if(TCPsend(kv_descriptor, item, sizeof(item))==-1){
        return -1;
    }

    return 0;
}

int delete_req(hash_table store, int kv_descriptor, uint32_t key, uint32_t size){
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
