#include "textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../filesystem/filesystem.h"
#include "../shell/shell.h"

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define MAX_INPUT 1000
#define BUFFER_SIZE 4096

void clear_and_reset_screen(void);
void textfile_scroll_screen(void);
void textfile_scroll_horizontal(int direction);
void display_save_message(const char *message);

// Function to clear the entire screen with a white background
void clear_screen(void) {
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); // Set white background
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            print_set_cursor(x, y);
            print_char(' '); // Fill the screen with spaces
        }
    }
    print_set_cursor(0, 0); // Reset cursor position
}

// Function to scroll the screen up by one line
void textfile_scroll_screen(void) {
    for (int y = 2; y < SCREEN_HEIGHT - 1; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            print_set_cursor(x, y);
            char ch = print_get_char(x, y + 1);
            print_char(ch);
        }
    }
    // Clear the last line
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        print_set_cursor(x, SCREEN_HEIGHT - 1);
        print_char(' ');
    }
}

// Function to scroll the screen horizontally by one character
void textfile_scroll_horizontal(int direction) {
    for (int y = 2; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH - 1; ++x) {
            int src_x = (direction > 0) ? x + 1 : x;
            int dest_x = (direction > 0) ? x : x + 1;
            print_set_cursor(dest_x, y);
            char ch = print_get_char(src_x, y);
            print_char(ch);
        }
        print_set_cursor((direction > 0) ? SCREEN_WIDTH - 1 : 0, y);
        print_char(' ');
    }
}

// Function to update the filename section
void update_filename(const char *filename) {
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); // Set background for filename section
    print_set_cursor(0, 0);
    print_str(filename);
}

// Function to update the character count section
void update_char_count(int count) {
    static int last_count = -1; // Initialize to an invalid value

    if (count == last_count) {
        return; // No change in count, so no need to update
    }

    last_count = count;
    char count_str[20];
    
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); // Set background for count section
    print_set_cursor(SCREEN_WIDTH - 25, 0);
    print_str("Characters: ");

    int temp = count;
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
        print_char(' '); // Fill extra space for alignment
    }
}

// Function to update the text content section
void update_text_content(const uint8_t *buffer, int length) {
    int cursor_x = 0;
    int cursor_y = 2; // Start after header

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK); // Set text content background color
    print_set_cursor(0, cursor_y);
    for (int i = 0; i < length; ++i) {
        if (buffer[i] == '\n') {
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= SCREEN_HEIGHT) {
                textfile_scroll_screen();
                cursor_y = SCREEN_HEIGHT - 1;
            }
        } else {
            print_set_cursor(cursor_x, cursor_y);
            print_char(buffer[i]);
            cursor_x++;
            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
                if (cursor_y >= SCREEN_HEIGHT) {
                    textfile_scroll_screen();
                    cursor_y = SCREEN_HEIGHT - 1;
                }
            }
        }
    }
}

// Function to clear the screen and reset it
void clear_and_reset_screen(void) {
    clear_screen(); // Clear the screen
    // Reset the screen colors
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, 0);
    print_str("Kernel v1");
}

void delay(int milliseconds) {
    // Assuming the CPU runs at a certain number of cycles per millisecond
    // You'll need to adjust this constant based on your actual CPU speed
    volatile int cycles = milliseconds * 1000;
    
    while (cycles > 0) {
        cycles--;
    }
}

void display_save_message(const char *message) {
    // Display the popup message
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE); // White text on blue background
    int popup_start_x = (SCREEN_WIDTH - 30) / 2; // Center the popup horizontally
    int popup_start_y = (SCREEN_HEIGHT - 3) / 2; // Center the popup vertically
    print_set_cursor(popup_start_x, popup_start_y);
    for (int i = 0; i < 30; ++i) {
        print_char(' '); // Top border
    }
    print_set_cursor(popup_start_x, popup_start_y + 1);
    print_char(' ');
    print_str(message);
    print_char(' ');
    print_set_cursor(popup_start_x, popup_start_y + 2);
    for (int i = 0; i < 30; ++i) {
        print_char(' '); // Bottom border
    }

    // Wait for 5 seconds before clearing the popup
    delay(80000);

    // Clear the popup message
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); // Reset color
    for (int y = popup_start_y; y <= popup_start_y + 2; ++y) {
        print_set_cursor(popup_start_x, y);
        for (int x = 0; x < 30; ++x) {
            print_char(' '); // Clear the area
        }
    }
}

// Function to save the current content to the file
void save_current_file(const char *filename, const uint8_t *content, int length) {
    int save_result = write_file(filename, content, length);
    if (save_result < 0) {
        display_save_message("Error saving file.");
    } else {
        display_save_message("File saved successfully.");
    }
}

void display_textfile(const char *filename) {
    char input[MAX_INPUT] = {0};
    int input_length = 0;
    int cursor_x = 0;
    int cursor_y = 2;
    char key;
    uint8_t file_buffer[BUFFER_SIZE];

    // Clear and initialize the screen
    clear_screen();
    update_filename(filename);

    // Read file and update content section
    int read_result = read_file(filename, file_buffer, BUFFER_SIZE);

    if (read_result > 0) {
        update_text_content(file_buffer, read_result);
        // Copy the content into the input buffer
        for (int i = 0; i < read_result; i++) {
            if (input_length < MAX_INPUT) { // Prevent buffer overflow
                input[input_length++] = file_buffer[i];
            } else {
                // Handle buffer overflow
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
                print_set_cursor(0, SCREEN_HEIGHT - 1);
                print_str("Error: Input buffer overflow");
                while (1); // Halt
            }
        }
    }

    // Update the separator line
    print_set_cursor(0, 1);
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        print_char('-');
    }
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);

    // Initial update of character count
    update_char_count(input_length);

    while (1) {
        print_set_cursor(cursor_x, cursor_y);
        key = keyboard_get_char();

        if (key == 0x1B) {  // Escape key
            // Clear the screen and reset
            clear_screen();
            clear_and_reset_screen(); // Ensure screen colors are reset
            run_shell();  // Call the function to switch to the shell
            return;  // Exit display_textfile function
        } else if (key == '\n') {
            if (input_length < MAX_INPUT - 1) { // Ensure buffer has space
                input[input_length++] = key;  // Store the input
                cursor_y++;
                cursor_x = 0;
                if (cursor_y >= SCREEN_HEIGHT) {
                    textfile_scroll_screen();
                    cursor_y = SCREEN_HEIGHT - 1;
                }
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
        } else if (key == 0x13) {  // Ctrl + S for saving
            save_current_file(filename, (const uint8_t *)input, input_length);
        } else if (key >= 32 && key <= 126) {  // Printable characters
            if (input_length < MAX_INPUT - 1) { // Ensure buffer has space
                input[input_length++] = key;  // Store the input
                print_char(key);
                cursor_x++;
                if (cursor_x >= SCREEN_WIDTH) {
                    cursor_x = 0;
                    cursor_y++;
                    if (cursor_y >= SCREEN_HEIGHT) {
                        textfile_scroll_screen();
                        cursor_y = SCREEN_HEIGHT - 1;
                    }
                }
            }
        }

        update_char_count(input_length); // Update the character count
    }
}