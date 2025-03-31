#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt.h"
#include "idt.h"
#include "io_ports.h"  // For inportb()
#include "vga.h"

#define GRID_ROWS       20
#define GRID_COLS       10
#define CELL_WIDTH      2   // Each cell is 2 characters wide
#define GRID_ORIGIN_X   10
#define GRID_ORIGIN_Y   5

// Global variables tracking the currently selected cell.
int selected_row = 0;
int selected_col = 0;

/**
 * Draw a single cell at (row, col). If is_selected is non-zero,
 * draw the cell as a cursor ("[]"). Otherwise, draw it as empty ("..").
 */
void draw_cell(int row, int col, int is_selected) {
    console_gotoxy(GRID_ORIGIN_X + col * CELL_WIDTH, GRID_ORIGIN_Y + row);
    if (is_selected)
        console_putstr("[]");
    else
        console_putstr("..");
}

/**
 * Draw the entire grid.
 */
void draw_grid() {
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            draw_cell(row, col, (row == selected_row && col == selected_col));
        }
    }
}

/**
 * Waits until there is a key available by polling the keyboard status port (0x64)
 * and then reads the scancode from port 0x60.
 */
uint8 get_scancode() {
    // Wait for the keyboard's output buffer to be full (bit 0 of port 0x64).
    while (!(inportb(0x64) & 1)) {
        // Busy-wait
    }
    return inportb(0x60);
}

/**
 * Update the grid when the selection changes.
 * This function redraws the old cell and the new cell.
 */
void update_selection(int new_row, int new_col) {
    // Redraw the old cell as empty.
    draw_cell(selected_row, selected_col, 0);
    
    // Update selection globals.
    selected_row = new_row;
    selected_col = new_col;
    
    // Draw the new cell as selected.
    draw_cell(selected_row, selected_col, 1);
}

/**
 * Entry point for the kernel.
 */
void kmain() {
    // Initialize GDT, IDT, and console.
    gdt_init();
    idt_init();
    console_init(COLOR_WHITE, COLOR_BLACK);

    // Draw our initial grid.
    draw_grid();

    // Main loop: poll for keyboard input and update the grid selection.
    while (1) {
        uint8 scancode = get_scancode();

        /* 
         * The following scancodes are for the arrow keys (make codes):
         * Up    : 0x48
         * Down  : 0x50
         * Left  : 0x4B
         * Right : 0x4D
         *
         * (Release codes typically have bit 7 set. We ignore those for this example.)
         */
        switch (scancode) {
            case 0x48: // Up arrow
                if (selected_row > 0)
                    update_selection(selected_row - 1, selected_col);
                break;
            case 0x50: // Down arrow
                if (selected_row < GRID_ROWS - 1)
                    update_selection(selected_row + 1, selected_col);
                break;
            case 0x4B: // Left arrow
                if (selected_col > 0)
                    update_selection(selected_row, selected_col - 1);
                break;
            case 0x4D: // Right arrow
                if (selected_col < GRID_COLS - 1)
                    update_selection(selected_row, selected_col + 1);
                break;
            default:
                // Ignore other keys.
                break;
        }
    }

    // The following code is unreachable but left from your original example.
    asm volatile("\txorl %edx, %edx");
    asm volatile("\tmovl $0x7b, %eax");
    asm volatile("\tmovl $0, %ecx");
    asm volatile("\tidivl %ecx");
}
