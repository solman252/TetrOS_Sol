# Makefile for tetrOS

# Tools
ASM = nasm
CC = i686-linux-musl-gcc
LD = i686-linux-musl-ld

# Files
BOOTLOADER_ASM = bootloader.asm
BOOTLOADER_BIN = bootloader.bin
KERNEL_C = kernel.c
KERNEL_O = kernel.o
LINKER_SCRIPT = linker.ld
KERNEL_BIN = kernel.bin
IMG = tetrOS.img

# Default target
all: $(IMG)

# Assemble bootloader
$(BOOTLOADER_BIN): $(BOOTLOADER_ASM)
	$(ASM) -f bin $(BOOTLOADER_ASM) -o $(BOOTLOADER_BIN)

# Compile kernel (disable PIE for flat binary)
$(KERNEL_O): $(KERNEL_C)
	$(CC) -m32 -ffreestanding -fno-pie -c $(KERNEL_C) -o $(KERNEL_O)

# Link kernel using custom linker script
$(KERNEL_BIN): $(KERNEL_O) $(LINKER_SCRIPT)
	$(LD) -T $(LINKER_SCRIPT) $(KERNEL_O) -o $(KERNEL_BIN)

# Create bootable image by concatenating bootloader and kernel
$(IMG): $(BOOTLOADER_BIN) $(KERNEL_BIN)
	cat $(BOOTLOADER_BIN) $(KERNEL_BIN) > $(IMG)

# Clean target to remove built files
clean:
	rm -f $(BOOTLOADER_BIN) $(KERNEL_O) $(KERNEL_BIN) $(IMG)
