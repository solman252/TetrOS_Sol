//gtd setup

#ifndef GDT_H
#define GDT_H

#include "types.h"

#define NO_GDT_DESCRIPTORS     8

typedef struct {
    ushort segment_limit;  //segment limit first 0-15 bits
    ushort base_low;       //base first 0-15 bits
    uchar base_middle;     //base 16-23 bits
    uchar access;          //access byte
    uchar granularity;     //high 4 bits (flags) low 4 bits (limit 4 last bits)(limit is 20 bit wide)
    uchar base_high;       //base 24-31 bits
} __attribute__((packed)) GDT;

typedef struct {
    ushort limit;       //limit size of all GDT segments
    uint base_address;  //base address of the first GDT segment
} __attribute__((packed)) GDT_PTR;

//asm gdt functions in load_gdt.asm
extern void load_gdt(uint gdt_ptr);

//fill enteries
void gdt_set_entry(int index, uint base, uint limit, uchar access, uchar gran);

// initialize it
void gdt_init();

#endif
