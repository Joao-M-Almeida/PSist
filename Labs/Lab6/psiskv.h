#include <stdint.h>
#include "item.h"

#ifndef _PSISKV_H
#define _PSISKV_H

#define BUF_LEN 100
typedef enum msg_type {
    WRITE_REQ,
    WRITE_RESP,
    READ_REQ,
    READ_RESP,
    DELETE_REQ,
    DELETE_RESP,
    ERROR
} msg_type;

typedef struct kv_msg{
    msg_type type;
    uint32_t key;
    unsigned int value_len;
} kv_msg;

int kv_connect(char * kv_server_ip, int kv_server_port);
void kv_close(int kv_descriptor);
int kv_write(int kv_descriptor, uint32_t key, char * value, int value_length);
int kv_read(int kv_descriptor, uint32_t key, char * value, int value_length);
int kv_delete(int kv_descriptor, uint32_t key);

#endif
