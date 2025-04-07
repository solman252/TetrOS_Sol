//8259 pic setup

#ifndef _8259_PIC_H
#define _8259_PIC_H

#include "types.h"

//READ https://wiki.osdev.org/8259_PIC IF YOU WANNA UNDERSTAND!!!!! im not explaining
#define PIC1            0x20  
#define PIC2            0xA0  
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)    //master data 
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)    //slave data 

#define PIC_EOI         0x20   //eoi

#define ICW1            0x11    //like the interrupt control command word pic for initlization
#define ICW4_8086       0x01    


//init pic with irq defined in isr.h
void pic8259_init();

//send end of interrupt command to PIC 8259

void pic8259_eoi(uchar irq);

#endif

