#include "console.h"
#include "string.h"
#include "types.h"
#include "vga.h"
#include "io_ports.h"   //for outb

// offscreen buffer for double buffering
static uint16 offscreen_buffer[VGA_TOTAL_ITEMS];
//pointer for offscreen buffer
static uint16 *g_vga_buffer = offscreen_buffer;

//index for offscreen buffer
static uint32 g_vga_index = 0;
//cursor pos
static uint8 cursor_pos_x = 0, cursor_pos_y = 0;
//foreground and background colour
uint8 g_fore_color = COLOR_WHITE, g_back_color = COLOR_BLACK;


//clear off screen buffer
void console_clear(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    uint32 i;
    for (i = 0; i < VGA_TOTAL_ITEMS; i++) {
        // Use a space (0x20) as the default character.
        g_vga_buffer[i] = vga_item_entry(' ', fore_color, back_color);
    }
    g_vga_index = 0;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}

//init console subsystem
void console_init(VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color) {
    // Set the global colors.
    g_fore_color = fore_color;
    g_back_color = back_color;
    // Set the drawing target to the off-screen buffer.
    g_vga_buffer = offscreen_buffer;
    cursor_pos_x = 0;
    cursor_pos_y = 0;
    console_clear(fore_color, back_color);
}


//newline helper thingy
static void console_newline() {
    if (cursor_pos_y >= VGA_HEIGHT - 1) {
        //reset if reach bottom
        cursor_pos_x = 0;
        cursor_pos_y = 0;
        console_clear(g_fore_color, g_back_color);
    } else {
        //move index to beginning of new line
        g_vga_index += VGA_WIDTH - (g_vga_index % VGA_WIDTH);
        cursor_pos_x = 0;
        ++cursor_pos_y;
        vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
    }
}


//output a single char at cursor
void console_putchar(char ch) {
    if (ch == ' ') {
        g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
        vga_set_cursor_pos(cursor_pos_x++, cursor_pos_y);
    }
    else if (ch == '\t') {
        // Assume a tab is 4 spaces.
        for (int i = 0; i < 4; i++) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(' ', g_fore_color, g_back_color);
            vga_set_cursor_pos(cursor_pos_x++, cursor_pos_y);
        }
    }
    else if (ch == '\n') {
        console_newline();
    }
    else {
        // Only print if valid.
        if (ch > 0) {
            g_vga_buffer[g_vga_index++] = vga_item_entry(ch, g_fore_color, g_back_color);
            vga_set_cursor_pos(++cursor_pos_x, cursor_pos_y);
        }
    }
}


//remove char
void console_ungetchar() {
    if (g_vga_index > 0) {
        // Erase the last character.
        g_vga_buffer[--g_vga_index] = vga_item_entry(0, g_fore_color, g_back_color);
        if (cursor_pos_x > 0) {
            vga_set_cursor_pos(--cursor_pos_x, cursor_pos_y);
        } else {
            if (cursor_pos_y > 0) {
                cursor_pos_y--;
                cursor_pos_x = VGA_WIDTH - 1;
                vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
            }
        }
    }
}


//remove until n characters have been undone
void console_ungetchar_bound(uint8 n) {
    for (uint8 i = 0; i < n; i++) {
        console_ungetchar();
    }
}


 //move curson to specific x y cordinate
void console_gotoxy(uint16 x, uint16 y) {
    g_vga_index = (VGA_WIDTH * y) + x;
    cursor_pos_x = x;
    cursor_pos_y = y;
    vga_set_cursor_pos(cursor_pos_x, cursor_pos_y);
}


//print null terminated string at cursor current pos
void console_putstr(const char *str) {
    uint32 index = 0;
    while (str[index]) {
        if (str[index] == '\n')
            console_newline();
        else
            console_putchar(str[index]);
        index++;
    }
}


//very bad printf ripoff
void printf(const char *format, ...) {
    char **arg = (char **)&format;
    int c;
    char buf[32];

    arg++;

    memset(buf, 0, sizeof(buf));
    while ((c = *format++) != 0) {
        if (c != '%') {
            console_putchar(c);
        } else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;
            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }
            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }
            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;
                    goto string;
                    break;
                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";
                string:
                    for (p2 = p; *p2; p2++)
                        ;
                    for (; p2 < p + pad; p2++)
                        console_putchar(pad0 ? '0' : ' ');
                    while (*p)
                        console_putchar(*p++);
                    break;
                default:
                    console_putchar(*((int *)arg++));
                    break;
            }
        }
    }
}


//         drawing and buffer stuff

void console_swap_buffers(void) {
    //copy off screen buffer to vga memory
    uint16 *vga_memory = (uint16 *)VGA_ADDRESS;
    memcpy(vga_memory, offscreen_buffer, VGA_TOTAL_ITEMS * sizeof(uint16));
}


//update screen
void console_update_screen(void) {
    console_swap_buffers();
}


//just read the code gang
void console_draw_box(uint16 x, uint16 y, uint16 width, uint16 height, VGA_COLOR_TYPE border_color) {
    //save current colors.
    uint8 saved_fore = g_fore_color;
    uint8 saved_back = g_back_color;
    
    //set border color
    g_fore_color = border_color;
    
    //draw top border
    console_gotoxy(x, y);
    console_putchar('+');
    for (uint16 i = 1; i < width - 1; i++) {
        console_putchar('-');
    }
    console_putchar('+');

    //draw vertical sides
    for (uint16 j = y + 1; j < y + height - 1; j++) {
        console_gotoxy(x, j);
        console_putchar('|');
        console_gotoxy(x + width - 1, j);
        console_putchar('|');
    }
    
    //draw bottom border
    console_gotoxy(x, y + height - 1);
    console_putchar('+');
    for (uint16 i = 1; i < width - 1; i++) {
        console_putchar('-');
    }
    console_putchar('+');
    
    // Restore colors.
    g_fore_color = saved_fore;
    g_back_color = saved_back;
}


//print string at x y pos
void console_putstr_at(uint16 x, uint16 y, const char *str) {
    console_gotoxy(x, y);
    console_putstr(str);
}


//enable that ugly ass cursor
void console_enable_cursor(void) {
    outportb(0x3D4, 0x0A);
    outportb(0x3D5, 0x00);
    outportb(0x3D4, 0x0B);
    outportb(0x3D5, 0x0F);
}


//thank you
void console_disable_cursor(void) {
    vga_disable_cursor();
}
