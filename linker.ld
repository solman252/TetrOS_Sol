OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS {
    . = 0x10000;
    
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) *(COMMON) }
    _end = .;
    
    . = ALIGN(512);
}
