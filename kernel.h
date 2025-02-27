#ifndef KERNEL_H
#define KERNEL_H

//vga text mode color attribute
#define bool _Bool
#define true 1
#define false 0

//vga functions
void k_clear_screen();
unsigned int k_printf(char *message, unsigned int line);
void disable_cursor();

//utility functions
void itoa(int value, char *str, int base);

//io port functions
void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

//pic and interrupt setup
void pic_remap();
void k_install_idt();
void idt_load(unsigned int);

//device driver initialization
void init_timer();

//kernel main function
void k_main();

#endif
