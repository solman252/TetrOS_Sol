#ifndef TYPES_H
#define TYPES_H

#define NULL 0

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef uchar byte;
typedef ushort word;
typedef uint dword;

typedef enum {
    false,
    true
} bool;

typedef struct { int x,y; } vec2;
vec2 v2(int x, int y);
vec2 add_v2(vec2 a, vec2 b);
vec2 sub_v2(vec2 a, vec2 b);
vec2 mult_v2(vec2 a, vec2 b);
vec2 div_v2(vec2 a, vec2 b);

typedef struct { uint r,g,b; } rgb_val;
rgb_val rgb(uint r, uint g, uint b);

typedef struct { rgb_val colours[16]; } vga_pallete;
vga_pallete new_pallete();

#endif