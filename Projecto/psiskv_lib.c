#include "TCPlib.h"
#include "inetutils.h"
#include <stdint.h>
#include "psiskv.h"
#include <stdlib.h>
#include <string.h>
/*#include "debug.h"*/
#include <stdio.h>

const char * msg_type_to_str(int type){
    switch (type) {
        case WRITE_REQ:
            return "WRITE_REQ";
            break;
        case WRITE_REQ_OW:
            return "WRITE_REQ_OW";
            break;
        case WRITE_RESP:
            return "WRITE_RESP";
            break;
        case READ_REQ:
            return "READ_REQ";
            break;
        case READ_RESP:
            return "READ_RESP";
            break;
        case DELETE_REQ:
            return "DELETE_REQ";
            break;
        case DELETE_RESP:
            return "DELETE_RESP";
            break;
        case ERROR:
            return "ERROR";
            break;
    }
    return "Unknown";
}

int kv_connect(char * kv_server_ip, int kv_server_port){
    /* TODO: receber o endereço do DS e conectar se a ele */

    char buffer[128];
    char data_server_ip[32];
    int data_server_port;
    int server_fd = TCPconnect(atoh(kv_server_ip), (unsigned short) kv_server_port);

    if(TCPrecv(server_fd, (uint8_t*) buffer, 128*sizeof(char)) == -1){
        return -1;
    }

    TCPclose(server_fd);

    printf("> %s\n", buffer);

    if((sscanf(buffer, "%[^:]:%d", data_server_ip, &data_server_port)) == -1){
        return -1;
    }

    printf("Data Server Location: %s:%d\n", data_server_ip, data_server_port);

    return TCPconnect(atoh(data_server_ip), (unsigned short) data_server_port);
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
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length, int overwrite){

    kv_msg key_value;
    key_value.key = key;
    key_value.type = WRITE_REQ;
    if(overwrite == 1){
        key_value.type = WRITE_REQ_OW;
    }
    key_value.value_len = value_length;

    uint8_t * to_send = (uint8_t *) &key_value;


    #ifdef DEBUG
        printf("Writing key:%d\nValue_len:%d\n",key, value_length);
    #endif
    /*
        Send the Write_Request;
    */
    if (TCPsend(kv_descriptor, to_send, sizeof(kv_msg))==-1){
        return -1;
    }

    /*
        Send the value;
    */
    to_send = (uint8_t *) value;
    if (TCPsend(kv_descriptor, to_send, value_length)==-1){
        return -1;
    }

    /*
        Receive Write Response;
    */
    uint8_t * to_recv = (uint8_t *) &key_value;
    if(TCPrecv(kv_descriptor, to_recv, sizeof(kv_msg))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("After Write received: \ntype: %s\n",msg_type_to_str(key_value.type) );
    #endif

    if(key_value.type != WRITE_RESP){
        return -1;
    }
    if(key_value.key == 999){
        return -2;
    }

    return 0;
}

/*
TODO: Take precautions for disconnected connections during the TCPrecv and TCPwrite
*/

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
    if(TCPrecv(kv_descriptor, to_recv, sizeof(kv_msg))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("After READ_REQ received: \ntype: %s\nkey:%d\nValue_len:%d\n",
            msg_type_to_str(key_value.type), key_value.key, key_value.value_len);
    #endif

    if(key_value.type != READ_RESP){
        return -2;
    }

    /*
        Read the whole value;
    */
    to_recv = (uint8_t * ) malloc(key_value.value_len * sizeof(uint8_t));

    if(TCPrecv(kv_descriptor, to_recv, key_value.value_len)==-1){
        return -1;
    }
    #ifdef DEBUG
        printf("Value received: %s\n", (char *) to_recv );
    #endif

    /*
        Copy the read value to the given pointer at most value_len bytess
    */

    if ( (unsigned int) value_length > key_value.value_len){
        strncpy(value, (char *) to_recv, key_value.value_len);
    }else{
        strncpy(value, (char *) to_recv, value_length);
    }
    free(to_recv);

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
    if(TCPrecv(kv_descriptor, to_recv, sizeof(kv_msg))==-1){
        return -1;
    }

    #ifdef DEBUG
        printf("\nAfter DELETE_REQ received: \ntype: %s\nkey:%d\nValue_len:%d\n\n",
            msg_type_to_str(key_value.type), key_value.key, key_value.value_len);
    #endif

    if(key_value.type != DELETE_RESP){
        return -1;
    }

    if(key_value.value_len == 0){
        /*No item with that key*/
        /*TODO: Ask which value should return on this situation*/
        return 1;
    }

    return 0;
}
