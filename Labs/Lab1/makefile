CC= gcc
CFLAGS= -Wall -pedantic -ansi
DEPS = test.h
OBJ = test1.o prog1.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

prog1: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
