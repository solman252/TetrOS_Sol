BITS 16
ORG 0x7C00

start:
    cli
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7C00

    mov si, message
    call print_string

    ; load kernal by reading from disk
    mov bx, 0x1000  ; load address for kernel
    mov dh, 2       ; # of sectors to read
    call disk_load

    jmp 0x1000      ; jump to loaded kernel

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
    mov ah, 0x02    ; bios disk read function
    mov al, dh      ; # of sectors to read
    mov ch, 0x00    ; cylinder 0
    mov dh, 0x00    ; head 0
    mov cl, 0x02    ; start reading at sector 2
    mov dl, 0x80    ; first hard disk
    int 0x13
    jc error        ; if carry flag set, print error
    ret

error:
    mov si, disk_fail
    call print_string
    hlt

message db 'Booting tetrOS...', 0
disk_fail db 'stupid ass disk broke', 0

times 510-($-$$) db 0
dw 0xAA55
