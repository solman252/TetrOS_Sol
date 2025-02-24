BITS 16
ORG 0x7C00

start:
    cli
    mov ax, 0x0000  ; Set up segment registers correctly
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov sp, 0x7E00  ; Set stack higher in memory

    mov si, message
    call print_string

    ; Load kernel by reading from disk
    mov ax, 0x1000  ; Segment where kernel will be loaded
    mov es, ax
    mov bx, 0x0000  ; Offset in segment
    mov dh, 2       ; # of sectors to read
    call disk_load

    jmp 0x1000:0000  ; Jump to loaded kernel

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
    mov ah, 0x02    ; BIOS disk read function
    mov al, dh      ; # of sectors to read
    mov ch, 0x00    ; Cylinder 0
    mov dh, 0x00    ; Head 0
    mov cl, 0x02    ; Start reading at sector 2
    mov dl, 0x80    ; First hard disk
    int 0x13
    jc error        ; If carry flag set, print error
    ret

error:
    mov si, disk_fail
    call print_string
    hlt

message db 'Booting tetrOS...', 0
disk_fail db 'stupid ass disk broke', 0

times 510-($-$$) db 0  ; Pad to 512 bytes
dw 0xAA55               ; Boot signature
