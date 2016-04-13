#include "psiskv.h"
#include "hash-lib.h"


#ifndef _PSISKV_SERVER_H
#define _PSISKV_SERVER_H


int process_psiskv_request(int kv_descriptor, hash_table store);
int write_req(hash_table store, int kv_descriptor, uint32_t key, unsigned int value_len);
int read_req(hash_table store, int kv_descriptor, uint32_t key);
int delete_req(hash_table store, int kv_descriptor, uint32_t key);





#endif
