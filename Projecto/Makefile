CC = gcc
CFLAGS = -Wall
CLIENT_OBJ = client.c inetutils.o TCPlib.o psiskv_lib.o
FRONT_SERVER_OBJ = front-server.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o
DATA_SERVER_OBJ = data-server.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o
EDITOR = gedit

all: main client

main: front_server data_server

front_server: $(FRONT_SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

data_server: $(DATA_SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -pthread

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

c:
	clear
	@rm front_server || true
	@rm data_server || true
	@rm client || true
	@rm *.o || true

o:
	$(EDITOR) dd.c *.c *.h makefile &

# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
