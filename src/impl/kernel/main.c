#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"

void run_shell();

void kernel_main() {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Kernel v1\n");
    print_str(">");
    print_enable_cursor(14, 15);

    init_keyboard();

    int cursor_x = 0;
    int cursor_y = 2;

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
                    print_set_cursor(cursor_x, cursor_y);
                    print_char(' ');
                    print_set_cursor(cursor_x, cursor_y);
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';

                if (buffer_index == command_index && buffer_index == 5 && buffer[0] == 's' && buffer[1] == 'h' && buffer[2] == 'e' && buffer[3] == 'l' && buffer[4] == 'l') {
                    run_shell();
                }

                buffer_index = 0;
                command_index = 0;
                cursor_y++;
                cursor_x = 0;
                print_set_cursor(cursor_x, cursor_y);
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
