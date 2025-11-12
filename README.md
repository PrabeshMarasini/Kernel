# Custom x86_64 Kernel

## Project Description

SecureOS v2.0 is a custom x86_64 kernel implementation written in C and assembly. This bare-metal operating system features advanced capabilities including memory management, device drivers, file systems, graphics rendering with anti-aliased fonts. The kernel provides a command-line interface with a built-in shell and several utility applications.

## Prerequisites

### For Windows Users

Windows users must use the Windows Subsystem for Linux (WSL) to set up a Linux environment:

1. Open PowerShell as Administrator
2. Run: `wsl --install`
3. Install Ubuntu from the Microsoft Store
4. Launch Ubuntu and complete the initial setup

All subsequent steps should be performed within the WSL environment.

### For Linux Users

The following tools and libraries are required:

- `x86_64-elf-gcc` - Cross-compiler for x86_64 architecture
- `nasm` - Netwide Assembler for assembling x86_64 assembly files
- `x86_64-elf-ld` - Cross-linker for x86_64 architecture
- `grub-mkrescue` - For creating the bootable ISO image
- `qemu-system-x86_64` - Emulator for running the kernel

On Debian-based systems (Ubuntu, etc.), install with:

```bash
sudo apt update
sudo apt install gcc-x86-64-elf nasm grub-pc-bin qemu-system-x86_64
```

On Arch-based systems, install with:

```bash
sudo pacman -S gcc-multilib nasm grub qemu-system-x86_64
```

## Building the Kernel

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. Build the kernel:
   ```bash
   make
   ```

This will create the kernel binary and a bootable ISO image in the `dist/x86_64/` directory.

## Running the Kernel

To run the kernel in QEMU emulator:

```bash
make run
```

This command will:
- Build the kernel if not already built
- Create a disk image if it doesn't exist
- Start QEMU with the kernel ISO and disk image

To manually run QEMU with the built ISO:

```bash
qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso -hda disk.img -vga std -display sdl
```

## Available Commands

### Main Interface Commands

From the main kernel interface, you can type the following commands:

- `shell` - Enter the shell interface
- `calculator` - Launch the calculator application
- `scicalc` - Launch the scientific calculator
- `snake` - Play the Snake game

### Shell Commands

In the shell interface, the following commands are available:

- `dt` - Display current date and time
- `home` - Return to main interface
- `create <filename>` - Create a new file
- `ls` - List all files
- `open <filename>` - Open and display a file
- `delete <filename>` - Delete a file
- `clr` - Clear the screen
- `font-demo` - Show font demonstration (requires graphics)
- `font-small` - Set font size to small
- `font-medium` - Set font size to medium
- `font-large` - Set font size to large
- `font-xlarge` - Set font size to extra large
- `font-bold` - Set font weight to bold
- `font-normal` - Set font weight to normal
- `font-aa-on` - Enable font anti-aliasing
- `font-aa-off` - Disable font anti-aliasing
- `font-reset` - Reset font settings to defaults
- `help` - Display this help information

## Navigation Controls

- Arrow keys for cursor movement in text input
- Home/End keys for moving to beginning/end of input
- Backspace for deleting characters

## License

This project is released under the GNU General Public License v2.0 (GPL-2.0).
