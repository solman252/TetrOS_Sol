AS = nasm
CC = i686-linux-musl-gcc
LD = i686-linux-musl-ld

ASFLAGS = -f elf32
CFLAGS = -ffreestanding -m32 -c
LDFLAGS = -T linker.ld

BOOTLOADER_SRC = bootloader.asm
KERNEL_SRC = kernel.c
BOOTLOADER_OBJ = bootloader_asm.o
KERNEL_OBJ = kernel_c.o
KERNEL_BIN = kernel.elf

all: $(KERNEL_BIN)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL_BIN): $(BOOTLOADER_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

debug: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN) -S -s

clean:
	rm -f $(BOOTLOADER_OBJ) $(KERNEL_OBJ) $(KERNEL_BIN)
