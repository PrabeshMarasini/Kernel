#include "filesystem.h"
#include "../drivers/diskdriver/disk.h"
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

int custom_snprintf(char* str, size_t size, const char* format, ...);

#define FILE_TABLE_START 1000
#define MAX_FILE_CONTENT ATA_SECTOR_SIZE  

FileEntry file_table[MAX_FILES];
static int file_table_loaded = 0;
static int file_table_dirty = 0;

void init_fs() {
    for (int i = 0; i < MAX_FILES; i++) {
        memset(&file_table[i], 0, sizeof(FileEntry));  
    }
    file_table_loaded = 1;
    file_table_dirty = 1;
    save_file_table();  
}

void ensure_file_table_loaded() {
    if (!file_table_loaded) {
        for (int i = 0; i < MAX_FILES; i++) {
            ata_read_sector(FILE_TABLE_START + i, (uint8_t*)&file_table[i]);
        }
        file_table_loaded = 1;
    }
}

void load_file_table() {
    ensure_file_table_loaded();
}

void save_file_table() {
    if (file_table_dirty) {
        for (int i = 0; i < MAX_FILES; i++) {
            ata_write_sector(FILE_TABLE_START + i, (uint8_t*)&file_table[i]);
        }
        file_table_dirty = 0;
    }
}

void mark_file_table_dirty() {
    file_table_dirty = 1;
}

int create_file(const char* filename, const uint8_t* content, uint32_t size) {
    ensure_file_table_loaded();
    
    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            return -1;  
        }
    }

    // Find empty slot
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].filename[0] == '\0') {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        return -2;  
    }

    if (size > ATA_SECTOR_SIZE) {
        return -3;  
    }

    strncpy(file_table[file_index].filename, filename, FILENAME_LENGTH);
    file_table[file_index].size = size;
    memcpy(file_table[file_index].content, content, size);
    file_table[file_index].is_occupied = 1;  

    mark_file_table_dirty();
    save_file_table();  
    return 0;  
}

int save_file(const char* filename, const char* content, uint32_t size) {
    ensure_file_table_loaded();
    
    if (size > MAX_FILE_CONTENT) {
        return -1;  
    }
    
    // Try to update existing file
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].is_occupied && 
            strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            file_table[i].size = size;
            memcpy(file_table[i].content, content, size);
            file_table[i].content[size] = '\0';  
            mark_file_table_dirty();
            save_file_table();  
            return 0;  
        }
    }
    
    // Create new file
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_table[i].is_occupied) {
            memset(&file_table[i], 0, sizeof(FileEntry));  
            strncpy(file_table[i].filename, filename, FILENAME_LENGTH - 1);
            file_table[i].filename[FILENAME_LENGTH - 1] = '\0';  
            file_table[i].size = size;
            memcpy(file_table[i].content, content, size);
            file_table[i].content[MAX_FILE_CONTENT - 1] = '\0';  
            file_table[i].is_occupied = 1;  
            mark_file_table_dirty();
            save_file_table();  
            return 0;  
        }
    }
    
    return -1;  
}

int delete_file(const char* filename) {
    ensure_file_table_loaded();
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            memset(&file_table[i], 0, sizeof(FileEntry)); 
            mark_file_table_dirty();
            save_file_table();  
            return 0;  
        }
    }

    return -1;  
}

int read_file(const char* filename, uint8_t* buffer, uint32_t size) {
    ensure_file_table_loaded();
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            if (size > file_table[i].size) {
                size = file_table[i].size;
            }
            memcpy(buffer, file_table[i].content, size);
            return size;
        }
    }
    return -1;
}

int fs_open(const char* filename) {
    ensure_file_table_loaded();
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            return i;  
        }
    }

    return -1;
}

int fs_read(int file_index, uint8_t* buffer, uint32_t size) {
    ensure_file_table_loaded();
    
    if (file_index < 0 || file_index >= MAX_FILES || file_table[file_index].filename[0] == '\0') {
        return -1;
    }

    FileEntry* file = &file_table[file_index];
    if (size > file->size) {
        size = file->size;
    }

    memcpy(buffer, file->content, size);  
    return size;  
}

char* list_files() {
    static char buffer[4096];
    int pos = 0;
    buffer[0] = '\0';
    ensure_file_table_loaded();

    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].filename[0] != '\0') {
            pos += custom_snprintf(buffer + pos, sizeof(buffer) - pos, "File: %s\n",
                             file_table[i].filename, file_table[i].size);
        }
    }

    return buffer;
}

int fs_close(int file_index) {
    ensure_file_table_loaded();

    if (file_index < 0 || file_index >= MAX_FILES || file_table[file_index].filename[0] == '\0') {
        return -1;
    }

    if (!file_table[file_index].is_open) {
        return -2;
    }

    file_table[file_index].is_open = 0;
    mark_file_table_dirty();
    save_file_table();

    return 0;
}

int custom_snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int i = 0;

    for (int j = 0; format[j] != '\0'; j++) {
        if (i >= size - 1) break;

        if (format[j] == '%' && format[j + 1] == 's') {
            j++;
            char* s = va_arg(args, char*);
            while (*s != '\0') {
                str[i++] = *s++;
                if (i >= size - 1) break;
            }
        } else if (format[j] == '%' && format[j + 1] == 'd') {
            j++;
            int num = va_arg(args, int);
            if (num == 0) {
                str[i++] = '0';
            } else {
                char buf[10];
                int n = 0;
                while (num > 0) {
                    buf[n++] = '0' + (num % 10);
                    num /= 10;
                }
                for (int k = n - 1; k >= 0; k--) {
                    str[i++] = buf[k];
                    if (i >= size - 1) break;
                }
            }
        } else {
            str[i++] = format[j];
        }
    }

    str[i] = '\0';
    va_end(args);
    return i;
}
