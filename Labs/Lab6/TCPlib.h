#include <stdint.h>

#ifndef _TCPlib_H
#define _TCPlib_H

int TCPconnect(unsigned long IP, unsigned short port);
int TCPsend(int fd, uint8_t *message, unsigned int len);
int TCPrecv(int fd, uint8_t *str, unsigned int len);
int TCPclose(int fd);
int TCPcreate(unsigned long IP, unsigned short port);
int TCPaccept(int fd);

#endif
