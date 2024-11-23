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

void save_current_file(void);

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

char keyboard_get_char() {
    static int shift = 0;   
    static int ctrl = 0;    
    char c = 0;

    if (inb(KEYBOARD_STATUS_PORT) & 1) {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode & 0x80) {
            scancode -= 0x80;
            if (scancode == LSHIFT || scancode == RSHIFT) {
                shift = 0;
            } else if (scancode == LCTRL || scancode == RCTRL) {
                ctrl = 0;
            }
        } else {
            if (scancode == LSHIFT || scancode == RSHIFT) {
                shift = 1;
            } else if (scancode == LCTRL || scancode == RCTRL) {
                ctrl = 1;
            } else if (scancode == UP_ARROW || scancode == DOWN_ARROW || scancode == LEFT_ARROW || scancode == RIGHT_ARROW) {
                c = scancode;
            } else {
                if (shift) {
                    c = shift_keymap[scancode];
                } else {
                    c = keymap[scancode];
                }
                if (ctrl && !shift && (c == 's' || c == 'S')) {
                    return 0x13;
                }
                if (ctrl && shift && (c == 's' || c == 'S')) {
                    return 0x1B; 
                }
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
        switch_to_shell();
    } else if (c == 0x13) {
        save_current_file();
    } else if (c == UP_ARROW || c == DOWN_ARROW || c == LEFT_ARROW || c == RIGHT_ARROW) {
        print_char('^');
    } else if (c != 0) {
        print_char(c);
    }
}

void init_keyboard(){
    
}
