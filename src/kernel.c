#include "complex/kernel.h"
#include "screen.h"
#include "string.h"
#include "complex/gdt.h"
#include "complex/idt.h"
#include "keyboard.h"

vga_pallete pallete;

void kmain() {
    gdt_init();
    idt_init();

    screen_init();
    // keyboard_init();

    vga_disable_cursor();
    clear_screen();
    pallete = new_pallete();
    register_palette(pallete);

}

