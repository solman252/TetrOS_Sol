#ifndef KERNEL_H
#define KERNEL_H

// VGA text mode color attribute
#define WHITE_TXT 0x07

// --- VGA Functions ---
void k_clear_screen();
unsigned int k_printf(char *message, unsigned int line);
void disable_cursor();

// --- Utility Functions ---
void itoa(int value, char *str, int base);

// --- I/O Port Functions ---
void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

// --- PIC & Interrupt Setup ---
void pic_remap();
void k_install_idt();
void idt_load(unsigned int);

// --- Device Driver Initialization ---
void init_timer();
void init_keyboard();

// --- Kernel Main Function ---
void k_main();

#endif // KERNEL_H
