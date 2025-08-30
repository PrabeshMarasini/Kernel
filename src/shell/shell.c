#include "../textfile/textfile.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/graphics/graphics.h"
#include "../drivers/graphics/gfx_print.h"
#include "../filesystem/filesystem.h"
#include <string.h>
#include <stdlib.h>
#include "shell.h"
#include "../datetime/datetime.h"

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define MAX_INPUT 1000
#define HISTORY_SIZE 100

static char command_history[HISTORY_SIZE][MAX_INPUT] = {0};
static int history_count = 0;
static int history_index = -1;

struct vga_char
{
    char character;
    char color;
};

volatile struct vga_char *terminal_buffer = (volatile struct vga_char *)0xB8000;
static int cursor_x = 7;
static int cursor_y = 1;

void kernel_main();
void handle_special_keys(char key);
void run_shell();
void clear_line(int y);
void print_line_with_color(int x, int y, const char *line, int fg, int bg);
void display_loading_animation(const char *filename);
void scroll_screen();
void list_files_command();
int open_file_command(const char *filename);
void delete_file_command(const char *filename);
void set_first_line_color();
void start_shell();
void create_file_command(const char *filename);
void dt_command(void);
void handle_command_history(char key, char *buffer, int *buffer_index);
void print_int(int num);
void itoa(int num, char *str, int base);


void handle_special_keys(char key)
{
    static char input[MAX_INPUT] = {0};
    static int input_length = 0;

    if (key == '\n')
    {
        cursor_y++;
        cursor_x = 0;
        if (cursor_y >= SCREEN_HEIGHT)
        {
            scroll_screen();
        }
    }
    else if (key == '\b' && input_length > 0)
    {
        input[--input_length] = '\0';
        if (cursor_x > 0)
        {
            cursor_x--;
        }
        else if (cursor_y > 1)
        {
            cursor_y--;
            cursor_x = SCREEN_WIDTH - 1;
        }
        print_set_cursor(cursor_x, cursor_y);
        print_char(' ');
        print_set_cursor(cursor_x, cursor_y);
    }
    else if (key >= 32 && key <= 126 && input_length < MAX_INPUT - 1)
    {
        input[input_length++] = key;
        print_set_cursor(cursor_x, cursor_y);
        print_char(key);
        if (++cursor_x >= SCREEN_WIDTH)
        {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= SCREEN_HEIGHT)
            {
                scroll_screen();
            }
        }
    }
}

void display_time()
{
    struct tm time = get_rtc_time();
    adjust_time_for_nepal(&time);
    print_set_color(PRINT_COLOR_CYAN, PRINT_COLOR_BLACK);
    print_set_cursor(0, 1);
    print_time(&time);
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
}

void clear_screen_but_keep_first_line()
{
    for (int y = 1; y < SCREEN_HEIGHT; y++)
    {
        clear_line(y);
    }

    cursor_x = 0;
    cursor_y = 1;
    print_set_cursor(cursor_x, cursor_y);
}

void run_shell()
{
    const char *prompt = "Shell> ";

    while (1)
    {
        print_clear();
        set_first_line_color();
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_set_cursor(0, 1);
        print_str(prompt);
        print_enable_cursor(14, 15);
        cursor_y = 1;
        cursor_x = strlen(prompt);

        char buffer[MAX_INPUT] = {0};
        int buffer_index = 0;

        while (1)
        {
            char c = keyboard_get_char();

            if (c != 0)
            {
                if (c == '\b')
                {
                    if (buffer_index > 0)
                    {
                        buffer_index--;
                        if (cursor_x > strlen(prompt))
                        {
                            cursor_x--;
                            print_set_cursor(cursor_x, cursor_y);
                            print_char(' ');
                            print_set_cursor(cursor_x, cursor_y);
                        }
                    }
                }
                else if (c == '\n')
                {
                    buffer[buffer_index] = '\0';
                    cursor_y++;
                    if (cursor_y >= SCREEN_HEIGHT)
                    {
                        scroll_screen();
                    }

                    if (buffer_index > 0)
                    {
                        strncpy(command_history[history_count % HISTORY_SIZE], buffer, MAX_INPUT);
                        history_count++;
                        history_index = history_count;
                    }

                    if (strncmp(buffer, "dt", 2) == 0)
                    {
                        dt_command();
                    }
                    else if (strncmp(buffer, "home", 4) == 0)
                    {
                        kernel_main();
                        break;
                    }
                    else if (strncmp(buffer, "create ", 7) == 0)
                    {
                        create_file_command(&buffer[7]);
                    }
                    else if (strncmp(buffer, "ls", 10) == 0)
                    {
                        list_files_command();
                    }
                    else if (strncmp(buffer, "open ", 5) == 0)
                    {
                        open_file_command(&buffer[5]);
                    }
                    else if (strncmp(buffer, "delete ", 7) == 0)
                    {
                        delete_file_command(&buffer[7]);
                    }
                    else if (strncmp(buffer, "clr", 3) == 0)
                    {
                        clear_screen_but_keep_first_line();
                    }
                    else if (strncmp(buffer, "font-demo", 9) == 0)
                    {
                        font_demo_command();
                    }
                    else if (strncmp(buffer, "font-small", 10) == 0)
                    {
                        font_size_command(FONT_SIZE_SMALL);
                    }
                    else if (strncmp(buffer, "font-medium", 11) == 0)
                    {
                        font_size_command(FONT_SIZE_MEDIUM);
                    }
                    else if (strncmp(buffer, "font-large", 10) == 0)
                    {
                        font_size_command(FONT_SIZE_LARGE);
                    }
                    else if (strncmp(buffer, "font-xlarge", 11) == 0)
                    {
                        font_size_command(FONT_SIZE_XLARGE);
                    }
                    else if (strncmp(buffer, "font-bold", 9) == 0)
                    {
                        font_weight_command(FONT_WEIGHT_BOLD);
                    }
                    else if (strncmp(buffer, "font-normal", 11) == 0)
                    {
                        font_weight_command(FONT_WEIGHT_NORMAL);
                    }
                    else if (strncmp(buffer, "font-aa-on", 10) == 0)
                    {
                        font_antialiasing_command(1);
                    }
                    else if (strncmp(buffer, "font-aa-off", 11) == 0)
                    {
                        font_antialiasing_command(0);
                    }
                    else if (strncmp(buffer, "font-reset", 10) == 0)
                    {
                        font_reset_command();
                    }
                    else if (strncmp(buffer, "help", 4) == 0)
                    {
                        help_command();
                    }
                    else if (buffer_index > 0)
                    {
                        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                        print_set_cursor(0, cursor_y);
                        print_str("Unknown command: ");
                        print_str(buffer);
                        cursor_y++;
                        if (cursor_y >= SCREEN_HEIGHT)
                        {
                            scroll_screen();
                        }
                    }

                    buffer_index = 0;
                    cursor_x = 0;
                    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                    print_set_cursor(cursor_x, cursor_y);
                    print_str(prompt);
                    cursor_x = strlen(prompt);
                }
                else if (c == UP_ARROW || c == DOWN_ARROW)
                {
                    handle_command_history(c, buffer, &buffer_index);
                }
                else
                {
                    if (buffer_index < MAX_INPUT - 1 && cursor_x < SCREEN_WIDTH - 1)
                    {
                        buffer[buffer_index++] = c;
                        print_char(c);
                        cursor_x++;
                    }
                }
            } else {
                // Add small delay when no input to reduce CPU usage
                for (volatile int i = 0; i < 500; i++) {}
            }
        }
    }
}

void handle_command_history(char key, char *buffer, int *buffer_index)
{
    static const int prompt_len = 7; // "Shell> " length
    
    if (key == UP_ARROW)
    {
        if (history_index > 0)
        {
            history_index--;
        }
    }
    else if (key == DOWN_ARROW)
    {
        if (history_index < history_count)
        {
            history_index++;
        }
    }

    // Clear current input more efficiently
    unsigned char* video_memory = (unsigned char*)0xB8000;
    unsigned char color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);
    
    for (int x = prompt_len; x < SCREEN_WIDTH; x++)
    {
        video_memory[(cursor_y * SCREEN_WIDTH + x) * 2] = ' ';
        video_memory[(cursor_y * SCREEN_WIDTH + x) * 2 + 1] = color;
    }

    cursor_x = prompt_len;
    print_set_cursor(cursor_x, cursor_y);

    if (history_index >= 0 && history_index < history_count)
    {
        // Fast string copy without strlen calls
        int i = 0;
        char* src = command_history[history_index % HISTORY_SIZE];
        while (src[i] != '\0' && i < MAX_INPUT - 1)
        {
            buffer[i] = src[i];
            i++;
        }
        buffer[i] = '\0';
        *buffer_index = i;
        
        print_str(buffer);
        cursor_x += *buffer_index;
    }
    else
    {
        *buffer_index = 0;
        buffer[0] = '\0';
    }

    print_set_cursor(cursor_x, cursor_y);
}

void clear_line(int y)
{
    if (y > 0)
    {
        // Direct VGA memory access for faster clearing
        unsigned char* video_memory = (unsigned char*)0xB8000;
        unsigned char color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);
        
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            video_memory[(y * SCREEN_WIDTH + i) * 2] = ' ';
            video_memory[(y * SCREEN_WIDTH + i) * 2 + 1] = color;
        }
    }
}

void print_line_with_color(int x, int y, const char *line, int fg, int bg)
{
    if (y > 0)
    {
        print_set_color(fg, bg);
        print_set_cursor(x, y);
        print_str(line);
    }
}

void scroll_screen()
{
    // Much faster scrolling using memory copy
    unsigned char* video_memory = (unsigned char*)0xB8000;
    unsigned char color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);
    
    // Save first line
    unsigned char first_line[SCREEN_WIDTH * 2];
    for (int i = 0; i < SCREEN_WIDTH * 2; i++)
    {
        first_line[i] = video_memory[i];
    }
    
    // Scroll up (copy line by line for better performance)
    for (int y = 2; y < SCREEN_HEIGHT; y++)
    {
        for (int i = 0; i < SCREEN_WIDTH * 2; i++)
        {
            video_memory[(y - 1) * SCREEN_WIDTH * 2 + i] = video_memory[y * SCREEN_WIDTH * 2 + i];
        }
    }
    
    // Clear last line
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        video_memory[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i) * 2] = ' ';
        video_memory[((SCREEN_HEIGHT - 1) * SCREEN_WIDTH + i) * 2 + 1] = color;
    }
    
    // Restore first line
    for (int i = 0; i < SCREEN_WIDTH * 2; i++)
    {
        video_memory[i] = first_line[i];
    }
    
    cursor_y = SCREEN_HEIGHT - 1;
}

void dt_command()
{
    struct tm time = get_rtc_time();
    adjust_time_for_nepal(&time);
    char time_str[32];
    simple_snprintf(time_str, time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec);
    print_set_color(PRINT_COLOR_LIGHT_CYAN, PRINT_COLOR_BLACK);
    print_set_cursor(0, cursor_y);
    print_str(time_str);
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT)
    {
        scroll_screen();
    }

    cursor_x = 0;
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(cursor_x, cursor_y);
    print_str("Shell> ");
    cursor_x = strlen("Shell> ");
}

void create_file_command(const char *filename)
{
    const uint8_t *content = NULL;
    uint32_t size = 0;
    int create_result = create_file(filename, content, size);

    if (create_result == 0)
    {
        print_line_with_color(0, cursor_y, "File created: ", PRINT_COLOR_GREEN, PRINT_COLOR_BLACK);
        print_str(filename);
    }
    else if (create_result == -1)
    {
        print_line_with_color(0, cursor_y, "Error: ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(filename);
        print_str(" already exists");
    }
    else
    {
        print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(filename);
        print_str(" could not be created");
    }

    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT)
    {
        scroll_screen();
    }

    cursor_x = 0;
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(cursor_x, cursor_y);
    print_str("Shell> ");
    cursor_x = strlen("Shell> ");
}

// int open_file_command(const char *filename) {
//     int file_index = fs_open(filename);  // This should return an index to file_table
//     if (file_index == -1) {
//         print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
//         print_str(filename);
//         print_str(" does not exist");

//         cursor_y++;
//         if (cursor_y >= SCREEN_HEIGHT) {
//             scroll_screen();
//         }

//         cursor_x = 0;
//         print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
//         print_set_cursor(cursor_x, cursor_y);
//         print_str("Shell> ");
//         cursor_x = strlen("Shell> ");
//         return -1;
//     }

//     file_entry_t *file = &file_table[file_index];
//     if (file->size == 0) {
//         print_line_with_color(0, cursor_y, "The file is empty!", PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);

//         cursor_y++;
//         if (cursor_y >= SCREEN_HEIGHT) {
//             scroll_screen();
//         }

//         cursor_x = 0;
//         print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
//         print_set_cursor(cursor_x, cursor_y);
//         print_str("Shell> ");
//         cursor_x = strlen("Shell> ");
//         return -1;
//     }

//     print_line_with_color(0, cursor_y, "File size: ", PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
//     print_int(file->size);
//     cursor_y++;
//     if (cursor_y >= SCREEN_HEIGHT) {
//         scroll_screen();
//     }

//     char read_buffer[128];
//     size_t total_read = 0;

//     print_line_with_color(0, cursor_y, "Content: ", PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
//     cursor_y++;
//     if (cursor_y >= SCREEN_HEIGHT) {
//         scroll_screen();
//     }

//     while (total_read < file->size) {
//         size_t bytes_to_read = sizeof(read_buffer);
//         if (file->size - total_read < bytes_to_read) {
//             bytes_to_read = file->size - total_read;
//         }

//         size_t bytes_read = fs_read(file_index, read_buffer, bytes_to_read);  // fs_read should take the index
//         if (bytes_read == 0) {
//             break;
//         }

//         read_buffer[bytes_read] = '\0';
//         print_str(read_buffer);        
//         total_read += bytes_read;
//     }

//     cursor_y++;
//     if (cursor_y >= SCREEN_HEIGHT) {
//         scroll_screen();
//     }

//     cursor_x = 0;
//     print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
//     print_set_cursor(cursor_x, cursor_y);
//     print_str("Shell> ");
//     cursor_x = strlen("Shell> ");
//     return 0;
// }



int open_file_command(const char *filename) {
    int file_index = fs_open(filename);  

    if (file_index == -1) {
        print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(filename);
        print_str(" does not exist");

        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            scroll_screen();
        }

        cursor_x = 0;
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        print_set_cursor(cursor_x, cursor_y);
        print_str("Shell> ");
        cursor_x = strlen("Shell> ");
        return -1;  
    }

    FileEntry *file = &file_table[file_index];
    display_textfile(filename);

    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }

    cursor_x = 0;
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(cursor_x, cursor_y);
    print_str("Shell> ");
    cursor_x = strlen("Shell> ");
    return 0;  
}



void print_int(int num) {
    char buffer[16];
    itoa(num, buffer, 10);
    print_str(buffer);
}

void itoa(int num, char *str, int base) {
    int i = 0;
    int isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void delete_file_command(const char *file)
{
    int delete_result = delete_file(file);

    if (delete_result == 0)
    {
        print_line_with_color(0, cursor_y, "File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" deleted!");
    }
    else if (delete_result == -1)
    {
        print_line_with_color(0, cursor_y, "Error: ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" could not be deleted");
    }
    else
    {
        print_line_with_color(0, cursor_y, "Error: File ", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        print_str(file);
        print_str(" does not exist");
    }

    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT)
    {
        scroll_screen();
    }

    cursor_x = 0;
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(cursor_x, cursor_y);
    print_str("Shell> ");
    cursor_x = strlen("Shell> ");
}

void list_files_command()
{
    char *files = list_files();
    if (files != NULL && strlen(files) > 0)
    {
        char *file = strtok(files, "\n");
        while (file != NULL)
        {
            print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
            print_set_cursor(0, cursor_y);

            if (strncmp(file, "File: ", 6) == 0)
            {
                file += 6;
            }

            print_str(file);
            cursor_y++;
            if (cursor_y >= SCREEN_HEIGHT)
            {
                scroll_screen();
            }
            file = strtok(NULL, "\n");
        }
        free(files);
    }
    else
    {
        print_line_with_color(0, cursor_y, "No files found!", PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT)
        {
            scroll_screen();
        }
    }

    cursor_x = 0;
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(cursor_x, cursor_y);
    print_str("Shell> ");
    cursor_x = strlen("Shell> ");
}

void set_first_line_color()
{
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_set_cursor(0, 0);
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        print_char(' ');
    }
    print_set_cursor(0, 0);
    print_str("Kernel shell!");
}

void start_shell()
{
    print_clear();
    run_shell();
}

void switch_to_shell()
{
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    for (int i = 0; i < SCREEN_HEIGHT; ++i)
    {
        for (int j = 0; j < SCREEN_WIDTH; ++j)
        {
            print_set_cursor(j, i);
            print_char(' ');
        }
    }

    print_set_cursor(0, 0);
    print_str("Shell>");
    run_shell();
}

// Font command implementations
void font_demo_command() {
    graphics_info_t* gfx = graphics_get_info();
    
    if (gfx && gfx->initialized) {
        // Switch to graphics mode and show font demo
        graphics_clear(COLOR_BLACK);
        
        font_style_t demo_style = {
            .size = FONT_SIZE_LARGE,
            .weight = FONT_WEIGHT_NORMAL,
            .anti_aliasing = true
        };
        
        graphics_draw_string_aa(50, 100, "Font Demo - SecureOS", COLOR_WHITE, COLOR_BLACK, &demo_style);
        graphics_draw_string_aa(50, 150, "Graphics Mode Active!", COLOR_GREEN, COLOR_BLACK, &demo_style);
        
        // Wait for user input
        for (volatile int i = 0; i < 100000000; i++) {}
        
        // Return to text mode
        print_clear();
        cursor_y++;
        print_set_cursor(0, cursor_y);
        print_str("Font demo completed.");
    } else {
        cursor_y++;
        print_set_cursor(0, cursor_y);
        print_str("Graphics mode not available. Font demo requires graphics.");
    }
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}

void font_size_command(font_size_t size) {
    const char* size_name;
    
    switch (size) {
        case FONT_SIZE_SMALL: size_name = "Small (8px)"; break;
        case FONT_SIZE_MEDIUM: size_name = "Medium (12px)"; break;
        case FONT_SIZE_LARGE: size_name = "Large (16px)"; break;
        case FONT_SIZE_XLARGE: size_name = "X-Large (20px)"; break;
        default: size_name = "Unknown"; break;
    }
    
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("Font size set to: ");
    print_str(size_name);
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}

void font_weight_command(font_weight_t weight) {
    const char* weight_name = (weight == FONT_WEIGHT_BOLD) ? "Bold" : "Normal";
    
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("Font weight set to: ");
    print_str(weight_name);
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}

void font_antialiasing_command(int enabled) {
    const char* status = enabled ? "Enabled" : "Disabled";
    
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("Font anti-aliasing: ");
    print_str(status);
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}

void font_reset_command() {
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("Font settings reset to defaults:");
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("  Size: Small (8px)");
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("  Weight: Normal");
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("  Anti-aliasing: Disabled");
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}

void help_command() {
    cursor_y++;
    print_set_cursor(0, cursor_y);
    print_str("Available commands:");
    cursor_y++;
    
    const char* commands[] = {
        "  dt           - Show date and time",
        "  home         - Return to main interface",
        "  create <file> - Create a new file",
        "  ls           - List files",
        "  open <file>  - Open a file",
        "  delete <file> - Delete a file",
        "  clr          - Clear screen",
        "  font-demo    - Show font demonstration",
        "  font-small   - Set small font size",
        "  font-medium  - Set medium font size",
        "  font-large   - Set large font size",
        "  font-xlarge  - Set extra large font size",
        "  font-bold    - Set bold font weight",
        "  font-normal  - Set normal font weight",
        "  font-aa-on   - Enable anti-aliasing",
        "  font-aa-off  - Disable anti-aliasing",
        "  font-reset   - Reset font to defaults",
        "  help         - Show this help"
    };
    
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (int i = 0; i < num_commands; i++) {
        print_set_cursor(0, cursor_y);
        print_str(commands[i]);
        cursor_y++;
        if (cursor_y >= SCREEN_HEIGHT) {
            scroll_screen();
        }
    }
    
    cursor_y++;
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll_screen();
    }
}