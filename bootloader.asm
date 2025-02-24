[BITS 16]
org 0x7c00

start:
    cli
    mov si, load_msg
    call print_string
    mov ax, 0x1000
    mov es, ax
    mov bx, 0x0000
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, 0x80
    int 0x13
    jc disk_error
    mov si, loaded_msg
    call print_string
    call enable_a20
    call switch_to_pm

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

enable_a20:
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

switch_to_pm:
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry

[BITS 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x08:0x10000

gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

load_msg   db "Loading kernel...", 0
loaded_msg db "Kernel loaded!", 0
error_msg  db "Disk read error!", 0

times 510 - ($ - $$) db 0
dw 0xAA55
