#include <stdlib.h>
#include <stdio.h>

char * find_char( char to_find, char * start){
    char * aux = start;
    while(*aux!=to_find){
        aux++;
    }
    return aux;
}

char to_upper(char c){
    if(c >=97 && c <=122)
        return c-32;
    return c;
}

int main(int argc, char *argv[]) {
    int i;
    long diff;
    char * aux;
    char * aux2;
    char * aux3;
    char ** new_agrv;
    new_agrv = (char**) malloc(argc*(sizeof(char*)));
    for(i = 0; i<argc; i++){
        aux = find_char('\0',argv[i]);
        diff = aux-argv[i];
        new_agrv[i] = (char*) malloc((diff+1)*sizeof(char));
        aux2 = new_agrv[i];
        for(aux3 = argv[i]; aux3<aux; aux3++){
            (*aux2)=to_upper(*aux3);
            aux2++;
        }
        (*aux2)='\0';
        printf("Argv[%d] = %s\n",i,new_agrv[i]);

    }
    return 0;
}
