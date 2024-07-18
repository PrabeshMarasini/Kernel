#include "../intf/print.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

unsigned char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

char get_key() {
    char c = 0;
    if (inb(KEYBOARD_STATUS_PORT) & 1) {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);
        if (scancode & 0x80) {
            // Key release
        } else {
            c = keymap[scancode];
        }
    }
    return c;
}

// Function prototype for run_shell in shell.c
void run_shell();

void kernel_main() {
    print_clear();
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
    print_str("Kernel v1\n");
    print_str("Start typing:\n");
    print_enable_cursor(14, 15);  // Set cursor shape

    int cursor_x = 0;
    int cursor_y = 2;  // Assuming "Start typing:" is on line 1

    char buffer[128];  // Command buffer
    int buffer_index = 0;

    // Command to compare against ("shell" in this case)
    const char command[] = "shell";
    int command_index = 0;  // Track position within the command string

    while (1) {
        char c = get_key();
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
                buffer[buffer_index] = '\0';  // Null-terminate the buffer

                // Check if the entered command matches "shell"
                if (buffer_index == command_index && buffer_index == 5 && buffer[0] == 's' && buffer[1] == 'h' && buffer[2] == 'e' && buffer[3] == 'l' && buffer[4] == 'l') {
                    run_shell();  // Call run_shell from shell.c
                }

                // Reset buffer and command index for next command
                buffer_index = 0;
                command_index = 0;
                cursor_y++;
                cursor_x = 0;
                print_set_cursor(cursor_x, cursor_y);
            } else {
                buffer[buffer_index++] = c;
                print_char(c);
                cursor_x++;
                if (cursor_x >= 80) {  // Assuming 80 columns
                    cursor_x = 0;
                    cursor_y++;
                }

                // Check against the command string character by character
                if (c == command[command_index]) {
                    command_index++;
                } else {
                    command_index = 0;  // Reset if not matching
                }
            }
        }
    }
}
