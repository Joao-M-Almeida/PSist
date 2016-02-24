#include <stdlib.h>
#include "test.h"
#include <dlfcn.h>



int main(){
	void *handle;
	void (*func)();

	handle = dlopen ("/media/jmirandadealme/DATA1/Dropbox/IST/Programação de Sistemas/PSist/Labs/Lab1/test2.so", RTLD_LAZY);
	        if (!handle) {
	            fputs (dlerror(), stderr);
	            exit(1);
	        }
	func = dlsym(handle, "func_1");

	(*func)();
	exit(0);
}
