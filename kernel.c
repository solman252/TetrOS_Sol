#include "kernel.h"

const int grid_width = 10;
const int grid_height = 20;
const int grid_start_line = 5;

int grid_sel_x = 0;
int grid_sel_y = 0;
int square_mode = 0;

volatile unsigned int tick_count = 0;

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
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

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
    } while (num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') {
        low++;
    }
    char *high = ptr - 1;
    while (low < high) {
        char temp = *low;
        *low++ = *high;
        *high-- = temp;
    }
}

void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_remap() { //remap pics to avoid irq conflicts
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xA1);
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, a1);
    outb(0xA1, a2);
}

struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

extern void idt_load(unsigned int);

void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (unsigned int)&idt;
    extern void timer_handler_stub();
    extern void keyboard_handler_stub();
    idt_set_gate(32, (unsigned int)timer_handler_stub, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)keyboard_handler_stub, 0x08, 0x8E);
    idt_load((unsigned int)&idtp);
}

char[] shape_o = "11-*1|11-*1|11-*1|11-*1";
char[] shape_i = "1-*-1-1|11*1|1-1-*-1|1*11";
char[] shape_s = "011-1*0|10-*1-01|0*1-110|10-1*-01";
char[] shape_z = "110-0*1|01-*1-10|1*0-011|01-1*-10";
char[] shape_l = "10-*0-11|1*1-100|11-0*-01|001-1*1";
char[] shape_j = "01-0*-11|100-1*1|11-*0-10|1*1-001";
char[] shape_t = "1*1-010|01-1*-01|010-1*1|10-*1-10";

void draw_grid() {
    for (int y = 0; y < grid_height; y++) {
        char buffer[256];
        int pos = 0;
        for (int x = 0; x < grid_width; x++) {
            if (y == grid_sel_y && x == grid_sel_x) {
                switch(square_mode){
                    case 0:
                        buffer[pos++] = '[';
                        buffer[pos++] = ']';
                        break;
                    case 1:
                        buffer[pos++] = ' ';
                        buffer[pos++] = 0xdc;
                        break;
                }
            } else {
                buffer[pos++] = ' ';
                buffer[pos++] = '.';
            }
        }
        buffer[pos] = '\0';
        k_printf(buffer, grid_start_line + y);
    }
}

void timer_handler() {
    tick_count++;
    if (tick_count % 18 == 0) {
        static unsigned int seconds = 0;
        seconds++;
        char buffer[16];
        itoa(seconds, buffer, 10);
        int i = 0;
        while (buffer[i]) i++;
        buffer[i] = 's';
        buffer[i+1] = '\0';
        k_printf(buffer, 2);
    }
    outb(0x20, 0x20);
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    switch(scancode) {
        case 0x4B:
            if (grid_sel_x > 0)
                grid_sel_x--;
            break;
        case 0x4D:
            if (grid_sel_x < grid_width - 1)
                grid_sel_x++;
            break;
        case 0x48:
            if (grid_sel_y > 0)
                grid_sel_y--;
            break;
        case 0x50:
            if (grid_sel_y < grid_height - 1)
                grid_sel_y++;
            break;
        case 0x3B:
            switch(square_mode) {
                case 0:
                    square_mode = 1;
                    break;
                case 1:
                    square_mode = 0;
                    break;
            };
            break;
        default:
            break;
    }
    draw_grid();
    outb(0x20, 0x20);
}

void init_timer() {
}

void init_keyboard() {
}

void kernel_loop() {
    while (1) {
        asm volatile ("hlt");
    }
}

void k_main() {
    k_clear_screen();
    disable_cursor();
    k_printf("it works. welcome to tetros", 0);
    draw_grid();
    pic_remap();
    k_install_idt();
    init_timer();
    init_keyboard();
    asm volatile ("sti");
    kernel_loop();
}
