#include "../textfile/textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../filesystem/filesystem.h"
#include <string.h>
#include <stdlib.h>
#include "shell.h"
#include "../datetime/datetime.h"

#define SCREEN_HEIGHT 25  
#define SCREEN_WIDTH 80
#define MAX_INPUT 1000  

// Define the VGA character structure
struct vga_char {
    char character;
    char color;
};

// VGA buffer pointer
volatile struct vga_char* terminal_buffer = (volatile struct vga_char*)0xB8000;

// Global variables
static int cursor_x = 7;
static int cursor_y = 1;

// Functions
void kernel_main();
void handle_special_keys(char key);
void run_shell();
void clear_line(int y);
void print_line_with_color(int x, int y, const char *line, int fg, int bg);
void display_loading_animation(const char *filename);
void scroll_screen();
void list_files_command();
void open_file_command(const char *filename);
void delete_file_command(const char *filename);
void set_first_line_color();
void start_shell();  // Declare start_shell

void handle_special_keys(char key) {
    static char input[MAX_INPUT] = {0};
    static int input_length = 0;

    if (key == '\n') {
        cursor_y++;
        cursor_x = 0;
        if (cursor_y >= SCREEN_HEIGHT) {
            scroll_screen();
        }
    } else if (key == '\b' && input_length > 0) {
        input[--input_length] = '\0';
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 1) {
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
                scroll_screen();
            }
        }
    }
}

// Date and Time Function
void display_time() {
    struct tm time = get_rtc_time();
    adjust_time_for_nepal(&time);
    print_set_color(PRINT_COLOR_CYAN, PRINT_COLOR_BLACK);
    print_set_cursor(0, 1);  // Display on the second line
    print_time(&time);
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
}

void clear_time_line() {
    print_set_cursor(0, 1);  // Assuming the time is displayed on the second line
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        print_char(' ');
    }
}


void run_shell() {
    while (1) {
        // Clear the screen and reset cursor position
        print_clear();
        set_first_line_color();
        print_set_cursor(0, 1);
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_str("Shell> ");
        print_enable_cursor(14, 15);
        cursor_y = 1;
        cursor_x = 7;

        char buffer[128];
        int buffer_index = 0;

        while (1) {
            char c = keyboard_get_char();

            if (c != 0) {
                if (c == '\b') {
                    if (buffer_index > 0) {
                        buffer_index--;
                        if (cursor_x > 7) {
                            cursor_x--;
                            print_set_cursor(cursor_x, cursor_y);
                            print_char(' ');
                            print_set_cursor(cursor_x, cursor_y);
                        }
                    }
                } else if (c == '\n') {
                    buffer[buffer_index] = '\0';
                    print_char(' ');
                    cursor_y++;

                    if (cursor_y >= SCREEN_HEIGHT) {
                        scroll_screen();
                    }

                    if (strncmp(buffer, "dt", 2) == 0) {
                        // Move cursor to a new line for the date/time
                        cursor_y++;
                        if (cursor_y >= SCREEN_HEIGHT) {
                            scroll_screen();
                        }
                        print_set_cursor(0, cursor_y);

                        // Display current date and time
                        struct tm time = get_rtc_time();
                        char time_str[32];
                        simple_snprintf(time_str, time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                                        time.tm_hour, time.tm_min, time.tm_sec);
                        print_str(time_str);

                        cursor_y++;
                        if (cursor_y >= SCREEN_HEIGHT) {
                            scroll_screen();
                        }

                        // Print the shell prompt again on the next line
                        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                        print_set_cursor(0, cursor_y);
                        print_str("Shell> ");
                        cursor_x = 7;
                        buffer_index = 0; // Reset buffer index after command
                    } else if (strncmp(buffer, "home", 4) == 0) {
                        kernel_main();  // Call kernel_main to execute the desired function
                        break;  // Exit the inner loop to refresh the shell
                    } else if (buffer_index >= 7 && strncmp(buffer, "create ", 7) == 0) {
                        const char *filename = &buffer[7];
                        display_loading_animation(filename);
                        int create_result = create_file(filename);

                        if (create_result == 0) {
                            print_line_with_color(0, cursor_y, "File created: ", PRINT_COLOR_GREEN, PRINT_COLOR_BLACK);
                            print_str(filename);
                        } else if (create_result == -1) {
                            print_line_with_color(0, cursor_y, "Error: ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                            print_str(filename);
                            print_str(" already exists");
                        } else {
                            print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                            print_str(filename);
                            print_str(" could not be created");
                        }
                    } else if (strncmp(buffer, "list_files", 10) == 0) {
                        list_files_command();
                    } else if (strncmp(buffer, "open ", 5) == 0) {
                        const char *filename = &buffer[5];
                        open_file_command(filename);
                    } else if (strncmp(buffer, "delete ", 7) == 0) {
                        const char *filename = &buffer[7];
                        delete_file_command(filename);
                    } else {
                        clear_line(cursor_y);
                        print_set_cursor(0, cursor_y);
                        print_line_with_color(0, cursor_y, "Unknown command: ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                        print_str(buffer);
                    }

                    cursor_y++;
                    if (cursor_y >= SCREEN_HEIGHT) {
                        scroll_screen();
                    }

                    // Print the shell prompt again
                    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                    print_set_cursor(0, cursor_y);
                    print_str("Shell> ");
                    cursor_x = 7;
                    buffer_index = 0; // Reset buffer index
                } else {
                    if (buffer_index < sizeof(buffer) - 1 && cursor_x < SCREEN_WIDTH - 1) {
                        buffer[buffer_index++] = c;
                        print_char(c);
                        cursor_x++;
                    }
                }
            }
        }
    }
}

void clear_line(int y) {
    if (y > 0) {  // Don't clear the first line
        print_set_cursor(0, y);
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            print_char(' ');
        }
    }
}

void print_line_with_color(int x, int y, const char *line, int fg, int bg) {
    if (y > 0) {  // Don't print on the first line
        print_set_color(fg, bg);
        print_set_cursor(x, y);
        print_str(line);
    }
}

void display_loading_animation(const char *filename) {
    // Implementation needed
}

void scroll_screen() {
    // Save the first line
    struct vga_char first_line[SCREEN_WIDTH];
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        first_line[x] = terminal_buffer[x];
    }

    // Move the screen buffer up by one row, starting from the second row
    for (int y = 2; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            terminal_buffer[(y - 1) * SCREEN_WIDTH + x] = terminal_buffer[y * SCREEN_WIDTH + x];
        }
    }
    
    // Clear the last row of the screen
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        terminal_buffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = (struct vga_char) {
            .character = ' ',
            .color = (PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4))
        };
    }

    // Restore the first line
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        terminal_buffer[x] = first_line[x];
    }
    
    // Adjust cursor_y after scrolling
    cursor_y = SCREEN_HEIGHT - 1;
}

void list_files_command() {
    char *files = list_files();  // Assuming list_files() returns a string with filenames or NULL
    if (files != NULL && strlen(files) > 0) {
        char *file = strtok(files, "\n");
        while (file != NULL) {
            print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
            print_set_cursor(0, cursor_y);

            if (strncmp(file, "File: ", 6) == 0) {
                file += 6;
            }

            print_str(file);
            cursor_y++;
            if (cursor_y >= SCREEN_HEIGHT) {
                scroll_screen();
            }
            file = strtok(NULL, "\n");
        }
        free(files);
    } else {
        print_line_with_color(0, cursor_y, "No files found!", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            scroll_screen();
        }
    }

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
    print_str("Shell> ");
    cursor_x = 7;
}


void open_file_command(const char *filename) {
    // Check if the file exists
    if (!file_exists(filename)) {
        // File does not exist, print error message
        print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(filename);
        print_str(" does not exist");

        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            scroll_screen();
        }

        // Show the shell prompt again
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_set_cursor(0, cursor_y);
        print_str("Shell> ");
        cursor_x = 7;
        return;
    }

    // Call the display_textfile function to open the file with the given filename
    display_textfile(filename);

    // Show the shell prompt again
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
    print_str("Shell> ");
    cursor_x = 7;
}


void delete_file_command(const char *file) {
    int delete_result = delete_file(file);

    if (delete_result == 0) {
        print_line_with_color(0, cursor_y, "File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" deleted!");
    } else if (delete_result == -1) {
        print_line_with_color(0, cursor_y, "Error: ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" could not be deleted");
    } else {
        print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" does not exist");
    }
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
    
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
    print_str("Shell> ");
    cursor_x = 7;
}

void set_first_line_color() {
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_set_cursor(0, 0);
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        print_char(' ');
    }
    print_set_cursor(0, 0);
    print_str("Kernel shell!");
}

void start_shell() {
    print_clear();
    run_shell();
}

void switch_to_shell() {
    // Clear the screen or reset display state
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        for (int j = 0; j < SCREEN_WIDTH; ++j) {
            print_set_cursor(j, i);
            print_char(' ');
        }
    }

    // Reset cursor position or display shell prompt
    print_set_cursor(0, 0);
    print_str("Shell>");
    
    // Call the shell initialization or main function if necessary
    run_shell();  // Start the shell
}
