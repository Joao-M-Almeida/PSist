/*
gcc -Wall -pedantic -ansi -std=c99 lab3-1.c -o lab3-1.out
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    int k;
    unsigned int m[3] = {0, UINT_MAX/3+1, (UINT_MAX/3)*2+1};
    unsigned int M[3] = {UINT_MAX/3, (UINT_MAX/3)*2, UINT_MAX};

    /*for (int j = 0; j < 3; j++) {
        printf("m[%d] = %u\n", j, m[j]);
        printf("M[%d] = %u\n", j, M[j]);
    }*/

    unsigned int  m_7, m_19, m_7_19;
    m_7 = m_19 = m_7_19 = 0;

    int myid = getpid();

    for(k=0;k<3;k++){
        if(fork()==0)
            break;
    }

    if (myid == getpid()){
        int status;
        int id;
        int sum=0;
        for(int i = 0; i<3;i++){
            id = wait(&status);
            sum+=WEXITSTATUS(status);
            printf("Son %d exited with status: %d\n", id, WEXITSTATUS(status));
        }
        exit(0);
    } else {
        /*printf("Son %d counting from %u to %u .\n",k, m[k],M[k]);*/
        for (unsigned int  i = m[k]; i <  M[k]; i++){
            if(i%7 == 0){
                m_7 ++;
            }
            if(i%19     == 0){
                m_19 ++;
            }
            if((i%7 == 0)    && (i%19 == 0)){
                m_7_19++;
            }
        }

        printf("I am Son # %d\n", k );
        printf("m 7    %d\n", m_7);
        printf("m   19 %d\n", m_19);
        printf("m 7 19 %d\n", m_7_19);
        exit(0);
    }
}
