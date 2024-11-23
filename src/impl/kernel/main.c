#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../datetime/datetime.h"
#include <string.h>

void run_shell();

void fill_screen(char color) {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            print_set_cursor(x, y);
            print_char(' ');
        }
    }
    print_set_cursor(0, 0);
}

void reset_screen() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_clear();
}

void update_datetime() {
    struct tm current_time = get_rtc_time();
    adjust_time_for_nepal(&current_time);
    char buffer[20];
    simple_snprintf(buffer, current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday, 
                    current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_set_cursor(80 - strlen(buffer), 0);
    print_str(buffer);
}

void display_welcome_animation() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_clear();
    
    const char* welcome = "WELCOME";
    int welcome_len = strlen(welcome);
    int start_x = (80 - welcome_len) / 2;
    int start_y = 12; 

    print_set_cursor(start_x, start_y);
    print_str(welcome);

    for (int i = 0; i < 5; i++) {
        print_set_cursor(start_x + welcome_len, start_y);
        print_str("...");
        
        for (volatile int k = 0; k < 20000000; k++) {}
        
        print_set_cursor(start_x + welcome_len, start_y);
        print_str("   ");
       
        for (volatile int k = 0; k < 10000000; k++) {}
    }
    for (volatile int k = 0; k < 100000000; k++) {}

    print_clear();
}

void initialize_kernel_interface() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    fill_screen(PRINT_COLOR_BLUE);
    for (int x = 0; x < 80; x++) {
        print_set_cursor(x, 0);
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_char(' ');
    }
    print_set_cursor(0, 0);
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_str("Kernel v1");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    print_set_cursor(0, 1);
    print_str(">");
}

void kernel_main() {
    static int first_run = 1;

    if (first_run) {
        display_welcome_animation();
        first_run = 0;
    }

    initialize_kernel_interface();

    init_keyboard();

    int cursor_x = 1;
    int cursor_y = 1;
    char buffer[128];
    int buffer_index = 0;
    const char command[] = "shell";

    print_set_cursor(cursor_x, cursor_y);
    print_enable_cursor(14, 15);

    int update_counter = 0;

    while (1) {
        if (update_counter == 0) {
            update_datetime();
        }
        update_counter = (update_counter + 1) % 1000;

        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
        print_set_cursor(cursor_x, cursor_y);

        char c = keyboard_get_char();
        if (c != 0) {
            if (c == '\b') {
                if (buffer_index > 0 && cursor_x > 1) {
                    buffer_index--;
                    cursor_x--;
                    print_set_cursor(cursor_x, cursor_y);
                    print_char(' ');
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';
                if (strcmp(buffer, command) == 0) {
                    reset_screen();
                    run_shell();
                    initialize_kernel_interface();
                    cursor_y = 1;
                }
                buffer_index = 0;
                cursor_y++;
                cursor_x = 0;
                print_set_cursor(cursor_x, cursor_y);
                print_str(">");
                cursor_x = 1;
            } else if (cursor_x < 79) {
                buffer[buffer_index++] = c;
                print_char(c);
                cursor_x++;
            }
        }
    }
}