#include "psiskv.h"
#include "phash-lib.h"

#ifndef _PSISKV_SERVER_H
#define _PSISKV_SERVER_H

typedef struct _value_struct {
    uint8_t * value;
    uint32_t size;
} value_struct;
void * create_struct( unsigned int size, uint8_t *value );
void destroy_struct(void * to_destroy);
Item copy_struct( void * to_copy );
char * struct_to_str(void * void_to_str);
void * create_struct_from_str( char * str);
uint32_t struct_get_size(void * );

int process_psiskv_prequest(int kv_descriptor, hash_table * store);
int write_preq(hash_table * store, int kv_descriptor, uint32_t key, unsigned int value_len, int overwrite);
int read_preq(hash_table * store, int kv_descriptor, uint32_t key);
int delete_preq(hash_table * store, int kv_descriptor, uint32_t key);

#endif
