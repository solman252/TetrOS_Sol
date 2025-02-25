#include "kernel.h"

// Global tick counter for timer interrupts
volatile unsigned int tick_count = 0;

// --- VGA Text Mode Functions ---
void k_clear_screen() {
    char *vidmem = (char *)0xb8000;
    unsigned int i = 0;
    while (i < (80 * 25 * 2)) {
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE_TXT;
    }
}

unsigned int k_printf(char *message, unsigned int line) {
    char *vidmem = (char *)0xb8000;
    unsigned int i = line * 80 * 2;
    while (*message) {
        if (*message == '\n') {
            line++;
            i = line * 80 * 2;
            message++;
        } else {
            vidmem[i++] = *message++;
            vidmem[i++] = WHITE_TXT;
        }
    }
    return 1;
}

void disable_cursor() {
    // Disable the VGA hardware text cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

// --- Utility Function: Simple itoa ---
// Converts an integer to a string (base 10).
void itoa(int value, char *str, int base) {
    char *rc = str, *ptr = str, *low;
    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    int num = value;
    do {
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base];
        num /= base;
    } while(num);
    *ptr = '\0';
    low = rc;
    if (*rc == '-') { low++; }
    char *high = ptr - 1;
    while(low < high) {
        char temp = *low;
        *low++ = *high;
        *high-- = temp;
    }
}

// --- I/O Port Functions ---
void outb(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// --- PIC Remapping ---
// Remaps the PIC so that IRQs start at vector 32.
void pic_remap() {
    unsigned char a1 = inb(0x21);
    unsigned char a2 = inb(0xA1);

    outb(0x20, 0x11);  // Start initialization in cascade mode.
    outb(0xA0, 0x11);
    outb(0x21, 0x20);  // Master PIC vector offset.
    outb(0xA1, 0x28);  // Slave PIC vector offset.
    outb(0x21, 0x04);  // Tell Master PIC about slave PIC at IRQ2.
    outb(0xA1, 0x02);  // Tell Slave PIC its cascade identity.
    outb(0x21, 0x01);  // Set PICs to 8086/88 (MCS-80/85) mode.
    outb(0xA1, 0x01);

    // Restore saved masks.
    outb(0x21, a1);
    outb(0xA1, a2);
}

// --- IDT Structures and Setup ---
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;      // Kernel segment selector.
    unsigned char  always0;
    unsigned char  flags;    // Present, ring 0, 32-bit interrupt gate.
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

// Set an entry in the IDT.
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

// Declaration for assembly routine to load the IDT.
extern void idt_load(unsigned int);

void k_install_idt() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (unsigned int)&idt;
    
    // External assembly stubs for interrupt handlers.
    extern void timer_handler_stub();
    extern void keyboard_handler_stub();
    
    // Map IRQ0 (timer) to vector 32 and IRQ1 (keyboard) to vector 33.
    idt_set_gate(32, (unsigned int)timer_handler_stub, 0x08, 0x8E);
    idt_set_gate(33, (unsigned int)keyboard_handler_stub, 0x08, 0x8E);
    
    idt_load((unsigned int)&idtp);
}

// --- Timer Interrupt Handler ---
// Increments a seconds counter every 18 ticks (roughly 1 second).
void timer_handler() {
    tick_count++;
    if (tick_count % 18 == 0) {
        static unsigned int seconds = 0;
        seconds++;
        char buffer[16];
        itoa(seconds, buffer, 10);
        // Append 's' to display seconds (e.g., "1s", "2s", etc.)
        int i = 0;
        while (buffer[i]) i++;
        buffer[i] = 's';
        buffer[i+1] = '\0';
        k_printf(buffer, 22);  // Display on line 22.
    }
    // Send End-of-Interrupt (EOI) signal.
    outb(0x20, 0x20);
}

// --- Keyboard Interrupt Handler ---
// Future: convert scancodes to key events.
void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    k_printf("Key pressed", 23);
    outb(0x20, 0x20);
}

// --- Device Driver Initializations ---
void init_timer() {
    // Future: configure the PIT (Programmable Interval Timer) if needed.
}

void init_keyboard() {
    // Future: initialize keyboard state or scancode translation.
}

// --- Kernel Main Loop ---
// This loop serves as a placeholder for future event processing, scheduling,
// or game updates (such as your Tetris logic).
void kernel_loop() {
    while (1) {
        // Future: dispatch events, update game state, etc.
        asm volatile ("hlt");
    }
}

// --- Kernel Main Function ---
void k_main() {
    k_clear_screen();
    disable_cursor();
    k_printf("IT WORKS. Welcome to TetrOS", 0);

    // System initialization.
    pic_remap();
    k_install_idt();
    init_timer();
    init_keyboard();
    
    // Enable interrupts.
    asm volatile ("sti");
    
    // Enter main loop.
    kernel_loop();
}
