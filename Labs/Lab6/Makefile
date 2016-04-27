CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99
DEPS = debug.h item.h hash-lib.h inetutils.h psiskv.h psiskv_server.h psiskv_pserver.h TCPlib.h
OBJ = kvserver.o hash-lib.o inetutils.o psiskv_lib.o psiskv_server.o psiskv_pserver.o TCPlib.o
TEST_OBJ = thkvserver.c hash-lib.o inetutils.o psiskv_lib.o psiskv_pserver.o TCPlib.o
WCLIENT = writeclient.o hash-lib.o inetutils.o psiskv_lib.o TCPlib.o
RCLIENT = readclient.o hash-lib.o inetutils.o psiskv_lib.o TCPlib.o
CLIENT = client.o hash-lib.o inetutils.o psiskv_lib.o TCPlib.o
EDITOR = gedit

server.out: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test.out: $(TEST_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

c:
	clear
	rm *.o

o:
	$(EDITOR) *.c *.h makefile &
client.out: $(CLIENT)
	$(CC) $(CFLAGS) $^ -o $@


# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
