#include "vga.h"
#include "complex/io_ports.h"
#include "types.h"

uchar combine_colour(char fg, char bg) {
    return 16*bg+fg;
}

void vga_disable_cursor() {
    outb(0x3D4, 10);
    outb(0x3D5, 32);
}

void set_pallete_colour(vga_pallete* pallete, uint index, rgb_val colour) {
    if (index > 15) { return; }
    pallete->colours[index] = colour;
}

void __register_palette_colour__(uint index, rgb_val colour) {
    if (index > 15) { return; }
    if (index == 6) { index = 20; } else if (index > 7) { index += 48; }
    outb(0x3C8, index);
    outb(0x3C9, colour.r & 0x3F);
    outb(0x3C9, colour.g & 0x3F);
    outb(0x3C9, colour.b & 0x3F);
}

void register_palette(vga_pallete pallete) {
    for (uint i = 0; i < 17; i++) {
        __register_palette_colour__(i, pallete.colours[i]);
    }
}