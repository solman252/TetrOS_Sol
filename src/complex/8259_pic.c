#include "complex/isr.h"
#include "complex/idt.h"
#include "complex/io_ports.h"
#include "complex/8259_pic.h"

void pic8259_init() {
    uchar a1, a2;

    //save mask registers
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    //send commands to pic to initialize both master & slave
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);

    //map vector offset of all default irqs from 0x20 to 0x27 in master (ICW2)
    outb(PIC1_DATA, 0x20);
    // map vector offset of all default irq's from 0x28 to 0x2F in slave (ICW2)
    outb(PIC2_DATA, 0x28);

    // tell master pic that there is a slave pic at irq2  (0000 0100)
    outb(PIC1_DATA, 4);
    // tell slave pic its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);

    //make set x86
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    //restore the mask registers
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

//send eoi
void pic8259_eoi(uchar irq) {
    if(irq >= 0x28)
        outb(PIC2, PIC_EOI);
    outb(PIC1, PIC_EOI);
}

