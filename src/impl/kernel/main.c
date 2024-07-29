#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"

void run_shell();

// New function to fill the screen with a color
void fill_screen(char color) {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            print_set_cursor(x, y);
            print_char(' ');
        }
    }
    print_set_cursor(0, 0);
}

// New function to reset screen to default state
void reset_screen() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_clear();
}

void kernel_main() {
    // Fill the entire screen with blue
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    fill_screen(PRINT_COLOR_BLUE);
    
    // Set the first line to white background
    for (int x = 0; x < 80; x++) {
        print_set_cursor(x, 0);
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_char(' ');
    }
    
    // Print "Kernel v1" in black on the white background
    print_set_cursor(0, 0);
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_str("Kernel v1");
    
    // Reset color for the rest of the screen
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    
    // Set cursor and print prompt
    print_set_cursor(0, 1);
    print_str(">");
    print_enable_cursor(14, 15);
    init_keyboard();
    int cursor_x = 1;
    int cursor_y = 1;
    print_set_cursor(cursor_x, cursor_y);
    char buffer[128];
    int buffer_index = 0;
    const char command[] = "shell";
    int command_index = 0;
    while (1) {
        char c = keyboard_get_char();
        if (c != 0) {
            if (c == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    cursor_x--;
                    if (cursor_x < 0) {
                        cursor_y--;
                        cursor_x = 79; // Move to the end of the previous line
                    }
                    print_set_cursor(cursor_x, cursor_y);
                    print_char(' ');
                    print_set_cursor(cursor_x, cursor_y);
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';
                if (buffer_index == command_index && buffer_index == 5 &&
                    buffer[0] == 's' && buffer[1] == 'h' && buffer[2] == 'e' &&
                    buffer[3] == 'l' && buffer[4] == 'l') {
                    reset_screen(); // Reset screen before entering shell
                    run_shell();
                    // After returning from shell, restore blue background
                    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
                    fill_screen(PRINT_COLOR_BLUE);
                    print_str("Kernel v1");
                    cursor_y = 1;
                    cursor_x = 0;
                }
                buffer_index = 0;
                command_index = 0;
                cursor_y++;
                cursor_x = 0;
                print_set_cursor(cursor_x, cursor_y);
                print_char('>'); // Re-print the prompt after a newline
            } else {
                buffer[buffer_index++] = c;
                print_char(c);
                cursor_x++;
                if (cursor_x >= 80) {
                    cursor_x = 0;
                    cursor_y++;
                }
                if (c == command[command_index]) {
                    command_index++;
                } else {
                    command_index = 0;
                }
            }
        }
    }
}