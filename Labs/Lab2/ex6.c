#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024

int main(int argc, char *argv[], char *envp[]) {

    char s[MAX_LINE];
    char prg_name[MAX_LINE];
    char arg1[MAX_LINE];
    char * new_argv[5];

    int aux;
    int mypid;

    FILE * fd;
    fd = fopen("test.script", "r");

    while(fgets(s,MAX_LINE,fd)!=NULL){
        aux = sscanf(s, "%s %s",prg_name,arg1);
        new_argv[0]=prg_name;
        new_argv[1] = arg1;
        new_argv[2] = NULL;

        if(aux){
            printf("Going to execute command: %s\n", prg_name );
            mypid = fork();
            if(mypid == 0){
                printf("SON with PID: %d\n",getpid());
                execve(prg_name, new_argv, envp);
            }
            wait(NULL);
            printf("Finished execution of command\n" );
        }
}




    fclose(fd);
    return 0;
}
