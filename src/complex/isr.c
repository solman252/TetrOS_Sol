#include "complex/isr.h"
#include "complex/idt.h"
#include "complex/8259_pic.h"
#include "screen.h"

ISR g_interrupt_handlers[NO_INTERRUPT_HANDLERS];

void isr_register_interrupt_handler(int num, ISR handler) {
    if (num < NO_INTERRUPT_HANDLERS)
        g_interrupt_handlers[num] = handler;
}

void isr_end_interrupt(int num) {
    pic8259_eoi(num);
}

void isr_irq_handler(REGISTERS *reg) {
    if (g_interrupt_handlers[reg->int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg->int_no];
        handler(reg);
    }
    pic8259_eoi(reg->int_no);
}

void isr_exception_handler(REGISTERS reg) {
    if (reg.int_no < 32) {
        for (;;)
            ;
    }
    if (g_interrupt_handlers[reg.int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg.int_no];
        handler(&reg);
    }
}

