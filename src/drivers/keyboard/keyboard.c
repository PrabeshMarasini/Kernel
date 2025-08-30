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

// Standard PC keyboard scancodes mapping - exactly 120 elements (0-119)
unsigned char keymap[120] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   // 0-14
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   // 15-28
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,        // 29-42
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',   // 43-57
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 58-73
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 74-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 90-105
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                 // 106-119
};

unsigned char shift_keymap[120] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',   // 0-14
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   // 15-28
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,        // 29-42
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',   // 43-57
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 58-73
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 74-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                          // 90-105
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                 // 106-119
};

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

unsigned char keyboard_get_char() {
    static int shift = 0;   
    static int ctrl = 0;    
    unsigned char c = 0;

    // Quick check if keyboard has data
    if (!(inb(KEYBOARD_STATUS_PORT) & 1)) {
        return 0; // No data available
    }

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
        } else if (scancode == UP_ARROW || scancode == DOWN_ARROW || scancode == LEFT_ARROW || scancode == RIGHT_ARROW ||
                   scancode == PAGE_UP || scancode == PAGE_DOWN || scancode == HOME_KEY || scancode == END_KEY) {
            // Handle special navigation keys - return SPECIAL CODES, not scancodes
            if (ctrl) {
                // Handle Ctrl combinations
                if (scancode == HOME_KEY) {
                    c = CTRL_HOME;
                } else if (scancode == END_KEY) {
                    c = CTRL_END;
                } else {
                    c = scancode; // Other keys with Ctrl (future expansion)
                }
            } else {
                // Return SPECIAL NAVIGATION CODES (not conflicting ASCII values)
                c = scancode | 0x80; // Set high bit to distinguish from ASCII
            }
        } else if (scancode < 120) { // Bounds check for our keymap size
            // CHARACTER GENERATION - only for actual typeable keys
            if (shift) {
                c = shift_keymap[scancode];
                // Enhanced fallback for common letters if mapping failed
                if (c == 0) {
                    if (scancode >= 16 && scancode <= 25) {
                        // QWERTY top row: Q W E R T Y U I O P
                        char qwerty[] = "QWERTYUIOP";
                        c = qwerty[scancode - 16];
                    } else if (scancode >= 30 && scancode <= 38) {
                        // ASDF row: A S D F G H J K L
                        char asdf[] = "ASDFGHJKL";
                        c = asdf[scancode - 30];
                    } else if (scancode >= 44 && scancode <= 50) {
                        // ZXCV row: Z X C V B N M
                        char zxcv[] = "ZXCVBNM";
                        c = zxcv[scancode - 44];
                    }
                }
            } else {
                c = keymap[scancode];
            }
            
            // Only handle specific Ctrl combinations, don't block everything
            if (ctrl) {
                if (c == 's' || c == 'S') {
                    return 0x13; // Ctrl+S for save
                } else if (shift && (c == 's' || c == 'S')) {
                    return 0x1B; // Ctrl+Shift+S for exit
                }
                // For other keys, let them through normally when Ctrl is pressed
                // This prevents Ctrl from blocking regular typing
            }
        }
    }

    return c;
}

void handle_keypress() {
    unsigned char c = keyboard_get_char();
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
