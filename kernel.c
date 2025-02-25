#include "kernel.h"

//global tick counter for timer interrupts
volatile unsigned int tick_count = 0

//vga text mode functions
void k_clear_screen() {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE_TXT;
    }
}

unsigned int k_printf(char *message, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = WHITE_TXT;
        }
    }
    return 1;
}

void disable_cursor() {
    //disable vga hardware text cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

//utility function itoa converts an integer to a string base 10
void itoa(int value, char *str, int base) {
    char *rc = str, *ptr = str, *low;
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    int num = value;
    do {
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base];
        num /= base;
    } while(num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') { low++; }
    char *high = ptr - 1;
    while(low < high) {
        char temp = *low;
        *low++ = *high;
        *high-- = temp;
    }
}

//io port functions
void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

//pic remapping remaps the pic so that irqs start at vector 32
void pic_remap() {
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xA1);

    outb(0x20, 0x11);  //start initialization in cascade mode
    outb(0xA0, 0x11);
    outb(0x21, 0x20);  //master pic vector offset
    outb(0xA1, 0x28);  //slave pic vector offset
    outb(0x21, 0x04);  //tell master pic about slave pic at irq2
    outb(0xA1, 0x02);  //tell slave pic its cascade identity
    outb(0x21, 0x01);  //set pics to 8086 88 mode
    outb(0xA1, 0x01);

    //restore saved masks
    outb(0x21, a1);
    outb(0xA1, a2);
}

//idt structures and setup
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;      //kernel segment selector
    unsigned char  always0;
    unsigned char  flags;    //present ring 0 32 bit interrupt gate
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

//set an entry in the idt
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

//declaration for assembly routine to load the idt
extern void idt_load(unsigned int);

void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (unsigned int)&idt;
    
    //external assembly stubs for interrupt handlers
    extern void timer_handler_stub();
    extern void keyboard_handler_stub();
    
    //map irq0 timer to vector 32 and irq1 keyboard to vector 33
    idt_set_gate(32, (unsigned int)timer_handler_stub, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)keyboard_handler_stub, 0x08, 0x8E);
    
    idt_load((unsigned int)&idtp);
}

//timer interrupt handler increments a seconds counter every 18 ticks roughly one second
void timer_handler() {
    tick_count++;
    if (tick_count % 18 == 0) {
        static unsigned int seconds = 0;
        seconds++;
        char buffer[16];
        itoa(seconds, buffer, 10);
        //append s to display seconds eg 1s 2s etc
        int i = 0;
        while (buffer[i]) i++;
        buffer[i] = 's';
        buffer[i+1] = '\0';
        k_printf(buffer, 22);  //display on line 22
    }
    //send endofinterrupt signal
    outb(0x20, 0x20);
}

//keyboard interrupt handler future convert scancodes to key events
void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    k_printf("Key pressed", 23);
    outb(0x20, 0x20);
}

//device driver initializations
void init_timer() {
    //future configure the pit if needed
}

void init_keyboard() {
    //future initialize keyboard state or scancode translation
}

//kernel main loop serves as placeholder for future event processing scheduling or game updates
void kernel_loop() {
    while (1) {
        //future dispatch events update game state etc
        asm volatile ("hlt");
    }
}

//kernel main function
void k_main() {
    k_clear_screen();
    disable_cursor();
    k_printf("IT WORKS. Welcome to TetrOS", 0);

    //system initialization
    pic_remap();
    k_install_idt();
    init_timer();
    init_keyboard();
    
    //enable interrupts
    asm volatile ("sti");
    
    //enter main loop
    kernel_loop();
}
