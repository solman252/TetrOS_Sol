#ifndef SCREEN_H
#define SCREEN_H

#include "vga.h"
#include "types.h"

void screen_init();
void save_screen_state(char to[2][VGA_HEIGHT][VGA_WIDTH]);
void set_screen_state(char from[2][VGA_HEIGHT][VGA_WIDTH]);
void buffer_display();
void clear_screen();

#define default_box_outline "+-++-+||+-+"
#define default_box_fill "[]"
#define default_box_outline_colour 0x01
#define default_box_label_colour 0x01
#define default_box_fill_colour 0x02

void draw_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour);
void draw_box(vec2 top_left, vec2 size);
void draw_filled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* fill, char fill_colour);
void draw_filled_box(vec2 top_left, vec2 size);
void draw_labelled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* label, char label_colour);
void draw_labelled_box(vec2 top_left, vec2 size, char* label);
void draw_labelled_filled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* fill, char fill_colour, char* label, char label_colour);
void draw_labelled_filled_box(vec2 top_left, vec2 size, char* label);

#endif
