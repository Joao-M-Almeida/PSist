CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99
DEPS = debug.h item.h hash-lib.h inetutils.h TCPlib.h psiskv.h psiskv_server.h
OBJ = kvserver.o hash-lib.o inetutils.o psiskv_lib.o psiskv_server.o TCPlib.o
EDITOR = gedit

main.out: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@


c:
	clear
	rm *.o

o:
	$(EDITOR) *.c *.h makefile &




# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
