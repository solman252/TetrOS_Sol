AS = nasm
CC = i686-linux-musl-gcc
LD = i686-linux-musl-ld

ASFLAGS = -f elf32
CFLAGS = -ffreestanding -m32 -c
LDFLAGS = -T linker.ld

#source files
BOOTLOADER_SRC = bootloader.asm
KERNEL_SRC = kernel.c
KEYBOARD_SRC = keyboard.c

#object files
BOOTLOADER_OBJ = bootloader.o
KERNEL_OBJ = kernel.o
KEYBOARD_OBJ = keyboard.o

KERNEL_BIN = kernel.elf

#build target
all: $(KERNEL_BIN)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_SRC) kernel.h keyboard.h
	$(CC) $(CFLAGS) $< -o $@

$(KEYBOARD_OBJ): $(KEYBOARD_SRC) keyboard.h
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL_BIN): $(BOOTLOADER_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

#run in QEMU
run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

#debugging mode
debug: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN) -S -s

# clean build artifacts
clean:
	rm -f $(BOOTLOADER_OBJ) $(KERNEL_OBJ) $(KEYBOARD_OBJ) $(KERNEL_BIN)
