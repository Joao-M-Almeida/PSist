#include "storyserver.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(){
    char fifo_name[MESSAGE_LEN];
    message m;
    char * story;
 
        
	story = strdup("");
	
     /* create socket  */ 
        
    while(1){
        /* read message */
        /* process message */
        
     }
    printf("OK\n");
    exit(0);
    
}
