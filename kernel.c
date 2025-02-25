#include "kernel.h"
#include "keyboard.h"

volatile unsigned int tick_count = 0;
volatile unsigned int seconds_value = 0;
char timer_str[16] = "0s";

char input_buffer[256] = {0};
int input_index = 0;

int active_region = 0;

#define GRID_COLS 20
#define GRID_ROWS 10
int grid_sel_x = 0;
int grid_sel_y = 0

void k_clear_screen() {
    char *vidmem = (char *)0xb8000;
    for (unsigned int i = 0; i < 80 * 25 * 2; i += 2) {
        vidmem[i] = ' ';
        vidmem[i+1] = WHITE_TXT;
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
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x20);
}

void k_clear_line(unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    for (int j = 0; j < 80; j++) {
        vidmem[i] = ' ';
        vidmem[i+1] = WHITE_TXT;
        i += 2;
    }
}

struct region regions[4] = {
    {0, 11, 0, 39},
    {0, 11, 40, 79},
    {12, 24, 0, 39},
    {12, 24, 40, 79}
};

void clear_region(int region_index, unsigned char attribute) {
    struct region r = regions[region_index];
    char *vidmem = (char *)0xb8000;
    for (int row = r.row_start; row <= r.row_end; row++) {
        for (int col = r.col_start; col <= r.col_end; col++) {
            int index = (row * 80 + col) * 2;
            vidmem[index] = ' ';
            vidmem[index+1] = attribute;
        }
    }
}

void region_print(int region_index, unsigned int row_offset, unsigned int col_offset, char *message, unsigned char attribute) {
    struct region r = regions[region_index];
    int row = r.row_start + row_offset;
    int col = r.col_start + col_offset;
    char *vidmem = (char *)0xb8000;
    while (*message && row <= r.row_end) {
        if (col > r.col_end) {
            col = r.col_start;
            row++;
            if (row > r.row_end)
                break;
        }
        int index = (row * 80 + col) * 2;
        vidmem[index] = *message;
        vidmem[index+1] = attribute;
        col++;
        message++;
    }
}

// draw grid for region 2
void draw_grid() {
    char row_buffer[41];
    row_buffer[40] = '\0';
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            if (row == grid_sel_y && col == grid_sel_x) {
                row_buffer[col*2] = '[';
                row_buffer[col*2+1] = ']';
            } else {
                row_buffer[col*2] = ' ';
                row_buffer[col*2+1] = '.';
            }
        }
        region_print(2, row, 0, row_buffer, (active_region == 2) ? BLUE_BG : WHITE_TXT);
    }
}

void update_all_regions() {
    int attr;
    attr = (active_region == 0) ? BLUE_BG : WHITE_TXT;
    clear_region(0, attr);
    region_print(0, 0, 0, input_buffer, attr);
    attr = (active_region == 1) ? BLUE_BG : WHITE_TXT;
    clear_region(1, attr);
    region_print(1, 0, 0, timer_str, attr);
    clear_region(2, (active_region == 2) ? BLUE_BG : WHITE_TXT);
    draw_grid();
    attr = (active_region == 3) ? BLUE_BG : WHITE_TXT;
    clear_region(3, attr);
    region_print(3, 0, 0, "extra", attr);
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
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[num % base];
        num /= base;
    } while (num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') { low++; }
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

void pic_remap() {
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xa1);
    outb(0x20, 0x11);
    outb(0xa0, 0x11);
    outb(0x21, 0x20);
    outb(0xa1, 0x28);
    outb(0x21, 0x04);
    outb(0xa1, 0x02);
    outb(0x21, 0x01);
    outb(0xa1, 0x01);
    outb(0x21, a1);
    outb(0xa1, a2);
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
    idt[num].base_low = base & 0xffff;
    idt[num].base_high = (base >> 16) & 0xffff;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

extern void idt_load(unsigned int);

void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base = (unsigned int)&idt;
    extern void timer_handler_stub();
    extern void keyboard_handler_stub();
    idt_set_gate(32, (unsigned int)timer_handler_stub, 0x08, 0x8e);
    idt_set_gate(33, (unsigned int)keyboard_handler_stub, 0x08, 0x8e);
    idt_load((unsigned int)&idtp);
}

// timer interrupt handler; sends eoi
void timer_handler() {
    tick_count++;
    if (tick_count % 18 == 0) {
        seconds_value++;
        char buf[16];
        itoa(seconds_value, buf, 10);
        int i = 0;
        while (buf[i]) i++;
        buf[i] = 's';
        buf[i+1] = '\0';
        int j = 0;
        while (buf[j]) {
            timer_str[j] = buf[j];
            j++;
        }
        timer_str[j] = '\0';
        update_all_regions();
    }
    outb(0x20, 0x20);
}

void keyboard_handle_key(char ascii) {
    if (active_region == 0) {
        if (ascii == '\b') {
            if (input_index > 0) {
                input_index--;
                input_buffer[input_index] = '\0';
            }
        } else if (ascii == '\n') {
            input_index = 0;
            input_buffer[0] = '\0';
        } else {
            if (input_index < 255) {
                input_buffer[input_index++] = ascii;
                input_buffer[input_index] = '\0';
            }
        }
        update_all_regions();
    } else if (active_region == 2) {
        if (ascii == 'w' || ascii == 'W') {
            if (grid_sel_y > 0)
                grid_sel_y--;
        } else if (ascii == 's' || ascii == 'S') {
            if (grid_sel_y < GRID_ROWS - 1)
                grid_sel_y++;
        } else if (ascii == 'a' || ascii == 'A') {
            if (grid_sel_x > 0)
                grid_sel_x--;
        } else if (ascii == 'd' || ascii == 'D') {
            if (grid_sel_x < GRID_COLS - 1)
                grid_sel_x++;
        }
        update_all_regions();
    }
}

void keyboard_handle_special(key_code_t key) {
    switch(key) {
        case KEY_UP:
            if (active_region == 3) active_region = 1;
            break;
        case KEY_DOWN:
            if (active_region == 0) active_region = 2;
            else if (active_region == 1) active_region = 3;
            break;
        case KEY_LEFT:
            if (active_region == 1) active_region = 0;
            else if (active_region == 3) active_region = 2;
            break;
        case KEY_RIGHT:
            if (active_region == 0) active_region = 1;
            else if (active_region == 2) active_region = 3;
            break;
        default:
            break;
    }
    update_all_regions();
}

void init_timer() {
}

void init_keyboard() {
    init_keyboard_driver();
}

void kernel_loop() {
    while (1) {
        asm volatile ("hlt");
    }
}

// kernel main; entry point
void k_main() {
    k_clear_screen();
    disable_cursor();
    update_all_regions();
    pic_remap();
    k_install_idt();
    init_timer();
    init_keyboard();
    asm volatile ("sti");
    kernel_loop();
}
