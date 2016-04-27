#include "psiskv.h"
#include <stdio.h>
#include <string.h>

#define MAX_VALUES 10
int main(){
	char linha[100];
	int kv = kv_connect("127.0.0.1", 9999);

	for (uint32_t i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i);
		kv_write(kv, i , linha, strlen(linha)+1, 0);
	}

	printf("press enter to read values\n");
	getchar();
	for (uint32_t i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s", i, linha);
		}
	}

	printf("press enter to delete even values\n");
	getchar();
	for (uint32_t i = 0; i < MAX_VALUES; i +=2){
		kv_delete(kv, i);
	}

	printf("press enter to read values\n");
	getchar();
	for (uint32_t i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s", i, linha);
		}
	}

	for (uint32_t i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i*10);
		kv_write(kv, i , linha, strlen(linha)+1, 0); /* will not overwrite*/
	}

	printf("press enter to read new values\n");
	getchar();
	for (uint32_t i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("key - %10u value %s", i, linha);
		}
	}
	kv_close(kv);
	
	
	
}
