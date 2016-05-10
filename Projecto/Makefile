CC = gcc
CFLAGS = -Wall
DEPS = debug.h item.h phash-lib.h inetutils.h TCPlib.h psiskv.h psiskv_server.h
OBJ = front-server.o data-server.o phash-lib.o inetutils.o TCPlib.o psiskv_lib.o psiskv_server.o
EDITOR = gedit

main: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

c:
	clear
	rm *.o

o:
	$(EDITOR) dd.c *.c *.h makefile &




# $@ reffers to named before :
# $^ reffers to all named after :
# $< reffers to first after :
