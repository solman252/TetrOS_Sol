#include "types.h"

vec2 v2(int x, int y) {
    vec2 out = {x,y};
    return out;
}
vec2 add_v2(vec2 a, vec2 b) { vec2 out = a; out.x += b.x; out.y += b.y; return out; }
vec2 sub_v2(vec2 a, vec2 b) { vec2 out = a; out.x -= b.x; out.y -= b.y; return out; }
vec2 mult_v2(vec2 a, vec2 b) { vec2 out = a; out.x *= b.x; out.y *= b.y; return out; }
vec2 div_v2(vec2 a, vec2 b) { vec2 out = a; out.x /= b.x; out.y /= b.y; return out; }

rgb_val rgb(uint r, uint g, uint b) {
    if (r > 255) { r = 255; }; if (g > 255) { g = 255; }; if (b > 255) { b = 255; };
    r = (int)(r/4); g = (int)(g/4); b = (int)(b/4);
    rgb_val out = {r,g,b};
    return out;
}

vga_pallete new_pallete() {
    vga_pallete out = {{{0,0,0},{63,63,63},{26,26,26},{63,0,0},{63,32,0},{50,50,0},{0,50,0},{0,55,55},{0,0,63},{63,0,63},{63,32,32},{63,48,32},{63,63,50},{50,63,63},{50,63,50},{63,48,63}}};
    return out;
}