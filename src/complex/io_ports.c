#include "complex/io_ports.h"

uchar inb(ushort port) {
    uchar ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outb(ushort port, uchar val) {
    asm volatile("outb %1, %0" :: "dN"(port), "a"(val));
}

ushort inports(ushort port) {
    ushort rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outports(ushort port, ushort data) {
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}

uint inportl(ushort port) {
    uint rv;
    asm volatile ("inl %%dx, %%eax" : "=a" (rv) : "dN" (port));
    return rv;
}

void outportl(ushort port, uint data) {
    asm volatile ("outl %%eax, %%dx" : : "dN" (port), "a" (data));
}

