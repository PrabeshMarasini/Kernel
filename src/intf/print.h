#ifndef PRINT_H
#define PRINT_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

enum {
    PRINT_COLOR_BLACK = 0,
    PRINT_COLOR_BLUE = 1,
    PRINT_COLOR_GREEN = 2,
    PRINT_COLOR_CYAN = 3,
    PRINT_COLOR_RED = 4,
    PRINT_COLOR_MAGENTA = 5,
    PRINT_COLOR_BROWN = 6,
    PRINT_COLOR_LIGHT_GRAY = 7,
    PRINT_COLOR_DARK_GRAY = 8,
    PRINT_COLOR_LIGHT_BLUE = 9,
    PRINT_COLOR_LIGHT_GREEN = 10,
    PRINT_COLOR_LIGHT_CYAN = 11,
    PRINT_COLOR_LIGHT_RED = 12,
    PRINT_COLOR_PINK = 13,
    PRINT_COLOR_YELLOW = 14,
    PRINT_COLOR_WHITE = 15,
};

void print_clear();
void print_char(char character);
void print_str(const char* string);
void print_set_color(int foreground, int background);
void print_set_cursor(int x, int y);
void print_enable_cursor(int cursor_start, int cursor_end);
void print_disable_cursor();
void print_update_cursor();
void print_clear_screen();
char print_char_at(int x, int y, char c);
char print_get_char(int x, int y);
void print_get_cursor(int *x, int *y);


#endif
