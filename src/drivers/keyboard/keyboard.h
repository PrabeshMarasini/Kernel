#ifndef KEYBOARD_H
#define KEYBOARD_H

void init_keyboard();
char keyboard_get_char();
void switch_to_shell();

#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D

#endif
