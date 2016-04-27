#include <unistd.h>
#DEFINE MAX_CLIENTS 10

int main(int argc, char *argv[]){
    int i;
    char * args[2];
    pthread_t tid[MAX_CLIENTS];
    int ret_val, err_val;

    for(i = 0; i < MAX_CLIENTS/2; i++){
        if(i%2==0) {
            err_val = pthread_create(tid[i], writeclient, NULL);

        } else {
            err_val = pthread_create(tid[i+1], readclient, NULL);

        }
    }

    for(i = 0; i < MAX_CLIENTS; i++){
        if(i%2==0){
            err_val = pthread_join(tid[i], &&ret_val);

        } else {
            err_val = pthread_join(tid[i+1], &&ret_val);

        }
    }

    return 0;
}

void *writeclient(void *args){
    execve("./client", args, NULL);
    pthread_exit(&0);
}

void *readclient(void *args){
    execve("./rclient", args, NULL);
    pthread_exit(&0);
}
