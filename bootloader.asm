BITS 16
ORG 0x7C00

start:
    cli
    mov ax, 0x0000    ; set up segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov sp, 0x7E00    ; set up stack higher to stop it from overwriting bootloader

    mov si, message
    call print_string

    ; Load kernel from disk
    mov ax, 0x1000    ; segment where kernel will be loaded
    mov es, ax
    mov bx, 0x0000    ; offset in segment
    mov dh, 2         ; number of sectors to read
    call disk_load

    ; jump to kernel if it works
    jmp 0x1000:0000

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done:
    ret

disk_load:
    mov si, disk_reading_msg
    call print_string

    mov ah, 0x02    ; bios disk read function
    mov al, dh      ; # of sectors to read
    mov ch, 0x00    ; cylinder 0
    mov dh, 0x00    ; head 0
    mov cl, 0x02    ; start reading at sector 2
    mov dl, 0x80    ; first hard disk
    int 0x13
    jc error        ; if carry flag set make it print error

    mov si, disk_read_success
    call print_string

    ret

error:
    mov si, disk_fail
    call print_string
    hlt

message db "Booting tetrOS...", 0
disk_reading_msg db "Reading kernel...", 0
disk_read_success db "Kernel loaded!", 0
disk_fail db "Disk read failed!", 0

times 510-($-$$) db 0
dw 0xAA55
