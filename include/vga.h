#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_ADDRESS 0xB8000

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

uchar combine_colour(char fore_color, char back_color);

void vga_disable_cursor();

// Sets the colour at index {index} of the provided pallete that {pallete} points to.
void set_pallete_colour(vga_pallete* pallete, uint index, rgb_val colour);

// Register the colour at index {index} of the VGA pallete to {colour}.
void __register_palette_colour__(uint index, rgb_val colour);

// Register the VGA pallete using the colours from {pallete}.
void register_palette(vga_pallete pallete);

#endif
