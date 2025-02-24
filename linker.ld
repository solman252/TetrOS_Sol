ENTRY(kernel_main)

SECTIONS
{
    . = 0x10000;  /* THIS IS THE ENTRY POINT GET IT RIGHT !!!!! */

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
