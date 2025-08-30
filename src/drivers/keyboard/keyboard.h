#ifndef KEYBOARD_H
#define KEYBOARD_H

void init_keyboard();
char keyboard_get_char();
void switch_to_shell();

#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D
#define PAGE_UP 0x49
#define PAGE_DOWN 0x51
#define HOME_KEY 0x47
#define END_KEY 0x4F

// Special key combinations
#define CTRL_HOME 0x77
#define CTRL_END 0x75

#endif
