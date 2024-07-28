# Define source files
kernel_source_files := $(shell find src/impl/kernel -name *.c)
kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find src/impl/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/impl/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/impl/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/impl/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

shell_source_files := $(shell find src/shell -name *.c)
shell_object_files := $(patsubst src/shell/%.c, build/shell/%.o, $(shell_source_files))

textfile_source_files := src/textfile/textfile.c
textfile_object_files := build/textfile/textfile.o

# Add filesystem source files
filesystem_source_files := src/filesystem/filesystem.c
filesystem_object_files := build/filesystem/filesystem.o

# Add keyboard source files
keyboard_source_files := $(shell find src/drivers/keyboard -name *.c)
keyboard_object_files := $(patsubst src/drivers/keyboard/%.c, build/drivers/keyboard/%.o, $(keyboard_source_files))

# Add memory source files
memory_source_files := src/memory/memory.c
memory_object_files := build/kernel/memory.o

# Define all object files
x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)
all_object_files := $(kernel_object_files) $(x86_64_object_files) $(shell_object_files) $(keyboard_object_files) $(textfile_object_files) $(filesystem_object_files) $(memory_object_files)

# Compilation rules for kernel, x86_64 C files, x86_64 assembly files, shell files, keyboard files, textfile files, filesystem files, and memory files
build/kernel/%.o: src/impl/kernel/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

build/shell/%.o: src/shell/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/drivers/keyboard/%.o: src/drivers/keyboard/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/textfile/%.o: src/textfile/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/filesystem/%.o: src/filesystem/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/kernel/memory.o: src/memory/memory.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

# Build target for x86_64
.PHONY: build-x86_64
build-x86_64: $(all_object_files)
	mkdir -p dist/x86_64
	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(all_object_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso

# Build target for textfile
.PHONY: build-textfile
build-textfile: $(textfile_object_files)
	mkdir -p build/textfile

# Run target to execute kernel in QEMU
.PHONY: run
run: build-x86_64
	qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso
