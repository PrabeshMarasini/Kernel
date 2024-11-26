#include "../intf/print.h"

// VGA constants
#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colors
#define PRINT_COLOR_BLACK 0
#define PRINT_COLOR_BLUE 1
#define PRINT_COLOR_GREEN 2
#define PRINT_COLOR_CYAN 3
#define PRINT_COLOR_RED 4
#define PRINT_COLOR_MAGENTA 5
#define PRINT_COLOR_BROWN 6
#define PRINT_COLOR_LIGHT_GRAY 7
#define PRINT_COLOR_DARK_GRAY 8
#define PRINT_COLOR_LIGHT_BLUE 9
#define PRINT_COLOR_LIGHT_GREEN 10
#define PRINT_COLOR_LIGHT_CYAN 11
#define PRINT_COLOR_LIGHT_RED 12
#define PRINT_COLOR_LIGHT_MAGENTA 13
#define PRINT_COLOR_LIGHT_YELLOW 14
#define PRINT_COLOR_WHITE 15

static int current_color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);
static int cursor_x = 0;
static int cursor_y = 0;

static void outb(unsigned short port, unsigned char val);
static unsigned char inb(unsigned short port);

void print_clear() {
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        *video_memory++ = ' ';
        *video_memory++ = current_color;
    }
    cursor_x = 0;
    cursor_y = 0;
    print_update_cursor();
}

void print_char(char character) {
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
    video_memory[offset] = character;
    video_memory[offset + 1] = current_color;
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            // Implement scrolling here if needed
            cursor_y = VGA_HEIGHT - 1;
        }
    }
    print_update_cursor();
}

void print_str(const char* string) {
    while (*string) {
        print_char(*string++);
    }
}

void print_set_color(int foreground, int background) {
    current_color = foreground | (background << 4);
}

void print_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    print_update_cursor();
}

char print_char_at(int x, int y, char c) {
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    int offset = (y * VGA_WIDTH + x) * 2;
    char old_char = video_memory[offset];
    video_memory[offset] = c;
    video_memory[offset + 1] = current_color;
    return old_char;
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
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void print_clear_screen() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            video_memory[index * 2] = ' ';
            video_memory[index * 2 + 1] = current_color;
        }
    }

    // Reset cursor position and update cursor on screen
    cursor_x = 0;
    cursor_y = 0;
    print_update_cursor();
}

char print_get_char(int x, int y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return '\0'; 
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    return video_memory[(y * VGA_WIDTH + x) * 2]; 
}

static void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void print_get_cursor(int *x, int *y) {
    if (x) {
        *x = cursor_x;
    }
    if (y) {
        *y = cursor_y;
    }
}