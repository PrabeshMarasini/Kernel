# Kernel Project

## Required Libraries and Tools

To build and run this project, you will need the following libraries and tools:

### On Windows

If you are using Windows, you will need to install the Windows Subsystem for Linux (WSL) to get a Linux environment. To do this, open a PowerShell window as an administrator and run:

wsl --install

### On Linux

1. **GNU Compiler Collection (GCC) for x86_64**
   - `x86_64-elf-gcc` (part of the cross-compiler toolchain)

2. **Netwide Assembler (NASM)**
   - `nasm` for assembling x86_64 assembly files

3. **GNU Linker (LD) for x86_64**
   - `x86_64-elf-ld` (part of the cross-compiler toolchain)

4. **GRUB**
   - `grub-mkrescue` for creating the ISO image

5. **QEMU**
   - `qemu-system-x86_64` for running the kernel image in an emulator

You can install these tools using your package manager. For example, on Debian-based systems (like Ubuntu), you can run:

```bash
sudo apt update
sudo apt install gcc-x86-64-elf nasm grub-pc-bin qemu

## Building the project

### Clone the repository:

git clone <repository-url>
cd <repository-directory>

## Run make to build the project:

make

## Run the kernel in QEMU:

make run


