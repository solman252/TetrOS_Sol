#ifndef KERNEL_H
#define KERNEL_H

//vga text mode color attribute
#define WHITE_TXT 0x0F
#define YELLOW_TXT 0x0E
#define CYAN_TXT 0x0B
#define RED_TXT 0x0C
#define GREEN_TXT 0x02
#define ORANGE_TXT 0x06
#define PINK_TXT 0x0D
#define PURPLE_TXT 0x05

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
void init_keyboard();

//kernel main function
void k_main();

#endif
