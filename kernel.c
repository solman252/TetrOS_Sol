#include "kernel.h"

// using the provided color macros:
// BLACK_TXT    0x00
// BLUE_TXT     0x01
// GREEN_TXT    0x02
// CYAN_TXT     0x03
// RED_TXT      0x04 
// MAGENTA_TXT  0x05
// BROWN_TXT    0x06
// LIGHT_GRAY_TXT 0x07
// GRAY_TXT     0x08
// LIGHT_BLUE_TXT 0x09
// LIGHT_GREEN_TXT 0x0A
// LIGHT_CYAN_TXT 0x0B
// LIGHT_RED_TXT  0x0C
// LIGHT_MAGENTA_TXT 0x0D
// LIGHT_YELLOW_TXT  0x0E
// WHITE_TXT    0x0F

const int grid_width = 10;
const int grid_height = 20;
const int grid_start_line = 5;

int grid_sel_x = 0;
int grid_sel_y = 0;
int current_shape = 0;
int square_mode = 0;
int highlight_bg = 1; // 0 = normal, 1 = highlighted

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

unsigned int k_set_color_at(unsigned char attr, unsigned int line, unsigned int pos) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 1 + (line * 80 * 2) + (2 * pos);
    vidmem[i] = attr;
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

// Shapes as defined previously
int shape_o[4][4][4] = {
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,1,0,0}, {0,0,0,0}, {0,0,0,0} }
};
int shape_i[4][4][4] = {
    { {1,0,0,0}, {2,0,0,0}, {1,0,0,0}, {1,0,0,0} },
    { {1,1,2,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {1,0,0,0}, {2,0,0,0}, {1,0,0,0} },
    { {1,2,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} }
};
int shape_s[4][4][4] = {
    { {0,1,1,0}, {1,2,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,1,0,0}, {0,1,0,0}, {0,0,0,0} },
    { {0,2,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {1,2,0,0}, {0,1,0,0}, {0,0,0,0} }
};
int shape_z[4][4][4] = {
    { {1,1,0,0}, {0,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {2,1,0,0}, {1,0,0,0}, {0,0,0,0} },
    { {1,2,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {1,2,0,0}, {1,0,0,0}, {0,0,0,0} }
};
int shape_l[4][4][4] = {
    { {1,0,0,0}, {2,0,0,0}, {1,1,0,0}, {0,0,0,0} },
    { {1,2,1,0}, {1,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {0,2,0,0}, {0,1,0,0}, {0,0,0,0} },
    { {0,0,1,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} }
};
int shape_j[4][4][4] = {
    { {0,1,0,0}, {0,2,0,0}, {1,1,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,1,0,0}, {2,0,0,0}, {1,0,0,0}, {0,0,0,0} },
    { {1,2,1,0}, {0,0,1,0}, {0,0,0,0}, {0,0,0,0} }
};
int shape_t[4][4][4] = {
    { {1,2,1,0}, {0,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {1,2,0,0}, {0,1,0,0}, {0,0,0,0} },
    { {0,1,0,0}, {1,2,1,0}, {0,0,0,0}, {0,0,0,0} },
    { {1,0,0,0}, {2,1,0,0}, {1,0,0,0}, {0,0,0,0} }
};

//
// draw_grid now centers the board and draws an outline around the grid
// using double-line box drawing characters from code page 437:
// ╔ = 0xC9 ═ = 0xCD ╗ = 0xBB, ║ = 0xBA, ╚ = 0xC8 and ╝ = 0xBC
//
void draw_grid() {
    int board_width = grid_width * 2 + 2;
    int board_height = grid_height + 2;
    int left_margin = (80 - board_width) / 2;
    int top_margin = (25 - board_height) / 2;
    
    char line[256];
    int pos, i;
    
    // draw top border using ╔, ═, ╗
    pos = 0;
    for (i = 0; i < left_margin; i++) {
        line[pos++] = ' ';
    }
    line[pos++] = 0xC9;  // ╔
    for (i = 0; i < board_width - 2; i++) {
        line[pos++] = 0xCD;  // ═
    }
    line[pos++] = 0xBB;  // ╗
    line[pos] = '\0';
    k_printf(line, top_margin);
    
    // draw grid rows with vertical borders using ║
    for (int r = 0; r < grid_height; r++) {
        pos = 0;
        for (i = 0; i < left_margin; i++) {
            line[pos++] = ' ';
        }
        line[pos++] = 0xBA;  // ║
        for (int c = 0; c < grid_width; c++) {
            if (r == grid_sel_y && c == grid_sel_x) {
                if (square_mode == 0) {
                    line[pos++] = '[';
                    line[pos++] = ']';
                } else {
                    line[pos++] = ' ';
                    line[pos++] = 0xdc;
                }
            } else {
                line[pos++] = ' ';
                line[pos++] = '.';
            }
        }
        line[pos++] = 0xBA;  // ║
        line[pos] = '\0';
        k_printf(line, top_margin + 1 + r);
    }
    
    // draw bottom border using ╚, ═, ╝
    pos = 0;
    for (i = 0; i < left_margin; i++) {
        line[pos++] = ' ';
    }
    line[pos++] = 0xC8;  // ╚
    for (i = 0; i < board_width - 2; i++) {
        line[pos++] = 0xCD;  // ═
    }
    line[pos++] = 0xBC;  // ╝
    line[pos] = '\0';
    k_printf(line, top_margin + board_height - 1);
    
    // update the attribute for the selected cell
    int sel_line = top_margin + 1 + grid_sel_y;
    int sel_col = left_margin + 1 + grid_sel_x * 2;
    
    // choose the color based on current_shape using existing macros
    char col = WHITE_TXT;
    switch(current_shape) {
        case 0: col = LIGHT_YELLOW_TXT; break;
        case 1: col = CYAN_TXT; break;
        case 2: col = RED_TXT; break;
        case 3: col = GREEN_TXT; break;
        case 4: col = BROWN_TXT; break;
        case 5: col = LIGHT_MAGENTA_TXT; break;
        case 6: col = MAGENTA_TXT; break;
    }
    unsigned char attr;
    if (highlight_bg) {
        attr = (col << 4) | WHITE_TXT;
    } else {
        attr = col;
    }
    
    // update video memory for the two characters in the selected cell
    char *vidmem = (char *)0xb8000;
    unsigned int offset = (sel_line * 80 + sel_col) * 2 + 1;
    vidmem[offset] = attr;
    offset = (sel_line * 80 + sel_col + 1) * 2 + 1;
    vidmem[offset] = attr;
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
        case 0x4B: // left key
            if (grid_sel_x > 0)
                grid_sel_x--;
            break;
        case 0x4D: // right key
            if (grid_sel_x < grid_width - 1)
                grid_sel_x++;
            break;
        case 0x48: // down key (moves grid up)
            if (grid_sel_y > 0)
                grid_sel_y--;
            break;
        case 0x50: // up key (moves grid down)
            if (grid_sel_y < grid_height - 1)
                grid_sel_y++;
            break;
        case 0x3B: // f1 key to change current shape
            current_shape++;
            if (current_shape >= 7)
                current_shape = 0;
            break;
        case 0x3C: // f2 key to toggle highlight/background color
            highlight_bg = !highlight_bg;
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
