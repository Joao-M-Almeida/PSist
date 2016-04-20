CC = gcc
CFLAGS = -Wall
DEPS =
OBJ =
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
