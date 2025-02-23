ENTRY(kernel_main)

SECTIONS
{
    . = 0x1000;  /* Kernel start address */

    .text :
    {
        *(.text)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}
