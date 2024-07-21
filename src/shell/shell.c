#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../textfile/textfile.h"

#define COLOR_YELLOW 14
#define COLOR_GREEN 10
#define COLOR_RED 4
#define COLOR_WHITE 15
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

// Function prototypes
void delay(int iterations);
void display_loading_animation(const char* filename);
void print_str_at(int x, int y, const char* str);
void clear_line(int y);
void scroll_screen();
void print_line_with_color(int x, int y, const char* str, unsigned char fg_color, unsigned char bg_color);

// Global variables
int cursor_x = 0;
int cursor_y = 0;

void delay(int iterations) {
    for (volatile int i = 0; i < iterations; i++) {
    }
}

void display_loading_animation(const char* filename) {
    // Move the cursor to a new line for the loading animation
    cursor_y++;
    if (cursor_y >= CONSOLE_HEIGHT) {
        scroll_screen();
    }
    print_set_cursor(0, cursor_y);

    // Display the loading animation
    print_set_color(COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Creating file \"");
    print_set_color(COLOR_GREEN, PRINT_COLOR_BLACK);
    print_str(filename);
    print_set_color(COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("\"");
   
    for (int i = 0; i < 5; i++) {
        delay(30000000);
        print_char('.');
    }
    print_char('\n');
}

void print_str_at(int x, int y, const char* str) {
    print_set_cursor(x, y);
    print_str(str);
}

void clear_line(int y) {
    print_set_cursor(0, y);
    for (int i = 0; i < CONSOLE_WIDTH; i++) {
        print_char(' ');
    }
}

void scroll_screen() {
    unsigned char* video_memory = (unsigned char*)VGA_BUFFER;
    unsigned char* color_memory = (unsigned char*)(VGA_BUFFER + 0x8000); // Color memory offset

    for (int y = 1; y < CONSOLE_HEIGHT; y++) {
        for (int x = 0; x < CONSOLE_WIDTH; x++) {
            int current_index = (y * CONSOLE_WIDTH + x) * 2;
            int prev_index = ((y - 1) * CONSOLE_WIDTH + x) * 2;

            // Move characters up
            video_memory[prev_index] = video_memory[current_index];
            color_memory[prev_index] = color_memory[current_index];
        }
    }
    // Clear the last line
    clear_line(CONSOLE_HEIGHT - 1);
    cursor_y = CONSOLE_HEIGHT - 1;
}

void print_line_with_color(int x, int y, const char* str, unsigned char fg_color, unsigned char bg_color) {
    print_set_color(fg_color, bg_color);
    print_set_cursor(x, y);
    print_str(str);
}

void run_shell() {
    print_clear();
    cursor_x = 0;
    cursor_y = 0;
    print_set_color(COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Kernel shell!");
    print_set_cursor(0, 1);
    print_str("Shell>");
    print_enable_cursor(14, 15);
    cursor_x = 7;
    cursor_y = 1;
    char buffer[128];
    int buffer_index = 0;

    while (1) {
        char c = keyboard_get_char();
        if (c != 0) {
            if (c == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    if (cursor_x > 7) {
                        cursor_x--;
                        print_set_cursor(cursor_x, cursor_y);
                        print_char(' ');
                        print_set_cursor(cursor_x, cursor_y);
                    }
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';
                print_char(' ');
                cursor_y++;
               
                if (cursor_y >= CONSOLE_HEIGHT) {
                    scroll_screen();
                }

                if (buffer_index >= 7 && buffer[0] == 'c' && buffer[1] == 'r' && buffer[2] == 'e' && buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == ' ') {
                    const char *filename = &buffer[7];
                    display_loading_animation(filename);
                    print_clear();
                    display_textfile(filename);
                   
                    // Clear buffer and reset cursor after returning from textfile
                    buffer_index = 0;
                    cursor_x = 0;
                    cursor_y = 0;
                    print_clear();
                    print_set_color(COLOR_WHITE, PRINT_COLOR_BLACK);
                    print_str("Kernel shell!\n");
                    print_str("Shell> ");
                    cursor_x = 7;
                    cursor_y = 1;
                } else {
                    // Clear the line where the prompt and the command were typed
                    clear_line(cursor_y);

                    // Move the cursor to the start of the current line
                    print_set_cursor(0, cursor_y);
                    
                    // Print error message with red color
                    print_line_with_color(0, cursor_y, "Unknown command: ", COLOR_RED, PRINT_COLOR_BLACK);
                    print_str(buffer);
                    print_set_color(COLOR_WHITE, PRINT_COLOR_BLACK);

                    // Ensure the prompt is displayed on a new line
                    cursor_y++;
                    if (cursor_y >= CONSOLE_HEIGHT) {
                        scroll_screen();
                    }
                    print_set_cursor(0, cursor_y);
                    print_str("Shell> ");
                    cursor_x = 7;
                }
               
                buffer_index = 0;
            } else {
                if (buffer_index < sizeof(buffer) - 1 && cursor_x < CONSOLE_WIDTH - 1) {
                    buffer[buffer_index++] = c;
                    print_char(c);
                    cursor_x++;
                }
            }
        }
    }
}