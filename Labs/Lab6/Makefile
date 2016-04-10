main_c = Lab6.c
main_file = main
server_c = server.c
server_file = server
client_c = client.c
client_file = client

inet: inetutils.c inetutils.h
	gcc -c inetutils.c -o inetutils.o -Wall

tcp: TCPlib.c TCPlib.h
	gcc -c TCPlib.c -o TCPlib.o -Wall

hash: hash-lib.c hash-lib.h
	gcc -c hash-lib.c -o hash-lib.o -Wall

server: hash tcp inet $(server_c)
	gcc $(server_c) -o $(server_file) TCPlib.o inetutils.o hash-lib.o

server_run: server
	./server

client: tcp inet $(client_c)
	gcc $(client_c) -o $(client_file) TCPlib.o inetutils.o

client_run: client
	./client

main: hash tcp inet $(main_c)
	gcc $(main_c) -o $(main_file) TCPlib.o inetutils.o hash-lib.o
