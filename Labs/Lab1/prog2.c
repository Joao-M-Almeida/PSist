#include "test.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
	int a;
	printf("What version of the functions you whant to use?\n");
	printf("\t1 - Normal    (test1)\n");
	printf("\t2 - Optimized (test2)\n");
	scanf("%d", &a);
	if (a == 1){
		printf("running the normal versions from \n");
		/* call func_1 from test1 */
		/* call func_2 from test1 */
			

	}else{
		if(a== 2){
			printf("runnin the normal versions\n");
			/* call func_1 from test2 */
			/* call func_2 from test2 */

		}else{
			printf("Not running anything\n");

		}
	}
	exit(0);
	
	
}
