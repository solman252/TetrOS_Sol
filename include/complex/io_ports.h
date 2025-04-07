#ifndef IO_PORTS_H
#define IO_PORTS_H

#include "types.h"

//read a byte from given port number
uchar inb(ushort port);

//write a given byte to given port number
void outb(ushort port, uchar val);

//read 2 bytes(short) from given port number
ushort inports(ushort port);

//write given 2(short) bytes to given port number
void outports(ushort port, ushort data);

//read 4 bytes(long) from given port number
uint inportl(ushort port);

//write given 4 bytes(long) to given port number
void outportl(ushort port, uint data);

#endif
