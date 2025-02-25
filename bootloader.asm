bits 32		;nasm directive
section .text
	;multiboot spec
	align 4
	dd 0x1BADB002			;magic
	dd 0x00				;flags
	dd - (0x1BADB002 + 0x00)	;checksum

global start
extern k_main	;k_main is defined in the kernel.c

start:
	cli  ; stop interrupts

	call k_main

	hlt ; halt the CPU
