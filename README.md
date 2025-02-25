# tetrOS

tetrOS is a simple, bootable OS designed to run a game of Tetris. This document covers **compiling**, **installing dependencies**, and **running tetrOS** on **Arch Linux** and **Ubuntu/Debian**.

---

## **1. Installing Dependencies**

### **Arch Linux**
Install the required packages using `pacman`:
```bash
sudo pacman -S --needed base-devel qemu nasm
```
For musl cross-compilation:
```bash
sudo pacman -S --needed musl musl-dev musl-tools
```

### **Ubuntu/Debian**
Install required packages using `apt`:
```bash
sudo apt update
sudo apt install build-essential qemu-system-x86 nasm musl-tools
```
For musl cross-compilation:
```bash
sudo apt install musl musl-dev
```

### **Building `i686-linux-musl-gcc` Manually (If Missing)**

If `i686-linux-musl-gcc` is not available, follow these steps:

#### **Step 1: Install Build Dependencies**
```bash
sudo apt update
sudo apt install build-essential wget musl musl-dev musl-tools gcc-multilib
```

#### **Step 2: Download & Build Musl Cross Compiler**
```bash
git clone https://github.com/richfelker/musl-cross-make.git
cd musl-cross-make
```
Edit `config.mak` to specify the target architecture:
```bash
nano config.mak
```
Find `TARGET` and set it to:
```
TARGET = i686-linux-musl
```
Save and exit, then build the toolchain:
```bash
make install
```
Once finished, the binaries will be in `output/bin/`.

#### **Step 3: Add to PATH**
```bash
export PATH=$HOME/musl-cross-make/output/bin:$PATH
```
Check if `i686-linux-musl-gcc` is installed:
```bash
i686-linux-musl-gcc --version
```
If it prints a version, you’re good to go.

---

## **2. Compiling tetrOS**

### **Step 1: Assemble `bootloader.asm`**
Convert the assembly startup code into an object file:
```bash
nasm -f elf32 bootloader.asm -o bootloader_asm.o
```

### **Step 2: Compile `kernel.c`**
Compile the kernel C code with **musl** and `i686` architecture:
```bash
i686-linux-musl-gcc -ffreestanding -m32 -c kernel.c -o kernel_c.o
```

### **Step 3: Link Everything**
Link the object files into an **executable kernel**:
```bash
i686-linux-musl-ld -T linker.ld -o kernel.elf bootloader_asm.o kernel_c.o
```

---

## **3. Running tetrOS in QEMU**
Run the compiled kernel using QEMU:
```bash
qemu-system-i386 -kernel kernel.elf
```

To debug with GDB:
```bash
qemu-system-i386 -kernel kernel.elf -S -s
```
Then, in another terminal:
```bash
gdb -ex "target remote localhost:1234" kernel.elf
```

---

## **6. Contributing**
Feel free to contribute by submitting issues and pull requests.

---

## **7. License**
tetrOS is open-source and released under the **MIT License**.
