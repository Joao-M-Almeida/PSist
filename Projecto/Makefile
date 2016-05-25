CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99 -g
DEPS = log.h debug.h item.h phash-lib.h inetutils.h TCPlib.h psiskv.h psiskv_server.h
CLIENT_OBJ = client.o debug.o inetutils.o TCPlib.o psiskv_lib.o
FRONT_SERVER_OBJ = front-server.o debug.o log.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o
DATA_SERVER_OBJ = data-server.o debug.o log.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o
CLI1 = cli-exe-1.o psiskv_lib.o  inetutils.o TCPlib.o
CLI1PAR = cli-exe-par-1.o psiskv_lib.o inetutils.o TCPlib.o
EDITOR = gedit

all: main client.out cli-1.out cli-1-par.out

main: front_server.out data_server.out

front_server.out: $(FRONT_SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

data_server.out: $(DATA_SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

client.out: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

c:
	clear
	rm *.o -f
	rm *.gch -f
	rm *.out -f
#	rm *.data -f

C:
	clear
	rm *.o -f
	rm *.gch -f
	rm *.out -f
	rm *.data -f
	rm *.temp -f

o:
	$(EDITOR) *.c *.h makefile &

cli-1.out: $(CLI1)
	$(CC) $(CFLAGS) $^ -o $@ -pthread
cli-1-par.out: $(CLI1PAR)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
