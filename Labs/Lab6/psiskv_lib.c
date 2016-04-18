#include "TCPlib.h"
#include "inetutils.h"
#include <stdint.h>
#include "psiskv.h"
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include <stdio.h>


int kv_connect(char * kv_server_ip, int kv_server_port){
    return TCPconnect(atoh(kv_server_ip), (unsigned short) kv_server_port);
}

/*
    For now ignores error of TCPclose
*/
void kv_close(int kv_descriptor){
    TCPclose(kv_descriptor);
}

/*
    Sends a Write_Request to server and waits for Write_Response.
*/
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length){

    kv_msg key_value;
    key_value.key = key;
    key_value.type = WRITE_REQ;
    key_value.value_len = value_length;

    uint8_t * to_send = (uint8_t *) &key_value;

    /*
        Send the Write_Request;
    */
    if (TCPsend(kv_descriptor, to_send, sizeof(to_send))==-1){
        return -1;
    }

    /*
        Send the value;
    */
    to_send = (uint8_t *) value;
    if (TCPsend(kv_descriptor, to_send, sizeof(to_send))==-1){
        return -1;
    }

    /*
        Receive Write Response;
    */
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("After Write received: \ntype: %d\nkey:%d\nValue_len:\n%d\n",
            key_value.type, key_value.key, key_value.value_len);
    #endif

    if(key_value.type != WRITE_RESP){
        return -2;
    }

    return 0;
}

/*
    Sends a Read_request to server and waits for response key_read_server.
*/
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length){

    kv_msg key_value;
    key_value.key = key;
    key_value.type = READ_REQ;
    key_value.value_len = 0;

    uint8_t * to_send = (uint8_t *) &key_value;

    /*
        Send the Read Request;
    */
    if (TCPsend(kv_descriptor, to_send, sizeof(to_send))==-1){
        return -1;
    }

    /*
        Read the Read Response;
    */
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("After READ_REQ received: \ntype: %d\nkey:%d\nValue_len:\n%d\n",
            key_value.type, key_value.key, key_value.value_len);
    #endif

    if(key_value.type != READ_RESP){
        return -2;
    }

    /*
        Read the whole value;
    */
    to_recv = (uint8_t * ) malloc(key_value.value_len * sizeof(uint8_t));

    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    /*
        Copy the read value to the given pointer at most value_len bytess
    */
    strncpy(value, (char *) to_recv, value_length);

    return 0;
}
/*
    Sends a DELETE_REQ to server and waits for response DELETE_RESP.
*/
int kv_delete(int kv_descriptor, uint32_t key){
    kv_msg key_value;
    key_value.key = key;
    key_value.type = DELETE_REQ;
    key_value.value_len = 0;

    uint8_t * to_send = (uint8_t *) &key_value;

    /*
        Send the Delete Request;
    */
    if (TCPsend(kv_descriptor, to_send, sizeof(to_send))==-1){
        return -1;
    }

    /*
        Read the Delete Response;
    */
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(to_recv))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("After DELETE_REQ received: \ntype: %d\nkey:%d\nValue_len:\n%d\n",
            key_value.type, key_value.key, key_value.value_len);
    #endif

    if(key_value.type != DELETE_RESP){
        return -2;
    }

    return 0;
}
