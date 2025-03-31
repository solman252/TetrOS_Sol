//setup idt

#ifndef IDT_H
#define IDT_H

#include "types.h"

#define NO_IDT_DESCRIPTORS     256

typedef struct {
    uint16 base_low;          //lower 16 bits 0 to 15 of the address to jump to when this interrupt runs
    uint16 segment_selector;  //code segment selector in gtd
    uint8 zero;               //always be zero bc unused
    uint8 type;               //types trap interrupt gates
    uint16 base_high;         //upper 16 bits 16 31 of the address to jump to
} __attribute__((packed)) IDT;

typedef struct {
    uint16 limit;         //limit size of all idt segments
    uint32 base_address;  // base address of the first idt segment
} __attribute__((packed)) IDT_PTR;


extern void load_idt(uint32 idt_ptr);

//fill
void idt_set_entry(int index, uint32 base, uint16 seg_sel, uint8 flags);

void idt_init();

#endif
