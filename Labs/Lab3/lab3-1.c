/*
gcc -Wall -pedantic -ansi -std=c99 lab3-1.c -o lab3-1.out
*/

#include <limits.h>
#include <stdio.h>

int main(){
	unsigned int  m_7, m_19, m_7_19;
	m_7 = m_19 = m_7_19 = 0;
	for (unsigned int  i = 0; i <  UINT_MAX; i++){
		if(i%7 == 0){
			m_7 ++;
		}
		if(i%19 == 0){
			m_19 ++;
		}
		if((i%7 == 0)	&& (i%19 == 0)){
			m_7_19++;
		}
	}
	printf("m 7    %d\n", m_7);
	printf("m   19 %d\n", m_19);
	printf("m 7 19 %d\n", m_7_19);

}
