#ifndef KERNEL_H
#define KERNEL_H

//vga text mode color attribute
#define MAGENTA_TXT 0x05
#define BLACK_TXT 0x00
#define BLUE_TXT 0x01
#define GREEN_TXT 0x02
#define CYAN_TXT 0x03
#define RED_TXT 0x04
#define MAGENTA_TXT 0x05
#define BROWN_TXT 0x06
#define LIGHT_GRAY_TXT 0x07
#define GRAY_TXT 0x8
#define LIGHT_BLUE_TXT 0x09
#define LIGHT_GREEN_TXT 0x0A
#define LIGHT_CYAN_TXT 0x0B
#define LIGHT_RED_TXT 0x0C
#define LIGHT_MAGENTA_TXT 0x0D
#define LIGHT_YELLOW_TXT 0x0E
#define WHITE_TXT 0x0F

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
