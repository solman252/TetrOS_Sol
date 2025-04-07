//setup idt

#ifndef IDT_H
#define IDT_H

#include "types.h"

#define NO_IDT_DESCRIPTORS     256

typedef struct {
    ushort base_low;          //lower 16 bits 0 to 15 of the address to jump to when this interrupt runs
    ushort segment_selector;  //code segment selector in gtd
    uchar zero;               //always be zero bc unused
    uchar type;               //types trap interrupt gates
    ushort base_high;         //upper 16 bits 16 31 of the address to jump to
} __attribute__((packed)) IDT;

typedef struct {
    ushort limit;         //limit size of all idt segments
    uint base_address;  // base address of the first idt segment
} __attribute__((packed)) IDT_PTR;


extern void load_idt(uint idt_ptr);

//fill
void idt_set_entry(int index, uint base, ushort seg_sel, uchar flags);

void idt_init();

#endif
