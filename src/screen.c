#include "vga.h"
#include "types.h"
#include "screen.h"

char* vidmem;
char display_buffer[2][VGA_HEIGHT][VGA_WIDTH];
vec2 cursor_pos = {0,0};

void screen_init() {
    vidmem = (char*)0xB8000;
}

void save_screen_state(char to[2][VGA_HEIGHT][VGA_WIDTH]) {
    uint i = 0;
    for(uint y = 0; y < 25; y++) {
        for(uint x = 0; x < 80; x++) {
            to[0][y][x] = vidmem[i++];
            to[1][y][x] = vidmem[i++];
        }
    }
}

void set_screen_state(char from[2][VGA_HEIGHT][VGA_WIDTH]) {
    uint i = 0;
    for(uint y = 0; y < 25; y++) {
        for(uint x = 0; x < 80; x++) {
            vidmem[i++] = from[0][y][x];
            vidmem[i++] = from[1][y][x];
        }
    }
}

void buffer_display() {
    set_screen_state(display_buffer);
}

void clear_screen() {
    for (uint y = 0; y < VGA_HEIGHT; y++) {
        for (uint x = 0; x < VGA_WIDTH; x++) {
            display_buffer[0][y][x] = NULL;
            display_buffer[1][y][x] = NULL;
        }
    }
    cursor_pos.x = 0; cursor_pos.y = 0;
}

// Prints {string} to the displayed screen at it's current cursor position, in colour {colour}.
void printc(char* string, char colour) {
    if (cursor_pos.x < 0) { cursor_pos.x = 0; }
    if (cursor_pos.y < 0) { cursor_pos.y = 0; }
    if (cursor_pos.x > 79) { cursor_pos.x = 79; }
    if (cursor_pos.y > 24) { cursor_pos.y = 24; }
    unsigned startX = cursor_pos.x;
    bool newline = true;
    while (*string) {
        if (*string == '\n') {
            cursor_pos.x = 0;
            cursor_pos.y++;
            if (cursor_pos.y > 24) {
                cursor_pos.y = 24;
            }
            string++;
        } else if (*string == '\xFF') {
            *string++;
            newline = false;
        } else {
            display_buffer[0][cursor_pos.y][cursor_pos.x] = *string++;
            display_buffer[1][cursor_pos.y][cursor_pos.x++] = colour;
            if (cursor_pos.x > 79) {
                cursor_pos.x = 79;
            }
        }
    }
    if (newline) {
        cursor_pos.x = 0;
        cursor_pos.y++;
        if (cursor_pos.y > 24) {
            cursor_pos.y = 24;
        }
    }
}

// Prints {string} to the displayed screen at it's current cursor position.
void print(char* string) {
    printc(string,0x01);
}

// Prints {i} to the displayed screen at it's current cursor position.
void printi(int i) {
    char string[count_digits(i)+1];
    itoa(string,10,i);
    printc(string,0x01);
}

// Prints {b} to the displayed screen at it's current cursor position.
void printb(bool b) {
    if (b) {
        printc("true",0x01);
    } else {
        printc("false",0x01);
    }
}

// Draws a box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using characters from {outline} to draw the outline in colour {outline_colour}.
// {outline} should contain 11 characters, in the following order: top left corner, top edge, top right corner, vertical right intersection, seperating line, vertical left intersection, left edge, right edge, bottom left corner, bottom edge, bottom right corner.
void draw_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour) {
    if (size.x < 2 || size.y < 2 || (top_left.x + size.x - 1) > 79 || (top_left.y + size.y - 1) > 24) { return; }
    vec2 old_cur = cursor_pos;

    char c[3] = " \xFF";
    unsigned i = 0;
    cursor_pos = top_left;

    c[0] = outline[0];
    printc(c,outline_colour);

    c[0] = outline[1];
    for (i = 0; i < size.x-2; i++) {
        printc(c,outline_colour);
    }

    c[0] = outline[2];
    printc(c,outline_colour);

    for (i = 0; i < size.y - 2; i++) {
        cursor_pos = add_v2(top_left,v2(0,1+i));
        c[0] = outline[6];
        printc(c,outline_colour);

        cursor_pos.x += size.x - 2;
        c[0] = outline[7];
        printc(c,outline_colour);
    }

    cursor_pos = add_v2(top_left,v2(0,size.y-1));
    c[0] = outline[8];
    printc(c,outline_colour);

    c[0] = outline[9];
    for (i = 0; i < size.x-2; i++) {
        printc(c,outline_colour);
    }

    c[0] = outline[10];
    printc(c,outline_colour);

    cursor_pos = old_cur;
}

// Draws a box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using default characters (default_box_outline) to draw the outline in the default colour (default_box_outline_colour).
void draw_box(vec2 top_left, vec2 size) {
    draw_box_with(top_left, size, default_box_outline, default_box_outline_colour);
}

// Draws a filled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using characters from {outline} to draw the outline in colour {outline_colour}, and using {fill} to fill the inside with colour {fill_colour}.
// {outline} should contain 11 characters, in the following order: top left corner, top edge, top right corner, vertical right intersection, seperating line, vertical left intersection, left edge, right edge, bottom left corner, bottom edge, bottom right corner.
void draw_filled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* fill, char fill_colour) {
    if (size.x < 2 || size.y < 2 || (top_left.x + size.x - 1) > 79 || (top_left.y + size.y - 1) > 24) { return; }

    draw_box_with(top_left,size,outline,outline_colour);

    vec2 old_cur = cursor_pos;
    
    char c[3] = " \xFF";
    unsigned i = 0;
    for (unsigned y = 0; y < size.y - 2; y++) {
        cursor_pos = add_v2(top_left,v2(1,1+y));
        i = 0;
        for (unsigned x = 0; x < size.x - 2; x++) {
            c[0] = fill[i++];
            if (i >= str_length(fill)) {
                i = 0;
            }
            printc(c,fill_colour);
        }
    }

    cursor_pos = old_cur;
}

// Draws a filled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using default characters (default_box_outline) to draw the outline in the default colour (default_box_outline_colour), and using the default fill string (default_box_fill) to fill the inside with the default colour (default_box_fill_colour).
void draw_filled_box(vec2 top_left, vec2 size) {
    draw_filled_box_with(top_left, size, default_box_outline, default_box_outline_colour, default_box_fill, default_box_fill_colour);
}

// Draws a labelled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using characters from {outline} to draw the outline in colour {outline_colour}, and labelling it with {label} in colour {label_colour}.
// {outline} should contain 11 characters, in the following order: top left corner, top edge, top right corner, vertical right intersection, seperating line, vertical left intersection, left edge, right edge, bottom left corner, bottom edge, bottom right corner.
void draw_labelled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* label, char label_colour) {
    int label_x = (int)(size.x / 2) - (int)(str_length(label) / 2);
    if (size.x < 2 || size.y < 4 || (top_left.x + size.x - 1) > 79 || (top_left.y + size.y - 1) > 24 || (top_left.x + label_x) < 0 || ((top_left.x + label_x) + str_length(label) - 1) > 79 || (2 + str_length(label)) > size.x) { return; }

    draw_box_with(top_left,size,outline,outline_colour);

    vec2 old_cur = cursor_pos;

    cursor_pos = add_v2(top_left,v2(label_x,1));
    printc(label,label_colour);
    
    char c[3] = " \xFF";
    cursor_pos = add_v2(top_left,v2(0,2));

    c[0] = outline[3];
    printc(c,outline_colour);

    c[0] = outline[4];
    for (unsigned i = 0; i < size.x-2; i++) {
        printc(c,outline_colour);
    }

    c[0] = outline[5];
    printc(c,outline_colour);

    cursor_pos = old_cur;
}

// Draws a labelled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using default characters (default_box_outline) to draw the outline in the default colour (default_box_outline_colour), and labelling it with {label} in the default colour (default_box_label_colour).
void draw_labelled_box(vec2 top_left, vec2 size, char* label) {
    draw_labelled_box_with(top_left,size,default_box_outline,default_box_outline_colour,label,default_box_label_colour);
}

// Draws a labelled, filled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using characters from {outline} to draw the outline in colour {outline_colour}, using {fill} to fill the inside in colour {fill_colour}, and labelling it with {label} in colour {label_colour}.
// {outline} should contain 11 characters, in the following order: top left corner, top edge, top right corner, vertical right intersection, seperating line, vertical left intersection, left edge, right edge, bottom left corner, bottom edge, bottom right corner.
void draw_labelled_filled_box_with(vec2 top_left, vec2 size, char* outline, char outline_colour, char* fill, char fill_colour, char* label, char label_colour) {
    int label_x = (int)(size.x / 2) - (int)(str_length(label) / 2);
    if (size.x < 2 || size.y < 4 || (top_left.x + size.x - 1) > 79 || (top_left.y + size.y - 1) > 24 || (top_left.x + label_x) < 0 || ((top_left.x + label_x) + str_length(label) - 1) > 79 || (2 + str_length(label)) > size.x) { return; }

    draw_filled_box_with(top_left,size,outline,outline_colour,fill,fill_colour);

    vec2 old_cur = cursor_pos;

    unsigned i = 0;
    cursor_pos = add_v2(top_left,v2(1,1));
    for (i = 0; i < size.x-2; i++) {
        printc(" \xFF",outline_colour);
    }

    cursor_pos = add_v2(top_left,v2(label_x,1));
    printc(label,label_colour);
    
    char c[3] = " \xFF";
    cursor_pos = add_v2(top_left,v2(0,2));

    c[0] = outline[3];
    printc(c,outline_colour);

    c[0] = outline[4];
    for (i = 0; i < size.x-2; i++) {
        printc(c,outline_colour);
    }

    c[0] = outline[5];
    printc(c,outline_colour);

    cursor_pos = old_cur;
}

// Draws a labelled, filled box to the displayed screen, with the top left corner being at {top_left}, with size {size}, using default characters (default_box_outline) to draw the outline in the default colour (default_box_outline_colour), using the default fill string (default_box_fill) to fill the inside with the default colour (default_box_fill_colour), and labelling it with {label} in the default colour (default_box_label_colour).
void draw_labelled_filled_box(vec2 top_left, vec2 size, char* label) {
    draw_labelled_filled_box_with(top_left,size,default_box_outline,default_box_outline_colour,default_box_fill,default_box_fill_colour,label,default_box_label_colour);
}