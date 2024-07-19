#include "textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"

#define SCREEN_HEIGHT 25  
#define SCREEN_WIDTH 80  
#define MAX_INPUT 1000  

void display_textfile(const char *filename) {
    char input[MAX_INPUT] = {0};
    int input_length = 0;
    int cursor_x = 0;
    int cursor_y = 2;
    char key;
    char count_str[20]; 

    // Clear and initialize the screen
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        for (int j = 0; j < SCREEN_WIDTH; ++j) {
            print_set_cursor(j, i);
            print_char(' ');
        }
    }

    void update_header() {
        // Set header colors
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_set_cursor(0, 0);
        print_str(filename);
        
        print_set_cursor(SCREEN_WIDTH - 25, 0);
        print_str("Characters: ");
        
        // Convert input_length to string
        int temp = input_length;
        int i = 0;
        do {
            count_str[i++] = (temp % 10) + '0';
            temp /= 10;
        } while (temp > 0);
        if (i == 0) count_str[i++] = '0';
        count_str[i] = '\0';
        
        // Reverse the string
        for (int j = 0; j < i / 2; j++) {
            char tmp = count_str[j];
            count_str[j] = count_str[i - j - 1];
            count_str[i - j - 1] = tmp;
        }
        
        print_str(count_str);
        for (int i = 0; i < 5; i++) {
            print_char(' ');
        }
    }

    update_header();

    // Draw the border below the header
    print_set_cursor(0, 1);
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        print_char('-');
    }
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);

    while (1) {
        print_set_cursor(cursor_x, cursor_y);

        key = keyboard_get_char();

        if (key == '\n') {
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= SCREEN_HEIGHT) {
                cursor_y = 2;  // Reset to start of text area if we go beyond screen
                // Scroll text area logic can be added here if needed
            }
        } else if (key == '\b' && input_length > 0) {
            input[--input_length] = '\0';
            if (cursor_x > 0) {
                cursor_x--;
            } else if (cursor_y > 2) {
                cursor_y--;
                cursor_x = SCREEN_WIDTH - 1;
            }
            print_set_cursor(cursor_x, cursor_y);
            print_char(' ');  // Clear the character on screen
            print_set_cursor(cursor_x, cursor_y);
        } else if (key >= 32 && key <= 126 && input_length < MAX_INPUT - 1) {
            input[input_length++] = key;
            print_set_cursor(cursor_x, cursor_y);
            print_char(key);
            if (++cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
                if (cursor_y >= SCREEN_HEIGHT) {
                    cursor_y = 2;  // Reset to start of text area if we go beyond screen
                    // Scroll text area logic can be added here if needed
                }
            }
        }

        update_header();
    }
}
