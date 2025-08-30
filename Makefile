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
calculator_source_files := src/calculator/calculator.c
calculator_object_files := build/calculator/calculator.o
snake_source_files := src/snake/snake.c
snake_object_files := build/snake/snake.o
filesystem_source_files := src/filesystem/filesystem.c
filesystem_object_files := build/filesystem/filesystem.o
keyboard_source_files := $(shell find src/drivers/keyboard -name *.c)
keyboard_object_files := $(patsubst src/drivers/keyboard/%.c, build/drivers/keyboard/%.o, $(keyboard_source_files))
memory_source_files := src/memory/memory.c
memory_object_files := build/kernel/memory.o
datetime_source_files := src/datetime/datetime.c
datetime_object_files := build/datetime/datetime.o
e1000_source_files := $(shell find src/drivers/net/e1000 -name *.c)
e1000_object_files := $(patsubst src/drivers/net/e1000/%.c, build/drivers/net/e1000/%.o, $(e1000_source_files))

disk_source_files := src/drivers/diskdriver/disk.c
disk_object_files := build/drivers/diskdriver/disk.o

graphics_source_files := $(shell find src/drivers/graphics -name *.c)
graphics_object_files := $(patsubst src/drivers/graphics/%.c, build/drivers/graphics/%.o, $(graphics_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)
all_object_files := $(kernel_object_files) $(x86_64_object_files) $(shell_object_files) $(keyboard_object_files) $(textfile_object_files) $(calculator_object_files) $(snake_object_files) $(filesystem_object_files) $(memory_object_files) $(datetime_object_files) $(e1000_object_files) $(disk_object_files) $(graphics_object_files)

build/kernel/%.o: src/impl/kernel/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/shell/%.o: src/shell/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/drivers/keyboard/%.o: src/drivers/keyboard/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/textfile/%.o: src/textfile/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/calculator/%.o: src/calculator/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/snake/%.o: src/snake/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/filesystem/%.o: src/filesystem/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/kernel/memory.o: src/memory/memory.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/datetime/%.o: src/datetime/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/drivers/net/e1000/%.o: src/drivers/net/e1000/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/drivers/diskdriver/%.o: src/drivers/diskdriver/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/drivers/graphics/%.o: src/drivers/graphics/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/intf -ffreestanding $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

.PHONY: build-x86_64
build-x86_64: $(all_object_files)
	mkdir -p dist/x86_64
	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(all_object_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso

.PHONY: build-textfile
build-textfile: $(textfile_object_files)
	mkdir -p build/textfile

.PHONY: build-calculator
build-calculator: $(calculator_object_files)
	mkdir -p build/calculator

.PHONY: build-snake
build-snake: $(snake_object_files)
	mkdir -p build/snake

DISK_IMG = disk.img
DISK_SIZE = 100M

.PHONY: run create-disk
run: build-x86_64 create-disk
	@echo "Starting QEMU with kernel and disk image..."
	qemu-system-x86_64 \
		-d int \
		-cdrom dist/x86_64/kernel.iso \
		-hda $(DISK_IMG) \
		-D qemu.log \
		-vga std \
		-display sdl

create-disk:
	@if [ ! -f $(DISK_IMG) ]; then \
		echo "Creating new disk image ($(DISK_SIZE))..."; \
		qemu-img create -f raw $(DISK_IMG) $(DISK_SIZE); \
		echo "✓ Disk image created: $(DISK_IMG)"; \
	else \
		echo "✓ Using existing disk image: $(DISK_IMG)"; \
	fi

.PHONY: clean
clean:
	@if [ -f $(DISK_IMG) ]; then \
		echo "Removing disk image..."; \
		rm -f $(DISK_IMG); \
		echo "✓ Disk image removed"; \
	fi
	# Remove all .o files in the build directory
	@echo "Removing all object files..."
	@find build -type f -name '*.o' -exec rm -f {} +
	@echo "✓ Object files removed"


