#include <stdlib.h>
#include <stdio.h>

char * find_char( char to_find, char * start){
    char * aux = start;
    while(*aux!=to_find){
        aux++;
    }
    return aux;
}

int main(int argc, char *argv[]) {
    int len=0;
    int i;
    long diff;
    char * aux;
    for(i = 0; i<argc; i++){
        aux = find_char('\0',argv[i]);
        diff = aux-argv[i];
        len = len + diff;
        printf("String %d size = %ld\n",i ,diff);
    }
    printf("String size = %d\n", len);

    if(len>0){
        char * final_str;
        char * aux2;
        char * aux3;
        final_str = (char *) malloc((len+1) * sizeof(char));
        aux2=final_str;
        for(i = 0; i<argc; i++){
            aux = find_char('\0',argv[i]);
            for(aux3=argv[i];aux3<aux;aux3++){
                (*aux2)=*aux3;
                aux2++;
            }
        }
        /*(*aux2)='\0';*/
        printf("String = %s\n", final_str);
    }
    return 0;
}
