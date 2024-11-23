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
            }
        }
    }
}

void handle_command_history(char key, char *buffer, int *buffer_index)
{
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

    while (cursor_x > strlen("Shell> "))
    {
        cursor_x--;
        print_set_cursor(cursor_x, cursor_y);
        print_char(' ');
    }

    cursor_x = strlen("Shell> ");
    print_set_cursor(cursor_x, cursor_y);

    if (history_index >= 0 && history_index < history_count)
    {
        strncpy(buffer, command_history[history_index % HISTORY_SIZE], MAX_INPUT);
        *buffer_index = strlen(buffer);
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
        print_set_cursor(0, y);
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            print_char(' ');
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
    struct vga_char first_line[SCREEN_WIDTH];
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        first_line[x] = terminal_buffer[x];
    }

    for (int y = 2; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            terminal_buffer[(y - 1) * SCREEN_WIDTH + x] = terminal_buffer[y * SCREEN_WIDTH + x];
        }
    }

    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        terminal_buffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = (struct vga_char){
            .character = ' ',
            .color = (PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4))};
    }

    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        terminal_buffer[x] = first_line[x];
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
    int file_index = fs_open(filename);  // Now returns an index to file_table

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
        return -1;  // Return an integer error code
    }

    // Access the file using file_table index
    FileEntry *file = &file_table[file_index];

    // Call display_textfile with the filename or the content as required
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
    return 0;  // Return an integer success code
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