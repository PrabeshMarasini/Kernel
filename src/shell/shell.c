#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../textfile/textfile.h"

#define COLOR_YELLOW 14 
#define COLOR_GREEN 10  

void delay(int iterations) {
    for (volatile int i = 0; i < iterations; i++) {
    }
}

void display_loading_animation(const char* filename) {
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

void run_shell() {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Kernel shell!\n");
    print_str("Shell> ");
    int cursor_x = 7;
    int cursor_y = 1;
    char buffer[128];
    int buffer_index = 0;

    while (1) {
        char c = keyboard_get_char();
        if (c != 0) {
            if (c == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    cursor_x--;
                    print_set_cursor(cursor_x, cursor_y);
                    print_char(' ');
                    print_set_cursor(cursor_x, cursor_y);
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';
                print_char('\n');
                cursor_y++;
                
                if (buffer_index >= 7 && buffer[0] == 'c' && buffer[1] == 'r' && buffer[2] == 'e' && buffer[3] == 'a' && buffer[4] == 't' && buffer[5] == 'e' && buffer[6] == ' ') {
                    const char *filename = &buffer[7];
                    display_loading_animation(filename);
                    print_clear();
                    display_textfile(filename);
                    
                    // Clear buffer and reset cursor after returning from textfile
                    buffer_index = 0;
                    cursor_x = 7;
                    cursor_y = 1;
                    print_clear();
                    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                    print_str("Kernel shell!\n");
                    print_str("Shell> ");
                    print_set_cursor(cursor_x, cursor_y);
                } else {
                    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                    print_str("Unknown command: ");
                    print_str(buffer);
                    print_char('\n');
                    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                    cursor_y++;
                }
                
                buffer_index = 0;
                cursor_x = 7;
                print_str("Shell> ");
                print_set_cursor(cursor_x, cursor_y);
            } else {
                if (buffer_index < sizeof(buffer) - 1) {
                    buffer[buffer_index++] = c;
                    print_char(c);
                    cursor_x++;
                }
            }
        }
    }
}
