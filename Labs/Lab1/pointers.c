#include <stdlib.h>

int a;

int main(){
    int b;
    
    char *p = malloc(1);
    
    
    
    printf("&a - %p\n", &a);
    
    printf("&main - %p\n", main);
    printf("&exit %p\n", exit);
    
    printf("&b - %p\n", &b);
    printf("&p - %p\n", &p);
    
    printf("p - %p\n", p);

    
    
    exit(0);
    
    
}
