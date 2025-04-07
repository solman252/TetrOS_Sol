/**
 * Interrupt Descriptor Table(GDT) setup
 */

#include "complex/idt.h"
#include "complex/isr.h"
#include "complex/8259_pic.h"

IDT g_idt[NO_IDT_DESCRIPTORS];
IDT_PTR g_idt_ptr;

/**
 * fill entries of IDT 
 */
void idt_set_entry(int index, uint base, ushort seg_sel, uchar flags) {
    IDT *this = &g_idt[index];

    this->base_low = base & 0xFFFF;
    this->segment_selector = seg_sel;
    this->zero = 0;
    this->type = flags | 0x60;
    this->base_high = (base >> 16) & 0xFFFF;
}

void idt_init() {
    g_idt_ptr.base_address = (uint)g_idt;
    g_idt_ptr.limit = sizeof(g_idt) - 1;
    pic8259_init();

    idt_set_entry(0, (uint)exception_0, 0x08, 0x8E);
    idt_set_entry(1, (uint)exception_1, 0x08, 0x8E);
    idt_set_entry(2, (uint)exception_2, 0x08, 0x8E);
    idt_set_entry(3, (uint)exception_3, 0x08, 0x8E);
    idt_set_entry(4, (uint)exception_4, 0x08, 0x8E);
    idt_set_entry(5, (uint)exception_5, 0x08, 0x8E);
    idt_set_entry(6, (uint)exception_6, 0x08, 0x8E);
    idt_set_entry(7, (uint)exception_7, 0x08, 0x8E);
    idt_set_entry(8, (uint)exception_8, 0x08, 0x8E);
    idt_set_entry(9, (uint)exception_9, 0x08, 0x8E);
    idt_set_entry(10, (uint)exception_10, 0x08, 0x8E);
    idt_set_entry(11, (uint)exception_11, 0x08, 0x8E);
    idt_set_entry(12, (uint)exception_12, 0x08, 0x8E);
    idt_set_entry(13, (uint)exception_13, 0x08, 0x8E);
    idt_set_entry(14, (uint)exception_14, 0x08, 0x8E);
    idt_set_entry(15, (uint)exception_15, 0x08, 0x8E);
    idt_set_entry(16, (uint)exception_16, 0x08, 0x8E);
    idt_set_entry(17, (uint)exception_17, 0x08, 0x8E);
    idt_set_entry(18, (uint)exception_18, 0x08, 0x8E);
    idt_set_entry(19, (uint)exception_19, 0x08, 0x8E);
    idt_set_entry(20, (uint)exception_20, 0x08, 0x8E);
    idt_set_entry(21, (uint)exception_21, 0x08, 0x8E);
    idt_set_entry(22, (uint)exception_22, 0x08, 0x8E);
    idt_set_entry(23, (uint)exception_23, 0x08, 0x8E);
    idt_set_entry(24, (uint)exception_24, 0x08, 0x8E);
    idt_set_entry(25, (uint)exception_25, 0x08, 0x8E);
    idt_set_entry(26, (uint)exception_26, 0x08, 0x8E);
    idt_set_entry(27, (uint)exception_27, 0x08, 0x8E);
    idt_set_entry(28, (uint)exception_28, 0x08, 0x8E);
    idt_set_entry(29, (uint)exception_29, 0x08, 0x8E);
    idt_set_entry(30, (uint)exception_30, 0x08, 0x8E);
    idt_set_entry(31, (uint)exception_31, 0x08, 0x8E);
    idt_set_entry(32, (uint)irq_0, 0x08, 0x8E);
    idt_set_entry(33, (uint)irq_1, 0x08, 0x8E);
    idt_set_entry(34, (uint)irq_2, 0x08, 0x8E);
    idt_set_entry(35, (uint)irq_3, 0x08, 0x8E);
    idt_set_entry(36, (uint)irq_4, 0x08, 0x8E);
    idt_set_entry(37, (uint)irq_5, 0x08, 0x8E);
    idt_set_entry(38, (uint)irq_6, 0x08, 0x8E);
    idt_set_entry(39, (uint)irq_7, 0x08, 0x8E);
    idt_set_entry(40, (uint)irq_8, 0x08, 0x8E);
    idt_set_entry(41, (uint)irq_9, 0x08, 0x8E);
    idt_set_entry(42, (uint)irq_10, 0x08, 0x8E);
    idt_set_entry(43, (uint)irq_11, 0x08, 0x8E);
    idt_set_entry(44, (uint)irq_12, 0x08, 0x8E);
    idt_set_entry(45, (uint)irq_13, 0x08, 0x8E);
    idt_set_entry(46, (uint)irq_14, 0x08, 0x8E);
    idt_set_entry(47, (uint)irq_15, 0x08, 0x8E);
    idt_set_entry(128, (uint)exception_128, 0x08, 0x8E);

    load_idt((uint)&g_idt_ptr);
    asm volatile("sti");
}

