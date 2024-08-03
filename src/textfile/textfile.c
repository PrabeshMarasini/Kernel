#include "textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../filesystem/filesystem.h"
#include "../shell/shell.h"

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define MAX_INPUT 1000
#define BUFFER_SIZE 4096

void display_textfile(const char *filename) {
    char input[MAX_INPUT] = {0};
    int input_length = 0;
    int cursor_x = 0;
    int cursor_y = 2;
    char key;
    char count_str[20];
    uint8_t file_buffer[BUFFER_SIZE];

    // Clear and initialize the screen
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        for (int j = 0; j < SCREEN_WIDTH; ++j) {
            print_set_cursor(j, i);
            print_char(' ');
        }
    }

    void update_header() {
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_set_cursor(0, 0);
        print_str(filename);

        print_set_cursor(SCREEN_WIDTH - 25, 0);
        print_str("Characters: ");

        int temp = input_length;
        int i = 0;
        do {
            count_str[i++] = (temp % 10) + '0';
            temp /= 10;
        } while (temp > 0);
        if (i == 0) count_str[i++] = '0';
        count_str[i] = '\0';

        // Reverse the count_str array
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

    int read_result = read_file(filename, file_buffer, BUFFER_SIZE);

    if (read_result > 0) {
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_set_cursor(0, 2);
        for (int i = 0; i < read_result; ++i) {
            if (file_buffer[i] == '\n') {
                cursor_y++;
                cursor_x = 0;
                if (cursor_y >= SCREEN_HEIGHT) {
                    cursor_y = 2;
                }
            } else {
                print_set_cursor(cursor_x, cursor_y);
                print_char(file_buffer[i]);
                cursor_x++;
                if (cursor_x >= SCREEN_WIDTH) {
                    cursor_x = 0;
                    cursor_y++;
                    if (cursor_y >= SCREEN_HEIGHT) {
                        cursor_y = 2;
                    }
                }
            }
        }
    }

    print_set_cursor(0, 1);
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        print_char('-');
    }
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);

    update_header();

    while (1) {
        print_set_cursor(cursor_x, cursor_y);
        key = keyboard_get_char();

        if (key == 0x1B) {  // Escape key
            // Clear the screen
            print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
            for (int i = 0; i < SCREEN_HEIGHT; ++i) {
                for (int j = 0; j < SCREEN_WIDTH; ++j) {
                    print_set_cursor(j, i);
                    print_char(' ');
                }
            }
            run_shell();  // Call the function to switch to the shell
            return;  // Exit display_textfile function
        } else if (key == '\n') {
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= SCREEN_HEIGHT) {
                cursor_y = 2;
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
            print_char(' ');
            print_set_cursor(cursor_x, cursor_y);
        } else if (key >= 32 && key <= 126 && input_length < MAX_INPUT - 1) {
            input[input_length++] = key;
            print_set_cursor(cursor_x, cursor_y);
            print_char(key);
            if (++cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
                if (cursor_y >= SCREEN_HEIGHT) {
                    cursor_y = 2;
                }
            }
        }

        update_header();
    }
}
