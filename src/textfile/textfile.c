#include "textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/graphics/graphics.h"
#include "../filesystem/filesystem.h"
#include "../shell/shell.h"

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define MAX_INPUT 1000   // And this too
#define BUFFER_SIZE 1000   // This is the problem. Let's increase it to 1000.

void clear_and_reset_screen(void);
void textfile_scroll_screen(void);
void textfile_scroll_horizontal(int direction);
void display_save_message(const char *message);
void sync_cursor_position(int x, int y);
void force_text_mode(void);
void navigate_cursor(unsigned char key, char* input, int input_length, int* cursor_x, int* cursor_y, int* cursor_position);
void find_line_start_end(char* input, int input_length, int current_pos, int* line_start, int* line_end);
int calculate_cursor_position_from_coordinates(char* input, int input_length, int target_x, int target_y);
void get_cursor_coordinates_from_position(char* input, int position, int* x, int* y);

// Force the system into VGA text mode with full reset
void force_text_mode(void) {
    // Disable graphics mode if active
    graphics_info_t* gfx = graphics_get_info();
    if (gfx && gfx->initialized) {
        gfx->initialized = false;
    }
    
    // Reset VGA text mode completely
    print_clear();
    print_disable_cursor();
    print_enable_cursor(14, 15);
    
    // Reset the print system's internal cursor state
    print_set_cursor(0, 0);
    print_update_cursor();
}

// Navigation helper functions
void find_line_start_end(char* input, int input_length, int current_pos, int* line_start, int* line_end) {
    *line_start = current_pos;
    *line_end = current_pos;
    
    // Find start of current line
    while (*line_start > 0 && input[*line_start - 1] != '\n') {
        (*line_start)--;
    }
    
    // Find end of current line
    while (*line_end < input_length && input[*line_end] != '\n') {
        (*line_end)++;
    }
}

int calculate_cursor_position_from_coordinates(char* input, int input_length, int target_x, int target_y) {
    int current_x = 0;
    int current_y = 2; // Start from line 2 (content area)
    int position = 0;
    
    while (position < input_length && current_y < target_y) {
        if (input[position] == '\n') {
            current_y++;
            current_x = 0;
        } else {
            current_x++;
            if (current_x >= SCREEN_WIDTH) {
                current_x = 0;
                current_y++;
            }
        }
        position++;
    }
    
    // Now we're on the target line, move to target_x
    while (position < input_length && current_x < target_x && input[position] != '\n') {
        current_x++;
        position++;
        if (current_x >= SCREEN_WIDTH) {
            break;
        }
    }
    
    return position;
}

void get_cursor_coordinates_from_position(char* input, int position, int* x, int* y) {
    int current_x = 0;
    int current_y = 2; // Start from line 2 (content area)
    
    for (int i = 0; i < position; i++) {
        if (input[i] == '\n') {
            current_y++;
            current_x = 0;
        } else {
            current_x++;
            if (current_x >= SCREEN_WIDTH) {
                current_x = 0;
                current_y++;
            }
        }
    }
    
    *x = current_x;
    *y = current_y;
}

void navigate_cursor(unsigned char key, char* input, int input_length, int* cursor_x, int* cursor_y, int* cursor_position) {
    int new_position = *cursor_position;
    int line_start, line_end;
    
    switch (key) {
        case NAV_UP_ARROW:
            if (*cursor_y > 2) {
                int target_y = *cursor_y - 1;
                new_position = calculate_cursor_position_from_coordinates(input, input_length, *cursor_x, target_y);
                get_cursor_coordinates_from_position(input, new_position, cursor_x, cursor_y);
                *cursor_position = new_position;
            }
            break;
            
        case NAV_DOWN_ARROW:
            if (*cursor_y < SCREEN_HEIGHT - 1) {
                int target_y = *cursor_y + 1;
                new_position = calculate_cursor_position_from_coordinates(input, input_length, *cursor_x, target_y);
                if (new_position <= input_length) {
                    get_cursor_coordinates_from_position(input, new_position, cursor_x, cursor_y);
                    *cursor_position = new_position;
                }
            }
            break;
            
        case NAV_LEFT_ARROW:
            if (*cursor_position > 0) {
                (*cursor_position)--;
                if (*cursor_x > 0) {
                    (*cursor_x)--;
                } else if (*cursor_y > 2) {
                    (*cursor_y)--;
                    *cursor_x = SCREEN_WIDTH - 1;
                    // Find actual end of previous line
                    int temp_pos = *cursor_position;
                    while (temp_pos > 0 && input[temp_pos - 1] != '\n') {
                        temp_pos--;
                    }
                    int line_length = *cursor_position - temp_pos + 1;
                    *cursor_x = (line_length - 1) % SCREEN_WIDTH;
                }
            }
            break;
            
        case NAV_RIGHT_ARROW:
            if (*cursor_position < input_length) {
                (*cursor_position)++;
                (*cursor_x)++;
                if (*cursor_x >= SCREEN_WIDTH || 
                    (*cursor_position > 0 && input[*cursor_position - 1] == '\n')) {
                    *cursor_x = 0;
                    (*cursor_y)++;
                    if (*cursor_y >= SCREEN_HEIGHT) {
                        *cursor_y = SCREEN_HEIGHT - 1;
                    }
                }
            }
            break;
            
        case NAV_HOME_KEY:
            find_line_start_end(input, input_length, *cursor_position, &line_start, &line_end);
            *cursor_position = line_start;
            get_cursor_coordinates_from_position(input, *cursor_position, cursor_x, cursor_y);
            break;
            
        case NAV_END_KEY:
            find_line_start_end(input, input_length, *cursor_position, &line_start, &line_end);
            *cursor_position = line_end;
            get_cursor_coordinates_from_position(input, *cursor_position, cursor_x, cursor_y);
            break;
            
        case NAV_PAGE_UP:
            {
                int lines_to_move = SCREEN_HEIGHT - 3; // Leave room for header/footer
                int target_y = (*cursor_y - lines_to_move < 2) ? 2 : *cursor_y - lines_to_move;
                new_position = calculate_cursor_position_from_coordinates(input, input_length, *cursor_x, target_y);
                get_cursor_coordinates_from_position(input, new_position, cursor_x, cursor_y);
                *cursor_position = new_position;
            }
            break;
            
        case NAV_PAGE_DOWN:
            {
                int lines_to_move = SCREEN_HEIGHT - 3; // Leave room for header/footer
                int target_y = *cursor_y + lines_to_move;
                if (target_y >= SCREEN_HEIGHT) target_y = SCREEN_HEIGHT - 1;
                new_position = calculate_cursor_position_from_coordinates(input, input_length, *cursor_x, target_y);
                if (new_position <= input_length) {
                    get_cursor_coordinates_from_position(input, new_position, cursor_x, cursor_y);
                    *cursor_position = new_position;
                }
            }
            break;
            
        case CTRL_HOME:
            *cursor_position = 0;
            *cursor_x = 0;
            *cursor_y = 2;
            break;
            
        case CTRL_END:
            *cursor_position = input_length;
            get_cursor_coordinates_from_position(input, *cursor_position, cursor_x, cursor_y);
            break;
    }
    
    // Ensure cursor stays within bounds
    if (*cursor_x < 0) *cursor_x = 0;
    if (*cursor_x >= SCREEN_WIDTH) *cursor_x = SCREEN_WIDTH - 1;
    if (*cursor_y < 2) *cursor_y = 2;
    if (*cursor_y >= SCREEN_HEIGHT) *cursor_y = SCREEN_HEIGHT - 1;
    if (*cursor_position < 0) *cursor_position = 0;
    if (*cursor_position > input_length) *cursor_position = input_length;
}

// Ensure cursor position is properly synchronized with enhanced protection
void sync_cursor_position(int x, int y) {
    // Clamp coordinates to valid range
    if (x < 0) x = 0;
    if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
    
    // Force cursor position and update hardware immediately
    print_set_cursor(x, y);
    print_update_cursor();
    
    // Double-check: ensure the position stuck
    int check_x, check_y;
    print_get_cursor(&check_x, &check_y);
    if (check_x != x || check_y != y) {
        // Position didn't stick, try again
        print_set_cursor(x, y);
        print_update_cursor();
    }
}

void clear_screen(void) {
    // Much faster screen clearing with direct VGA memory access
    unsigned char* video_memory = (unsigned char*)0xB8000;
    unsigned char color = PRINT_COLOR_BLACK | (PRINT_COLOR_WHITE << 4);
    
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = color;
    }
    print_set_cursor(0, 0); 
}

void textfile_scroll_screen(void) {
    // Much faster scrolling with direct memory operations
    unsigned char* video_memory = (unsigned char*)0xB8000;
    unsigned char color = PRINT_COLOR_BLACK | (PRINT_COLOR_WHITE << 4);
    
    // Move all lines up by one
    for (int y = 1; y < SCREEN_HEIGHT; y++) {
        for (int i = 0; i < SCREEN_WIDTH * 2; i++) {
            video_memory[(y - 1) * SCREEN_WIDTH * 2 + i] = video_memory[y * SCREEN_WIDTH * 2 + i];
        }
    }
    
    // Clear the last line
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        video_memory[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x) * 2] = ' ';
        video_memory[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x) * 2 + 1] = color;
    }
}

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

void update_filename(const char *filename) {
    // Save current cursor position
    int saved_x, saved_y;
    print_get_cursor(&saved_x, &saved_y);
    
    // Update filename display
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); 
    print_set_cursor(0, 0);
    print_str(filename);
    
    // Restore cursor position
    print_set_cursor(saved_x, saved_y);
    print_update_cursor();
}

void update_char_count(int count) {
    static int last_count = -1; 

    if (count == last_count) {
        return; 
    }

    last_count = count;
    char count_str[20];
    
    // Save current cursor position
    int saved_x, saved_y;
    print_get_cursor(&saved_x, &saved_y);
    
    // Update character count display
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); 
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

    for (int j = 0; j < i / 2; j++) {
        char tmp = count_str[j];
        count_str[j] = count_str[i - j - 1];
        count_str[i - j - 1] = tmp;
    }

    print_str(count_str);
    for (int k = 0; k < 5; k++) {
        print_char(' '); 
    }
    
    // Restore cursor position
    print_set_cursor(saved_x, saved_y);
    print_update_cursor();
}

void clear_and_reset_screen(void) {
    clear_screen();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, 0);
    print_str("Kernel v1");
}

void delay(int milliseconds) {
    // More efficient delay - reduced cycles
    volatile int cycles = milliseconds * 100;
    
    while (cycles > 0) {
        cycles--;
    }
}

void display_save_message(const char *message) {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE); 
    int popup_start_x = (SCREEN_WIDTH - 30) / 2; 
    int popup_start_y = (SCREEN_HEIGHT - 3) / 2; 
    
    for (int y = 0; y < 3; ++y) {
        print_set_cursor(popup_start_x, popup_start_y + y);
        for (int i = 0; i < 30; ++i) {
            print_char(' ');
        }
    }

    print_set_cursor(popup_start_x + 1, popup_start_y + 1); 
    print_str(message);

    delay(80000);
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE); 
    for (int y = popup_start_y; y <= popup_start_y + 2; ++y) {
        print_set_cursor(popup_start_x, y);
        for (int x = 0; x < 30; ++x) {
            print_char(' '); 
        }
    }
}

void save_current_file(const char *filename, const uint8_t *content, int length) {
    int save_result = save_file(filename, content, length);
    if (save_result < 0) {
        display_save_message("Error saving file.");
    } else {
        display_save_message("File saved successfully.");
    }
}

void update_text_content(const uint8_t *buffer, int length, int *final_x, int *final_y) {
    int cursor_x = 0;
    int cursor_y = 2; 

    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    sync_cursor_position(0, cursor_y);
    
    for (int i = 0; i < length; ++i) {
        if (buffer[i] == '\n') {
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= SCREEN_HEIGHT) {
                textfile_scroll_screen();
                cursor_y = SCREEN_HEIGHT - 1;
            }
        } else {
            sync_cursor_position(cursor_x, cursor_y);
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
   
    *final_x = cursor_x;
    *final_y = cursor_y;
}

void display_textfile(const char *filename) {
    char input[MAX_INPUT] = {0};    
    int input_length = 0;
    int cursor_x = 0;
    int cursor_y = 2;
    int cursor_position = 0; // Track position in input buffer
    unsigned char key;
    uint8_t file_buffer[BUFFER_SIZE];

    // Ensure we're in VGA text mode for the text editor
    force_text_mode();
    
    clear_screen();
    update_filename(filename);

    int file_index = fs_open(filename);
    if (file_index == -1) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
        print_set_cursor(0, 0);
        print_str("Error: Unable to open file.");
        while (1); 
    }

    int read_result = fs_read(file_index, file_buffer, BUFFER_SIZE);  
    if (read_result > 0) {
        update_text_content(file_buffer, read_result, &cursor_x, &cursor_y);  
        for (int i = 0; i < read_result; i++) {
            if (input_length < MAX_INPUT - 1) { 
                input[input_length++] = file_buffer[i];
            } else {
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
                print_set_cursor(0, SCREEN_HEIGHT - 1);
                print_str("Error: Input buffer overflow");
                while (1);
            }
        }
        cursor_position = input_length; // Set cursor to end of loaded content
    }

    print_set_cursor(0, 1);
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        print_char('-');
    }
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    update_char_count(input_length);
    
    // Ensure cursor is at the correct position after file content
    sync_cursor_position(cursor_x, cursor_y);

    while (1) {
        key = keyboard_get_char();

        if (key == 0x1B) {  
            fs_close(file_index);  
            
            // Ensure we're back in proper text mode
            force_text_mode();
            
            clear_and_reset_screen(); 
            run_shell();  
            return;  
        } else if (key == '\n') {
            // Insert newline at cursor position
            if (input_length < MAX_INPUT - 1) {
                // Shift characters to the right
                for (int i = input_length; i > cursor_position; i--) {
                    input[i] = input[i - 1];
                }
                input[cursor_position] = key;
                input_length++;
                cursor_position++;
                
                // Update display - refresh from cursor position onwards
                clear_screen();
                update_filename(filename);
                print_set_cursor(0, 1);
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
                for (int i = 0; i < SCREEN_WIDTH; ++i) {
                    print_char('-');
                }
                print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
                
                int display_x, display_y;
                update_text_content((const uint8_t *)input, input_length, &display_x, &display_y);
                get_cursor_coordinates_from_position(input, cursor_position, &cursor_x, &cursor_y);
                sync_cursor_position(cursor_x, cursor_y);
            }
        } else if (key == '\b' && cursor_position > 0) {
            // Delete character before cursor
            cursor_position--;
            for (int i = cursor_position; i < input_length - 1; i++) {
                input[i] = input[i + 1];
            }
            input_length--;
            input[input_length] = '\0';
            
            // Update display - refresh from cursor position onwards
            clear_screen();
            update_filename(filename);
            print_set_cursor(0, 1);
            print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
            for (int i = 0; i < SCREEN_WIDTH; ++i) {
                print_char('-');
            }
            print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
            
            int display_x, display_y;
            update_text_content((const uint8_t *)input, input_length, &display_x, &display_y);
            get_cursor_coordinates_from_position(input, cursor_position, &cursor_x, &cursor_y);
            sync_cursor_position(cursor_x, cursor_y);
        } else if (key == 0x13) {  
            save_current_file(filename, (const uint8_t *)input, input_length);
        } else if (key == NAV_UP_ARROW || key == NAV_DOWN_ARROW || key == NAV_LEFT_ARROW || key == NAV_RIGHT_ARROW ||
                   key == NAV_PAGE_UP || key == NAV_PAGE_DOWN || key == NAV_HOME_KEY || key == NAV_END_KEY ||
                   key == CTRL_HOME || key == CTRL_END) {
            // Handle navigation keys BEFORE character input to avoid conflicts
            navigate_cursor(key, input, input_length, &cursor_x, &cursor_y, &cursor_position);
            sync_cursor_position(cursor_x, cursor_y);
        } else if ((key >= 32 && key <= 126)) {  
            // Insert character at cursor position (strict printable ASCII only)
            // Check this AFTER navigation keys to prevent conflicts with navigation scancodes
            if (input_length < MAX_INPUT - 1) {
                // Shift characters to the right
                for (int i = input_length; i > cursor_position; i--) {
                    input[i] = input[i - 1];
                }
                input[cursor_position] = key;
                input_length++;
                cursor_position++;
                
                // For simple character insertion, we can optimize by just redrawing affected area
                // But for simplicity, let's refresh the whole content area
                clear_screen();
                update_filename(filename);
                print_set_cursor(0, 1);
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_WHITE);
                for (int i = 0; i < SCREEN_WIDTH; ++i) {
                    print_char('-');
                }
                print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
                
                int display_x, display_y;
                update_text_content((const uint8_t *)input, input_length, &display_x, &display_y);
                get_cursor_coordinates_from_position(input, cursor_position, &cursor_x, &cursor_y);
                sync_cursor_position(cursor_x, cursor_y);
            }
        } else if (key == 0) {
            // Add small delay when no input
            for (volatile int i = 0; i < 300; i++) {}
        }

        // Only update cursor and char count when we have input
        if (key != 0) {
            // Update character count first (which saves/restores cursor)
            update_char_count(input_length);
            
            // Then ensure our cursor is in the right place
            sync_cursor_position(cursor_x, cursor_y);
        }
    }
}
