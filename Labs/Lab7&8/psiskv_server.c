#include "psiskv.h"
#include "phash-lib.h"
#include "psiskv_server.h"
#include "TCPlib.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int process_psiskv_prequest(int kv_descriptor, hash_table * store){

    kv_msg key_value;
    uint8_t * to_recv = (uint8_t *) &key_value;
    int result = TCPrecv(kv_descriptor, to_recv, sizeof(kv_msg));
    if( result < 0 ){
        return result;
    }

    #ifdef DEBUG
        printf("Request: \n\ttype: %s\n\tkey:%d\n\tValue_len:%d\n",
            msg_type_to_str(key_value.type), key_value.key, key_value.value_len);
    #endif

    switch (key_value.type) {
        case WRITE_REQ:
            #ifdef DEBUG
                printf("Received WRITE_REQ\n");
            #endif
            if(write_preq(store, kv_descriptor, key_value.key, key_value.value_len, 0)<0){
                return -1;
            }
            break;
        case WRITE_REQ_OW:
            #ifdef DEBUG
                printf("Received WRITE_REQ_OW\n");
            #endif
            if(write_preq(store, kv_descriptor, key_value.key, key_value.value_len, 1)<0){
                return -1;
            }
            break;
        case READ_REQ:
            #ifdef DEBUG
                printf("Received READ_REQ\n");
            #endif
            if(read_preq(store, kv_descriptor, key_value.key)<0){
                return -1;
            }
            break;
        case DELETE_REQ:
            #ifdef DEBUG
                printf("Received DELETE_REQ\n");
            #endif
            if(delete_preq(store, kv_descriptor, key_value.key)<0){
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
void * create_struct( unsigned int size, uint8_t *value ){
    value_struct * vs;
    vs = (value_struct *) malloc(sizeof(value_struct));
    vs->size = size;
    vs->value = value;
    return vs;
}

Item copy_struct( void * to_copy ){
    value_struct * to_cpy = (value_struct * ) to_copy;
    value_struct * vs;
    vs = (value_struct *) malloc(sizeof(value_struct));
    vs->size = to_cpy->size;
    vs->value = (uint8_t *) malloc(sizeof(uint8_t) * to_cpy->size);
    memcpy(vs->value, to_cpy->value, to_cpy->size);

    return vs;
}


void destroy_struct(void * void_to_destroy) {
    /*TODO: not sure if this is the correct way*/
    value_struct * to_destroy = (value_struct * ) void_to_destroy;
    free(to_destroy->value);
    free(to_destroy);
}

char * struct_to_str(void * void_to_str){
    value_struct * to_str = (value_struct * ) void_to_str;
    char * str = (char *) malloc(sizeof(char) * ( to_str->size+1));
    memcpy(str, (char *) to_str->value, to_str->size);
    str[to_str->size] = '\0';
    return str;
}

uint32_t struct_get_size(void * strct_size){
    value_struct * strct = (value_struct * ) strct_size;
    return strct->size;
}


/*Str is \0 terminated so remove it*/
void * create_struct_from_str( char * str){
    value_struct * vs;
    vs = (value_struct *) malloc(sizeof(value_struct));
    int size = strlen(str);
    uint8_t * value = (uint8_t *) malloc(sizeof(uint8_t)*size);

    /*TODO: copy from str to value*/

    memcpy(value, str,size);

    vs->size = size;
    vs->value = value;
    return vs;
}


int write_preq(hash_table * store, int kv_descriptor, uint32_t key, unsigned int value_len, int overwrite){
    int err;
    /*Now read value_len bytes from the socket*/
    uint8_t * item = (uint8_t *) malloc(value_len * sizeof(uint8_t));

    /*Receive the value from the socket*/
    if(TCPrecv(kv_descriptor, item, value_len)==-1){
        return -1;
    }
    #ifdef DEBUG
        printf("Value received: %s\n", (char *) item );
    #endif

    value_struct * to_store = (value_struct *) create_struct(value_len, item );


    kv_msg message;
    message.type = WRITE_RESP;
    message.key = 0;
    message.value_len = 0;

    /*Insert the item on the hash store*/
    err = insert_item(store,to_store,key,overwrite);
    #ifdef DEBUG
        printf("insert_item returned %d\n", err);
    #endif
    if(err < 0){
        return err;
    }else if (err == 1){
        /*already exists and didn't overwrite*/
        message.key = 999;
        #ifdef DEBUG
            printf("Item already exists and didn't overwrite\n");
        #endif
    }



    /*Send the response*/
    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }

    return 0;
}

int read_preq(hash_table * store, int kv_descriptor, uint32_t key){

    value_struct * to_send;

    to_send = (value_struct *) read_item(store, key);

    if(to_send == NULL){
        #ifdef DEBUG
            printf("Value not found.\n");
        #endif
        /*If the key is not present should send ERROR to client*/
        kv_msg message;
        message.type = ERROR;
        message.value_len = 0;
        message.key = 0;

        if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
            return -1;
        }
        return 0;
    }

    /* Send the response with the size of the item*/
    kv_msg read_response;
    read_response.type = READ_RESP;
    read_response.value_len = to_send->size;
    read_response.key = 0;
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

    destroy_struct(to_send);

    return 0;
}

int delete_preq(hash_table * store, int kv_descriptor, uint32_t key){

    kv_msg message;
    message.type = DELETE_RESP;
    message.key = 0;

    if (!delete_item(store, key)){
        /*Item doesn't exist*/
        message.value_len = 0;
    } else{
        message.value_len = 1;
    }


    if(TCPsend(kv_descriptor, (uint8_t *) &message, sizeof(message))==-1){
        return -1;
    }
    return 0;
}
