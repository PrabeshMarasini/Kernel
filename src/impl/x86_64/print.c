#include "print.h"

static int current_x = 0;
static int current_y = 0;
static int current_color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);

static unsigned short* const VGA_MEMORY = (unsigned short*)VGA_BUFFER;

static void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void print_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = (unsigned short)' ' | (unsigned short)(current_color << 8);
        }
    }
    current_x = 0;
    current_y = 0;
    print_update_cursor();
}

void print_char(char character) {
    if (character == '\n') {
        current_x = 0;
        current_y++;
    } else {
        const int index = current_y * VGA_WIDTH + current_x;
        VGA_MEMORY[index] = (unsigned short)character | (unsigned short)(current_color << 8);
        current_x++;
    }

    if (current_x >= VGA_WIDTH) {
        current_x = 0;
        current_y++;
    }

    if (current_y >= VGA_HEIGHT) {
        print_clear();
    }

    print_update_cursor();
}

void print_str(const char* string) {
    for (int i = 0; string[i] != '\0'; i++) {
        print_char(string[i]);
    }
}

void print_set_color(int foreground, int background) {
    current_color = foreground | (background << 4);
}

void print_set_cursor(int x, int y) {
    current_x = x;
    current_y = y;
    print_update_cursor();
}

void print_enable_cursor(int cursor_start, int cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void print_disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void print_update_cursor() {
    unsigned short pos = current_y * VGA_WIDTH + current_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}