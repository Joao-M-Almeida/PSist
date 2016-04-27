#include "psiskv.h"
#include "hash-lib.h"


#ifndef _PSISKV_SERVER_H
#define _PSISKV_SERVER_H

typedef struct _value_struct {
    uint8_t * value;
    uint32_t size; 
} value_struct;

int process_psiskv_request(int kv_descriptor, hash_table store, uint32_t size);
int write_req(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len, uint32_t size);
int read_req(hash_table store, int kv_descriptor, uint32_t key, uint32_t size);
int delete_req(hash_table store, int kv_descriptor, uint32_t key, uint32_t size);





#endif
