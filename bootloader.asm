; bootloader.asm - Multiboot compliant bootloader & interrupt stubs
section .text
    align 4
    dd 0x1BADB002           ; Multiboot magic number
    dd 0x00                 ; Flags
    dd -(0x1BADB002 + 0x00)  ; Checksum

global start
extern k_main

; Declare external C interrupt handlers.
extern timer_handler
extern keyboard_handler

start:
    cli             ; Disable interrupts.
    call k_main     ; Call the kernel main function.
    hlt             ; Halt if k_main ever returns.

; --- IDT Loading Routine ---
global idt_load
idt_load:
    mov eax, [esp+4]  ; Get pointer to our idt_ptr structure.
    lidt [eax]        ; Load the IDT register.
    ret

; --- Timer Interrupt Stub ---
global timer_handler_stub
timer_handler_stub:
    pusha                  ; Save registers.
    call timer_handler     ; Call C timer handler.
    popa                   ; Restore registers.
    iret                   ; Return from interrupt.

; --- Keyboard Interrupt Stub ---
global keyboard_handler_stub
keyboard_handler_stub:
    pusha                   ; Save registers.
    call keyboard_handler   ; Call C keyboard handler.
    popa                    ; Restore registers.
    iret                    ; Return from interrupt.
