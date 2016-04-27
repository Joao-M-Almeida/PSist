CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99
DEPS = debug.h item.h hash-lib.h inetutils.h psiskv.h psiskv_server.h psiskv_pserver.h TCPlib.h
OBJ = kvserver.o hash-lib.o inetutils.o psiskv_lib.o psiskv_server.o TCPlib.o
CLIENT = client.o hash-lib.o inetutils.o psiskv_lib.o TCPlib.o
EDITOR = gedit


server.out: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

client.out: $(CLIENT)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

c:
	clear
	rm *.o -f
	rm *.gch -f
	rm *.out -f
o:
	$(EDITOR) *.c *.h makefile &

# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
