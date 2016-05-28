CC = gcc
CFLAGS = -Wall -pedantic -ansi -std=gnu99 -g
DEPS = log.h debug.h item.h phash-lib.h inetutils.h TCPlib.h psiskv.h psiskv_server.h front-lib.h data-lib.h front-lib.h data-lib.h
CLIENT_OBJ = client.o debug.o inetutils.o TCPlib.o psiskv_lib.o
FRONT_SERVER_OBJ = debug.o log.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o front-lib.o front-server.o
DATA_SERVER_OBJ = debug.o log.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o data-lib.o data-server.o
CLI1_OBJ = cli-exe-1.o psiskv_lib.o  inetutils.o TCPlib.o
CLI1PAR_OBJ = cli-exe-par-1.o psiskv_lib.o inetutils.o TCPlib.o
EDITOR = gedit

all: main client.out cli-1.out cli-1-par.out

main: front_server.out data_server.out

run_front: front_server.out data_server.out
	@echo Running Front Server
	@./front_server.out

front_server.out: $(FRONT_SERVER_OBJ)
	@echo Compiling $@
	$(CC) $(CFLAGS) $^ -o $@ -pthread

run_data: front_server.out data_server.out
	@echo Running Data Server
	@./data_server.out

data_server.out: $(DATA_SERVER_OBJ)
	@echo Compiling $@
	$(CC) $(CFLAGS) $^ -o $@ -pthread

run_client: client.out
	@echo Running Client Server
	@./client.out

client.out: $(CLIENT_OBJ)
	@echo Compiling $@
	$(CC) $(CFLAGS) $^ -o $@ -pthread

%.o: %.c $(DEPS)
	@echo Compiling $<
	$(CC) $(CFLAGS) -c $< -o $@ -pthread

c:
	@rm *.o -f
	@rm *.gch -f
	@rm *.out -f
	clear
#	rm *.data -f

C: c
	@rm *.data -f
	@rm *.temp -f

o:
	$(EDITOR) *.c *.h makefile &

cli-1.out: $(CLI1_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

cli-1-par.out: $(CLI1PAR_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

test: C run_front

# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
