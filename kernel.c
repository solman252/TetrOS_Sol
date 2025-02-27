#include "kernel.h"

const int grid_width = 10;
const int grid_height = 20;

int grid_sel_x = 0;
int grid_sel_y = 0;
int current_shape = 0;
int current_rot = 0;
bool highlight_bg = true;
bool should_stamp = false;

volatile unsigned int tick_count = 0;

void set_vga_palette(int color_index, int r, int g, int b) {
    outb(0x3C8, color_index); // Select the color index to modify
    outb(0x3C9, r & 0x3F);    // Set red component (0-63)
    outb(0x3C9, g & 0x3F);    // Set green component (0-63)
    outb(0x3C9, b & 0x3F);    // Set blue component (0-63)
}

void set_custom_palette() {
    set_vga_palette(0,  0,  0,  0);   // BLACK
    set_vga_palette(1,  63, 63, 63);   // WHITE
    set_vga_palette(2,  32, 32, 32);   // GRAY
    set_vga_palette(3,  0,  63, 63);   // CYAN
    set_vga_palette(4,  0,  0,  63);  // BLUE
    set_vga_palette(5,  63, 32, 0);  // ORANGE
    set_vga_palette(6,  63, 0, 0);  // YELLOW
    set_vga_palette(7,  0,  63, 0);  // GREEN
    set_vga_palette(8,  63, 0,  63);  // PURPLE
    set_vga_palette(9,  63, 0,  0);  // RED
    set_vga_palette(10, 16, 63, 63);  // LIGHT_CYAN
    set_vga_palette(11, 63, 48, 16);  // LIGHT_ORANGE
    set_vga_palette(12, 63, 63, 16);  // LIGHT_YELLOW
    set_vga_palette(13, 16, 63, 16);  // LIGHT_GREEN
    set_vga_palette(14, 63, 16, 63);  // LIGHT_PURPLE
    set_vga_palette(15, 63, 16, 16);  // LIGHT_RED
}
#define BLACK 0x00
#define WHITE 0x01
#define GRAY 0x02
#define CYAN 0x03
#define BLUE 0x04
#define ORANGE 0x05
#define YELLOW 0x06
#define GREEN 0x07
#define PURPLE 0x08
#define RED 0x09
#define LIGHT_CYAN 0x0A
#define LIGHT_BLUE 0x03 // lgiht blue = cyan
#define LIGHT_ORANGE 0x0B
#define LIGHT_YELLOW 0x0C
#define LIGHT_GREEN 0x0D
#define LIGHT_PURPLE 0x0E
#define LIGHT_RED 0x0F

void k_clear_screen() {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE;
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
            vidmem[i++] = WHITE;
        }
    }
    return 1;
}
unsigned int k_printf_at(char *message, unsigned int line, unsigned int pos) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = (line * 80 * 2) + (2 * pos);
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = WHITE;
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



int tilemap[20][10];

//
// draw_grid now centers the board and draws an outline around the grid
// using double-line box drawing characters from code page 437:
// ╔ = 0xC9 ═ = 0xCD ╗ = 0xBB, ║ = 0xBA, ╚ = 0xC8 and ╝ = 0xBC
//
void draw_grid() {
    // shape_t[0][0][0] = 5;
    int board_width = grid_width * 2 + 2;
    int board_height = grid_height + 2;
    int left_margin = (80 - board_width) / 2;
    int top_margin = (27 - board_height) / 2;
    
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
    
    // get shape
    int (*rot)[4];
    switch(current_shape) {
        case 0:
            rot = shape_o[current_rot];
            break;
        case 1:
            rot = shape_i[current_rot];
            break;
        case 2:
            rot = shape_s[current_rot];
            break;
        case 3:
            rot = shape_z[current_rot];
            break;
        case 4:
            rot = shape_l[current_rot];
            break;
        case 5:
            rot = shape_j[current_rot];
            break;
        case 6:
            rot = shape_t[current_rot];
            break;
        default:
            rot = shape_o[current_rot];
            break;
    };
    int center_x;
    int center_y;
    int points[4][2];
    i = 0;
    for (int y = 0; y < 4; y++) {
        int *row = rot[y];
        for (int x = 0; x < 4; x++) {
            int ch = row[x];
            int point[2] = {x,y};
            if (ch == 1 || ch == 2) {
                points[i][0] = x;
		points[i][1] = y;
		i++;
            }
            if (ch == 2) {
                center_x = x;
                center_y = y;
            }
        }
    }
    int adjusted_points[4][2];
    i = 0;
    for (int index = 0; index <= 4; index++) {
        adjusted_points[i][0] = points[index][0]-center_x+grid_sel_x;
        adjusted_points[i][1] = points[index][1]-center_y+grid_sel_y;
        i++;
    }

    // draw grid rows with vertical borders using ║
    for (int r = 0; r < grid_height; r++) {
        pos = 0;
        for (i = 0; i < left_margin; i++) {
            line[pos++] = ' ';
        }
        line[pos++] = 0xBA;  // ║
        for (int c = 0; c < grid_width; c++) {
            bool in_points = false;
            for(i=0;i<=4;i++) {
                if (c == adjusted_points[i][0] && r == adjusted_points[i][1]) {
                    in_points = true;
                    break;
                }
            }
            if (in_points || tilemap[r][c] > 0) {
                line[pos++] = '[';
                line[pos++] = ']';
            } else {
                line[pos++] = 0xc4;
                line[pos++] = 0xb4;
            }
        }
        line[pos++] = 0xBA;  // ║
        line[pos] = '\0';
        k_printf(line, top_margin + 1 + r);
        i=0;
        for(int p = 0;p < 4;p++){
            for(int eee = 0;eee < 2;eee++){
                char buffer[16];
                itoa(adjusted_points[p][eee], buffer, 10);
                k_printf(buffer, 2+i);
                i++;
            }
        }
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
    
    for (int y = 0; y < grid_height; y++) {
        for (int x = 0; x < grid_width; x++) {
            int p = tilemap[y][x];
            char col = GRAY;
            switch(p) {
                case 0: col = GRAY; break;
                case 1: col = YELLOW; break;
                case 2: col = CYAN; break;
                case 3: col = GREEN; break;
                case 4: col = RED; break;
                case 5: col = ORANGE; break;
                case 6: col = BLUE; break;
                case 7: col = PURPLE; break;
            }
            unsigned char attr;
            if (highlight_bg && p != 0) {
                attr = (col << 4) | BLACK;
            } else {
                attr = col;
            }
            int sel_line = top_margin + 1 + y;
            int sel_col = left_margin + 1 + x * 2;
            // update video memory for the two characters in the selected cell
            char *vidmem = (char *)0xb8000;
            unsigned int offset = (sel_line * 80 + sel_col) * 2 + 1;
            vidmem[offset] = attr;
            offset = (sel_line * 80 + sel_col + 1) * 2 + 1;
            vidmem[offset] = attr;
        }
    }

    // choose the color based on current_shape using existing macros
    char col = WHITE;
    switch(current_shape) {
        case 0: col = YELLOW; break; // shape_o
        case 1: col = CYAN; break; // shape_i
        case 2: col = GREEN; break; // shape_s
        case 3: col = RED; break; // shape_z
        case 4: col = ORANGE; break; // shape_l
        case 5: col = BLUE; break; // shape_j
        case 6: col = PURPLE; break; // shape_t
    }
    unsigned char attr;
    if (highlight_bg) {
        attr = (col << 4) | BLACK;
    } else {
        attr = col;
    }
    bool persis_should_stamp = should_stamp;
    if (should_stamp) {
        should_stamp = false;
    }
    for (int index = 0; index <= 4; index++) {
        int x = adjusted_points[index][0];
        int y = adjusted_points[index][1];
        if (x >= 0 && y >= 0 && x < grid_width && y < grid_height) {
            if (persis_should_stamp) {
                tilemap[y][x] = 1+current_shape;
            }
            int sel_line = top_margin + 1 + y;
            int sel_col = left_margin + 1 + x * 2;
            // update video memory for the two characters in the selected cell
            char *vidmem = (char *)0xb8000;
            unsigned int offset = (sel_line * 80 + sel_col) * 2 + 1;
            vidmem[offset] = attr;
            offset = (sel_line * 80 + sel_col + 1) * 2 + 1;
            vidmem[offset] = attr;
        };
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
        k_printf(buffer, 1);
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
            current_rot = 0;
            current_shape++;
            if (current_shape >= 7)
                current_shape = 0;
            break;
        case 0x3C: // f2 key to change shape rotation
            current_rot++;
            if (current_rot >= 4)
                current_rot = 0;
            break;
        case 0x3D: // f3 key to stamp
            should_stamp = true;
            break;
        case 0x3E: // f4 key to toggle highlight/background color
            highlight_bg = !highlight_bg;
            break;
        default:
            break;
    }
    draw_grid();
    outb(0x20, 0x20);
}

void init_timer() {
    k_printf("0s", 1);
}

void kernel_loop() {
    while (1) {
        asm volatile ("hlt");
    }
}

void k_main() {
    set_custom_palette();
    k_clear_screen();
    disable_cursor();
    k_printf("it works. welcome to tetros", 0);
    draw_grid();
    pic_remap();
    k_install_idt();
    init_timer();
    asm volatile ("sti");
    kernel_loop();
}
