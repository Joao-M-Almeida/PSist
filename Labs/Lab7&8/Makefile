CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99 -g
DEPS = debug.h item.h phash-lib.h inetutils.h psiskv.h psiskv_server.h TCPlib.h
OBJ = kvserver.o phash-lib.o inetutils.o psiskv_lib.o psiskv_server.o TCPlib.o
CLIENT = client.o inetutils.o psiskv_lib.o TCPlib.o
CLI1 = cli-exe-1.o psiskv_lib.o  inetutils.o TCPlib.o
CLI1PAR = cli-exe-par-1.o psiskv_lib.o inetutils.o TCPlib.o
EDITOR = gedit

server.out: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

client.out: $(CLIENT)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

c:
	clear
	rm *.o -f
	rm *.gch -f
	rm *.out -f
o:
	$(EDITOR) *.c *.h makefile &

cli-1.out: $(CLI1)
	$(CC) $(CFLAGS) $^ -o $@ -pthread
cli-1-par.out: $(CLI1PAR)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
