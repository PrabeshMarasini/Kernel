#include "keyboard.h"
#include "../intf/print.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_RELEASE 0xB6
#define LCTRL 0x1D
#define RCTRL 0x1D
#define CTRL_RELEASE 0x9D

unsigned char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

unsigned char shift_keymap[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void init_keyboard() {
    // Initialize keyboard hardware (if necessary)
}

char keyboard_get_char() {
    static int shift = 0;   // Shift state: 0 = not pressed, 1 = pressed
    static int ctrl = 0;    // Ctrl state: 0 = not pressed, 1 = pressed
    char c = 0;

    if (inb(KEYBOARD_STATUS_PORT) & 1) {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode & 0x80) {
            // Key release
            scancode -= 0x80;
            if (scancode == LSHIFT || scancode == RSHIFT) {
                shift = 0;
            } else if (scancode == LCTRL || scancode == RCTRL) {
                ctrl = 0;
            }
        } else {
            // Key press
            if (scancode == LSHIFT || scancode == RSHIFT) {
                shift = 1;
            } else if (scancode == LCTRL || scancode == RCTRL) {
                ctrl = 1;
            } else {
                if (shift) {
                    c = shift_keymap[scancode];
                } else {
                    c = keymap[scancode];
                }

                // Check for Ctrl + S
                if (ctrl && !shift && (c == 's' || c == 'S')) {
                    return 0x13; // ASCII code for Ctrl + S
                }
                
                // Check for Ctrl + Shift + S
                if (ctrl && shift && (c == 's' || c == 'S')) {
                    return 0x1B; // Special code for Ctrl + Shift + S
                }

                // If Ctrl is pressed, don't return any character
                if (ctrl) {
                    return 0;
                }
            }
        }
    }

    return c;
}

void handle_keypress() {
    char c = keyboard_get_char();
    if (c == 0x1B) {
        // Handle Ctrl + Shift + S (switch to shell)
        switch_to_shell();  // Ensure this function is declared and defined globally
    } else if (c == 0x13) {
        // Handle Ctrl + S (save file)
        save_current_file(); // Ensure this function is declared and defined globally
    } else if (c != 0) {
        print_char(c);  // Print the character
    }
}
