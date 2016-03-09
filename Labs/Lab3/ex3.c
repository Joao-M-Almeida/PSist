/*
gcc -Wall -pedantic -ansi -std=gnu99 ex3.c -o ex3.out
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

typedef struct shared{
    unsigned int m_7[3];
    unsigned int m_7_19[3];
    unsigned int m_19[3];
} shared;

int main(){
    int k;
    unsigned int m[3] = {0, UINT_MAX/3+1, (UINT_MAX/3)*2+1};
    unsigned int M[3] = {UINT_MAX/3, (UINT_MAX/3)*2, UINT_MAX};

    /*unsigned int  m_7, m_19, m_7_19;
    m_7 = m_19 = m_7_19 = 0;*/

    shared * sh;

    sh = (shared *) mmap(NULL, sizeof(shared) , PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS  , -1, 0);

    int myid = getpid();

    for(k=0;k<3;k++){
        if(fork()==0)
            break;
    }

    if (myid != getpid()){
        sh->m_7[k] = 0;
        sh->m_19[k] = 0;
        sh->m_7_19[k] = 0;

        /*printf("Son %d counting from %u to %u .\n",k, m[k],M[k]);*/
        for (unsigned int  i = m[k]; i <  M[k]; i++){
            if(i%7 == 0){
                sh->m_7[k] ++;
            }
            if(i%19     == 0){
                sh->m_19[k] ++;

            }
            if((i%7 == 0)    && (i%19 == 0)){
                sh->m_7_19[k] ++;

            }
        }

        printf("I am Son # %d\n", k );
        printf("m 7    %d\n", sh->m_7[k]);
        printf("m   19 %d\n", sh->m_19[k]);
        printf("m 7 19 %d\n", sh->m_7_19[k]);
        exit(0);
    } else {
        int status;
        int id;
        for(int i = 0; i<3;i++){
            id = wait(&status);
            printf("Son %d exited with status: %d\n", id, WEXITSTATUS(status));
        }
        unsigned int sum[3] = {0,0,0};
        for (unsigned int j = 0; j < 3; j++) {
            /* code */
            sum[0]=sum[0]+sh->m_7[j];
            sum[1]=sum[1]+sh->m_19[j];
            sum[2]=sum[2]+sh->m_7_19[j];
        }

        printf("Totals are m7 = %u \t m19 = %u \t m7_19 = %u  \n", sum[0],sum[1],sum[2]);
        exit(0);

    }
}
