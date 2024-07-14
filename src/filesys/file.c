#include <string.h>
#include <stdio.h>

#define MAX_FILENAME_LENGTH 32
#define MAX_FILE_SIZE 4096

typedef struct {
    char name[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_SIZE];
    int size;
} File;

File current_file;
char text_buffer[MAX_FILE_SIZE];
int cursor_position = 0;

void create_file(const char* filename) {
    strncpy(current_file.name, filename, MAX_FILENAME_LENGTH);
    current_file.size = 0;
    memset(current_file.content, 0, MAX_FILE_SIZE);
}

void save_file() {
    memcpy(current_file.content, text_buffer, MAX_FILE_SIZE);
    current_file.size = strlen(text_buffer);
    // Here you would typically write to persistent storage
}

void insert_char(char c) {
    if (cursor_position < MAX_FILE_SIZE - 1) {
        memmove(&text_buffer[cursor_position + 1], &text_buffer[cursor_position], strlen(text_buffer) - cursor_position);
        text_buffer[cursor_position] = c;
        cursor_position++;
    }
}

void delete_char() {
    if (cursor_position > 0) {
        memmove(&text_buffer[cursor_position - 1], &text_buffer[cursor_position], strlen(text_buffer) - cursor_position + 1);
        cursor_position--;
    }
}

void display_buffer() {
    printf("%s\n", text_buffer);
    printf("Cursor position: %d\n", cursor_position);
}

char get_keypress() {
    return getchar();
}

void editor_loop() {
    create_file("test.txt");
    while (1) {
        display_buffer();
        char key = get_keypress();
        
        switch (key) {
            case '\n':
                insert_char('\n');
                break;
            case '\b':
                delete_char();
                break;
            case 'q':
                save_file();
                return;
            default:
                insert_char(key);
        }
    }
}

int main() {
    editor_loop();
    return 0;
}