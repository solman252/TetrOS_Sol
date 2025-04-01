#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "vga.h"

#define GRID_COLS       10
#define GRID_ROWS       20
#define CELL_WIDTH      2
#define GRID_X_OFFSET   10
#define GRID_Y_OFFSET   2
#define EMPTY_CELL      255

extern uint16 vga_item_entry(uint8 ch, VGA_COLOR_TYPE fore_color, VGA_COLOR_TYPE back_color);

uint8 grid[GRID_ROWS][GRID_COLS];

typedef struct {
    int shape_id;
    int rotation;
    int x;
    int y;
    VGA_COLOR_TYPE color;
} CurrentShape;

CurrentShape current;

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

int is_valid_position(int newX, int newY, int newRotation) {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            int val = 0;
            switch (current.shape_id) {
                case 0: val = shape_o[newRotation][r][c]; break;
                case 1: val = shape_i[newRotation][r][c]; break;
                case 2: val = shape_s[newRotation][r][c]; break;
                case 3: val = shape_z[newRotation][r][c]; break;
                case 4: val = shape_l[newRotation][r][c]; break;
                case 5: val = shape_j[newRotation][r][c]; break;
                case 6: val = shape_t[newRotation][r][c]; break;
                default: break;
            }
            if (val) {
                int gridX = newX + c;
                int gridY = newY + r;
                if (gridX < 0 || gridX >= GRID_COLS || gridY < 0 || gridY >= GRID_ROWS)
                    return 0;
                if (grid[gridY][gridX] != EMPTY_CELL)
                    return 0;
            }
        }
    }
    return 1;
}

void draw_cell(int row, int col, int filled, VGA_COLOR_TYPE color) {
    int screen_x = GRID_X_OFFSET + col * CELL_WIDTH;
    int screen_y = GRID_Y_OFFSET + row;
    int index = screen_y * VGA_WIDTH + screen_x;
    uint16 *vga = (uint16*)VGA_ADDRESS;
    if (filled) {
        vga[index]   = vga_item_entry('[', color, COLOR_BLACK);
        vga[index+1] = vga_item_entry(']', color, COLOR_BLACK);
    } else {
        vga[index]   = vga_item_entry('.', COLOR_WHITE, COLOR_BLACK);
        vga[index+1] = vga_item_entry('.', COLOR_WHITE, COLOR_BLACK);
    }
}

void draw_grid() {
    int r, c;
    for (r = 0; r < GRID_ROWS; r++) {
        for (c = 0; c < GRID_COLS; c++) {
            if (grid[r][c] == EMPTY_CELL)
                draw_cell(r, c, 0, COLOR_WHITE);
            else
                draw_cell(r, c, 1, grid[r][c]);
        }
    }
}

void draw_current_shape() {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            int val = 0;
            switch (current.shape_id) {
                case 0: val = shape_o[current.rotation][r][c]; break;
                case 1: val = shape_i[current.rotation][r][c]; break;
                case 2: val = shape_s[current.rotation][r][c]; break;
                case 3: val = shape_z[current.rotation][r][c]; break;
                case 4: val = shape_l[current.rotation][r][c]; break;
                case 5: val = shape_j[current.rotation][r][c]; break;
                case 6: val = shape_t[current.rotation][r][c]; break;
                default: break;
            }
            if (val) {
                int gridX = current.x + c;
                int gridY = current.y + r;
                if (gridX >= 0 && gridX < GRID_COLS && gridY >= 0 && gridY < GRID_ROWS) {
                    draw_cell(gridY, gridX, 1, current.color);
                }
            }
        }
    }
}

void place_shape() {
    int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            int val = 0;
            switch (current.shape_id) {
                case 0: val = shape_o[current.rotation][r][c]; break;
                case 1: val = shape_i[current.rotation][r][c]; break;
                case 2: val = shape_s[current.rotation][r][c]; break;
                case 3: val = shape_z[current.rotation][r][c]; break;
                case 4: val = shape_l[current.rotation][r][c]; break;
                case 5: val = shape_j[current.rotation][r][c]; break;
                case 6: val = shape_t[current.rotation][r][c]; break;
                default: break;
            }
            if (val) {
                int gridX = current.x + c;
                int gridY = current.y + r;
                if (gridX >= 0 && gridX < GRID_COLS && gridY >= 0 && gridY < GRID_ROWS) {
                    grid[gridY][gridX] = current.color;
                }
            }
        }
    }
}

VGA_COLOR_TYPE next_color(VGA_COLOR_TYPE cur) {
    switch(cur) {
        case COLOR_RED: return COLOR_GREEN;
        case COLOR_GREEN: return COLOR_BLUE;
        case COLOR_BLUE: return COLOR_YELLOW;
        case COLOR_YELLOW: return COLOR_MAGENTA;
        case COLOR_MAGENTA: return COLOR_CYAN;
        case COLOR_CYAN: return COLOR_WHITE;
        default: return COLOR_RED;
    }
}

void spawn_new_shape() {
    current.shape_id = 0;
    current.rotation = 0;
    current.x = (GRID_COLS - 4) / 2;
    current.y = 0;
    current.color = COLOR_RED;
}

void kmain() {
    int key;
    int newRotation;
    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);
    keyboard_init();
    int r, c;
    for (r = 0; r < GRID_ROWS; r++) {
        for (c = 0; c < GRID_COLS; c++) {
            grid[r][c] = EMPTY_CELL;
        }
    }
    spawn_new_shape();
    draw_grid();
    draw_current_shape();
    while (1) {
        key = kb_get_scancode();
        if (key == 0x4B) {
            if (is_valid_position(current.x - 1, current.y, current.rotation))
                current.x--;
        } else if (key == 0x4D) {
            if (is_valid_position(current.x + 1, current.y, current.rotation))
                current.x++;
        } else if (key == 0x48) {
            if (is_valid_position(current.x, current.y - 1, current.rotation))
                current.y--;
        } else if (key == 0x50) {
            if (is_valid_position(current.x, current.y + 1, current.rotation))
                current.y++;
        } else if (key == 0x13) {
            newRotation = (current.rotation + 1) % 4;
            if (is_valid_position(current.x, current.y, newRotation))
                current.rotation = newRotation;
        } else if (key == 0x2E) {
            current.color = next_color(current.color);
        } else if (key == 0x1F) {
            current.shape_id = (current.shape_id + 1) % 7;
            current.rotation = 0;
            current.x = (GRID_COLS - 4) / 2;
            current.y = 0;
        } else if (key == 0x39) {
            place_shape();
            current.shape_id = (current.shape_id + 1) % 7;
            current.rotation = 0;
            current.x = (GRID_COLS - 4) / 2;
            current.y = 0;
            current.color = COLOR_RED;
        }
        draw_grid();
        draw_current_shape();
    }
}
