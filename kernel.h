#ifndef KERNEL_H
#define KERNEL_H

//vga color attributes
#define WHITE_TXT 0x07      //white on black (inactive)
#define BLUE_BG   0x1F      //white on blue (active)

//vga functions
void k_clear_screen();
unsigned int k_printf(char *message, unsigned int line);
void disable_cursor();
void k_clear_line(unsigned int line);

//region functions
struct region {
    int row_start;
    int row_end;
    int col_start;
    int col_end;
};
void clear_region(int region_index, unsigned char attribute);
void region_print(int region_index, unsigned int row_offset, unsigned int col_offset, char *message, unsigned char attribute);
void update_all_regions();

//utility functions
void itoa(int value, char *str, int base);

//i/o port functions
void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

//pic & interrupt setup
void pic_remap();
void k_install_idt();
void idt_load(unsigned int);

//device driver initialization
void init_timer();
void init_keyboard();

//input / navigation functions
void process_input(unsigned char scancode);
char scancode_to_ascii(unsigned char scancode);

//kernel main & loop
void k_main();
void kernel_loop();

#endif //KERNEL_H
