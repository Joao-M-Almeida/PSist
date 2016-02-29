#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
 #include <signal.h>

int main(){
	int mypid;
	int i;
	
	if(	fork() == 0){
		fork();
		fork();
		
		mypid = getpid();
		printf("SON my PID is %d\n", mypid);
		kill(mypid, SIGSTOP);
		for(i=0; i<10; i++){
			printf("SON %d %d\n", mypid, i);
		}
		printf("SON Exiting %d\n", mypid);
	}		
	else{
		mypid = getpid();

		printf("PARENT my PID is %d\n", mypid);

		kill(mypid, SIGSTOP);
		printf("PARENT Exiting %d\n", mypid);

	}
	exit(0);
}
