#include "psiskv.h"
#include "hash-lib.h"
#include "psiskv_server.h"
#include "TCPlib.h"


int process_psiskv_request(int kv_descriptor, hash_table store){


    kv_msg key_value;
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    switch (key_value.type) {
        case WRITE_REQ:
            if(write_req(store, kv_descriptor, key_value.key, key_value.value_len)<0){
                return -1;
            }
            break;
        case READ_REQ:
            if(read_req(store, kv_descriptor, key_value.key)<0){
                return -1;
            }
            break;
        case DELETE_REQ:
            if(delete_req(store, kv_descriptor, key_value.key)<0){
                return -1;
            }
            break;
        default:
            return -1;

        }

    return 0;
}

/*
int write_req(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len){
    return 0;
}
int read_req(hash_table store, int kv_descriptor, uint32_t key){
    return 0;
}
int delete_req(hash_table store, int kv_descriptor, uint32_t key){
    return 0;
}*/
