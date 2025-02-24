# tetrOS

**tetrOS** is a bare-metal operating system that only plays Tetris. It is built from scratch using a simple assembly bootloader and a C kernel.

## Getting Started

### Prerequisites
To compile and run tetrOS, you need:
- **NASM** (for assembling the bootloader)
- **i686-elf GCC** (for compiling the kernel)
- **i686-elf LD** (for linking the kernel)
- **QEMU** (for testing)

#### Installing Dependencies (Linux - Ubuntu/Debian)
```bash
sudo apt update
sudo apt install nasm qemu-system build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
```

#### Installing Dependencies (Windows)
1. Install **WSL (Windows Subsystem for Linux)** or use **MSYS2**.
2. Install NASM, QEMU, and make:
   ```bash
   pacman -S nasm qemu make
   ```
3. Download and set up the **i686-elf cross-compiler**:
   - Download a prebuilt version from [https://github.com/lordmilko/i686-elf-tools/releases](https://github.com/lordmilko/i686-elf-tools/releases)
   - Extract and add to your PATH:
     ```powershell
     setx PATH "%PATH%;C:\path\to\i686-elf-tools\bin"
     ```

#### Building the Cross-Compiler (i686-elf) (Linux/WSL)
```bash
mkdir -p ~/cross-compiler
cd ~/cross-compiler

export TARGET=i686-elf
export PREFIX="$HOME/cross-compiler"
export PATH="$PREFIX/bin:$PATH"

# Download sources
wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz

# Extract
tar -xvzf binutils-2.38.tar.gz
tar -xvzf gcc-12.2.0.tar.gz

# Build binutils
mkdir build-binutils
cd build-binutils
../binutils-2.38/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

# Build GCC
mkdir build-gcc
cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc
```

Add the cross-compiler to your PATH:
```bash
echo 'export PATH="$HOME/cross-compiler/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Project Structure
```
/
├── bootloader.asm   # 16-bit x86 Assembly bootloader
├── kernel.c         # Simple C kernel
├── linker.ld        # Linker script
├── Makefile        # Build automation script
└── README.md        # Project documentation
```

## Building tetrOS
Run the following commands to build tetrOS:
```bash
# Assemble the bootloader
nasm -f bin bootloader.asm -o boot.bin

# Compile the kernel
i686-elf-gcc -ffreestanding -c kernel.c -o kernel.o

# Link the kernel
i686-elf-ld -T linker.ld -o kernel.bin kernel.o

# Create a bootable image
cat boot.bin kernel.bin > tetrOS.img
```

### Building on Windows (MSYS2/WSL)
```bash
nasm -f bin bootloader.asm -o boot.bin
i686-elf-gcc -ffreestanding -c kernel.c -o kernel.o
i686-elf-ld -T linker.ld -o kernel.bin kernel.o
cat boot.bin kernel.bin > tetrOS.img
```

## Running tetrOS in QEMU
```bash
qemu-system-x86_64 -drive format=raw,file=tetrOS.img
```

### Running on Windows
```powershell
qemu-system-x86_64.exe -drive format=raw,file=tetrOS.img
```

## Contributing
### Coding Standards
- Bootloader: **Assembly (NASM Syntax)**
- Kernel: **C (Freestanding, No Standard Library)**
- Compiler: **i686-elf GCC (version 12.2.0 recommended)**

## License
This project is open-source under the MIT License.

