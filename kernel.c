#include "kernel.h"

const int grid_width = 10;
const int grid_height = 20;

int grid_sel_x = 4;
int grid_sel_y = 0;
int current_shape = 0;
int current_rot = 0;
float fall_speed = 1.5; // speed in tiles per second
bool highlight_bg = true;
bool should_stamp = false;

volatile unsigned int tick_count = 0;
volatile unsigned int fall_tick_count = 0;
volatile unsigned int pause_tick_count = 0;

void set_vga_palette(int color_index, int r, int g, int b) {
    outb(0x3C8, color_index); // Select the color index to modify
    outb(0x3C9, r & 0x3F);    // Set red component (0-63)
    outb(0x3C9, g & 0x3F);    // Set green component (0-63)
    outb(0x3C9, b & 0x3F);    // Set blue component (0-63)
}

void set_custom_palette() {
    set_vga_palette(0,  0,  0,  0);   // BLACK
    set_vga_palette(1,  63, 63, 63);  // WHITE
    set_vga_palette(2,  26, 26, 26);  // GRAY
    set_vga_palette(3,  0,  55, 55);  // CYAN
    set_vga_palette(4,  0,  0,  63);  // BLUE
    set_vga_palette(5,  63, 32, 0);   // ORANGE
    set_vga_palette(7,  0,  50, 0);   // GREEN
    set_vga_palette(20, 50, 50, 0);   // YELLOW (using index 20)
    set_vga_palette(56, 63, 0,  63);  // PURPLE (using index 56)
    set_vga_palette(57, 63, 0,  0);   // RED (using index 57)
    set_vga_palette(58, 50, 63, 63);  // LIGHT_CYAN (using index 58)
    set_vga_palette(59, 63, 48, 32);  // LIGHT_ORANGE (using index 59)
    set_vga_palette(60, 63, 63, 50);  // LIGHT_YELLOW (using index 60)
    set_vga_palette(61, 50, 63, 50);  // LIGHT_GREEN (using index 61)
    set_vga_palette(62, 63, 48, 63);  // LIGHT_PURPLE (using index 62)
    set_vga_palette(63, 63, 32, 32);  // LIGHT_RED (using index 63)
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
#define LIGHT_BLUE 0x03  // light blue = cyan
#define LIGHT_ORANGE 0x0B
#define LIGHT_YELLOW 0x0C
#define LIGHT_GREEN 0x0D
#define LIGHT_PURPLE 0x0E
#define LIGHT_RED 0x0F

void clear_screen() {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE;
    }
}

unsigned int print(char *message, char color, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = color;
        }
    }
    return 1;
}

unsigned int print_cols(char *message, char *color, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message || *color) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
            color++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = *color++;
        }
    }
    return 1;
}

unsigned int round(float num) {
    return (int)(num * 10 + 0.5) / 10.;
}
unsigned int rand(unsigned int start_range,unsigned int end_range)
{
    static unsigned int rand = 0xACE1U; /* Any nonzero start state will work. */

    /*check for valid range.*/
    if(start_range == end_range) {
        return start_range;
    }

    /*get the random in end-range.*/
    rand += 0x3AD;
    rand %= end_range;

    /*get the random in start-range.*/
    while(rand < start_range){
        rand = rand + end_range - start_range;
    }

    return rand;
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

void pic_remap() { // remap pics to avoid irq conflicts
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
bool full_lines[20];

void reset() {
    fall_speed = 1.5;
    tick_count = 0;
    fall_tick_count = 0;
    for (int y = 0; y < grid_height; y++) {
        for (int x = 0; x < grid_width; x++) {
            tilemap[y][x] = 0;
        }
    }
}

int current_shape_points[4][2];
void get_current_shape_points() {
    int (*rot)[4];
    switch(current_shape) {
        case 0: rot = shape_o[current_rot]; break;
        case 1: rot = shape_i[current_rot]; break;
        case 2: rot = shape_s[current_rot]; break;
        case 3: rot = shape_z[current_rot]; break;
        case 4: rot = shape_l[current_rot]; break;
        case 5: rot = shape_j[current_rot]; break;
        case 6: rot = shape_t[current_rot]; break;
        default: rot = shape_o[current_rot]; break;
    }
    int center_x = 0, center_y = 0;
    int points[4][2] = { {0,0}, {0,0}, {0,0}, {0,0} };
    int i = 0;
    for (int y = 0; y < 4; y++) {
        int *row = rot[y];
        for (int x = 0; x < 4; x++) {
            int ch = row[x];
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
    for (i = 0; i < 4; i++) {
        current_shape_points[i][0] = points[i][0] - center_x + grid_sel_x;
        current_shape_points[i][1] = points[i][1] - center_y + grid_sel_y;
    }
}

bool is_current_shape_illegal_placement() {
    get_current_shape_points();
    for (int i = 0; i < 4; i++) {
        int x = current_shape_points[i][0];
        int y = current_shape_points[i][1];
        if (!(x >= 0 && x < grid_width && y < grid_height && (tilemap[y][x] == 0 || y < 0))) {
            return true;
        }
    }
    return false;
}

//
// draw_grid centers the board and draws an outline using double-line box drawing characters:
// Top border: ╔ (0xC9), ═ (0xCD), ╗ (0xBB)
// Side borders: ║ (0xBA)
// Bottom border: ╚ (0xC8), ═ (0xCD), ╝ (0xBC)
//
void draw_grid() {
    int board_width = grid_width * 2 + 2;
    int board_height = grid_height + 2;
    int left_margin = (80 - board_width) / 2;
    int top_margin = (27 - board_height) / 2;
    
    char line[256];
    char col_line[256];
    int pos, i;
    
    pos = 0;
    for (i = 0; i < left_margin; i++) {
        line[pos] = ' ';
        col_line[pos] = WHITE;
        pos++;
    }
    line[pos] = 0xC9;  // ╔
    col_line[pos] = WHITE;
    pos++;
    for (i = 0; i < board_width - 2; i++) {
        line[pos] = 0xCD;  // ═
        col_line[pos] = WHITE;
        pos++;
    }
    line[pos] = 0xBB;  // ╗
    col_line[pos] = WHITE;
    pos++;
    line[pos] = '\0';
    col_line[pos] = '\0';
    print_cols(line, col_line, top_margin);
    
    get_current_shape_points();
    
    // Draw grid rows with vertical borders
    for (int r = 0; r < grid_height; r++) {
        pos = 0;
        // Left margin spaces with attribute WHITE
        for (i = 0; i < left_margin; i++) {
            line[pos] = ' ';
            col_line[pos] = WHITE;
            pos++;
        }
        // Left border (║)
        line[pos] = 0xBA;
        col_line[pos] = WHITE;
        pos++;
        for (int c = 0; c < grid_width; c++) {
            bool in_points = false;
            for (i = 0; i < 4; i++) {
                if (c == current_shape_points[i][0] && r == current_shape_points[i][1]) {
                    in_points = true;
                    break;
                }
            }
            if (in_points || tilemap[r][c] != 0) {
                char cell_color = WHITE;
                char cell_inner = WHITE;
                if (in_points) {
                    switch(current_shape + 1) {
                        case 1: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; break;
                        case 2: cell_color = CYAN; cell_inner = LIGHT_CYAN; break;
                        case 3: cell_color = GREEN; cell_inner = LIGHT_GREEN; break;
                        case 4: cell_color = RED; cell_inner = LIGHT_RED; break;
                        case 5: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; break;
                        case 6: cell_color = BLUE; cell_inner = LIGHT_BLUE; break;
                        case 7: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; break;
                    }
                } else {
                    switch(tilemap[r][c]) {
                        case 1: cell_color = YELLOW; cell_inner = LIGHT_YELLOW; break;
                        case 2: cell_color = CYAN; cell_inner = LIGHT_CYAN; break;
                        case 3: cell_color = GREEN; cell_inner = LIGHT_GREEN; break;
                        case 4: cell_color = RED; cell_inner = LIGHT_RED; break;
                        case 5: cell_color = ORANGE; cell_inner = LIGHT_ORANGE; break;
                        case 6: cell_color = BLUE; cell_inner = LIGHT_BLUE; break;
                        case 7: cell_color = PURPLE; cell_inner = LIGHT_PURPLE; break;
                    }
                }
                unsigned char attr;
                if (highlight_bg && (tilemap[r][c] != 0 || in_points)) {
                    attr = (cell_color << 4) | cell_inner;
                } else {
                    attr = cell_color;
                }
                // Draw the cell as "[ ]" with the proper attribute
                line[pos] = '[';
                col_line[pos] = attr;
                pos++;
                line[pos] = ']';
                col_line[pos] = attr;
                pos++;
            } else {
                // Draw an empty cell using box-drawing characters (set with WHITE)
                line[pos] = 0xC4;  // horizontal line piece
                col_line[pos] = GRAY;
                pos++;
                line[pos] = 0xB4;  // vertical line piece
                col_line[pos] = GRAY;
                pos++;
            }
        }
        // Right border (║)
        line[pos] = 0xBA;
        col_line[pos] = WHITE;
        pos++;
        line[pos] = '\0';
        col_line[pos] = '\0';
        print_cols(line, col_line, top_margin + 1 + r);
    }
    
    // Draw bottom border using ╚, ═, ╝
    pos = 0;
    for (i = 0; i < left_margin; i++) {
        line[pos] = ' ';
        col_line[pos] = WHITE;
        pos++;
    }
    line[pos] = 0xC8;  // ╚
    col_line[pos] = WHITE;
    pos++;
    for (i = 0; i < board_width - 2; i++) {
        line[pos] = 0xCD;  // ═
        col_line[pos] = WHITE;
        pos++;
    }
    line[pos] = 0xBC;  // ╝
    col_line[pos] = WHITE;
    pos++;
    line[pos] = '\0';
    col_line[pos] = '\0';
    print_cols(line, col_line, top_margin + board_height - 1);
    
    // Stamp the piece if should_stamp is true (update tilemap)
    if (should_stamp) {
        should_stamp = false;
        bool did_reset = false;
        for (int index = 0; index < 4; index++) {
            int x = current_shape_points[index][0];
            int y = current_shape_points[index][1];
            if (x >= 0 && y >= 0 && x < grid_width && y < grid_height) {
                if (tilemap[y][x] != 0) {
                    reset();
                    did_reset = true;
                    break;
                } else {
                    tilemap[y][x] = 1 + current_shape;
                }
            }
        }
        if(!did_reset) {
            for (int y = 0; y < grid_height; y++) {
                bool line_full = true;
                for (int x = 0; x < grid_width; x++) {
                    if (tilemap[y][x] == 0) {
                        line_full = false;
                        break;
                    }
                };
                if(line_full) {
                    pause_tick_count = 6;
                    full_lines[y] = true;
                }
                full_lines[y] = line_full;
            }
        }
    }
}

void timer_handler() {
    tick_count++;
    fall_tick_count++;
    if (tick_count % 18 == 0) {
        static unsigned int seconds = 0;
        seconds++;
        char buffer[16];
        itoa(seconds, buffer, 10);
        int i = 0;
        while (buffer[i]) i++;
        buffer[i] = 's';
        buffer[i+1] = '\0';
        print(buffer, WHITE, 1);
    }
    if (pause_tick_count == 0) {
        if (fall_tick_count % round(18/fall_speed) == 0) {
            grid_sel_y++;
            if(is_current_shape_illegal_placement()) {
                grid_sel_y--;
                should_stamp = true;
                draw_grid();
                current_shape = rand(0,7);
                grid_sel_x = 4;
                grid_sel_y = 0;
                current_rot = 0;
            }
        }
    } else if (pause_tick_count > 0) {
        if (tick_count % 3 == 0) {
            pause_tick_count --;
        }
        if (pause_tick_count < 0) {
            pause_tick_count = 0;
        }
        for (int y = 0; y < grid_height; y++) {
            bool line_full = full_lines[y];
            if(line_full) {
                if(pause_tick_count == 0) {
                    for (int y2 = y; y2 > 0; y2--) {
                        for (int x = 0; x < grid_width; x++) {
                            tilemap[y2][x] = tilemap[y2-1][x];
                        }
                    }
                    full_lines[y] = false;
                } else {
                    for (int x = 0; x < grid_width; x++) {
                        if (pause_tick_count % 2 == 0) {
                            tilemap[y][x] = 0;
                        } else {
                            tilemap[y][x] = -1;
                        }
                    }
                }
            }
        }
    }
    draw_grid();
    outb(0x20, 0x20);
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    if (pause_tick_count == 0) {
        switch(scancode) {
            case 0x4B: // left key
                grid_sel_x--;
                if (is_current_shape_illegal_placement()) {
                    grid_sel_x++;
                }
                break;
            case 0x4D: // right key
                grid_sel_x++;
                if (is_current_shape_illegal_placement()) {
                    grid_sel_x--;
                }
                break;
            case 0x48: // down key (moves grid up)
                grid_sel_y--;
                if (is_current_shape_illegal_placement()) {
                    grid_sel_y++;
                } else {
                    fall_tick_count = 0;
                }
                break;
            case 0x50: // up key (moves grid down)
                grid_sel_y++;
                if (is_current_shape_illegal_placement()) {
                    grid_sel_y--;
                } else {
                    fall_tick_count = 0;
                }
                break;
            case 0x3B: // f1 key to change current shape
                current_rot = 0;
                current_shape++;
                if (current_shape >= 7) {
                    current_shape = 0;
                }
                break;
            case 0x3C: // f2 key to change shape rotation
                current_rot++;
                if (current_rot >= 4) {
                    current_rot = 0;
                }
                if (is_current_shape_illegal_placement()) {
                    current_rot--;
                    if (current_rot < 0) {
                        current_rot = 3;
                    }
                }
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
    }
    draw_grid();
    outb(0x20, 0x20);
}

void init_timer() {
    print("0s", WHITE, 1);
}

void kernel_loop() {
    while (1) {
        asm volatile ("hlt");
    }
}

void k_main() {
    outb(0x3C6, 0xFF);
    set_custom_palette();
    clear_screen();
    disable_cursor();
    print("Welcome to TetrOS!", WHITE, 0);
    draw_grid();
    pic_remap();
    k_install_idt();
    init_timer();
    asm volatile ("sti");
    kernel_loop();
}
