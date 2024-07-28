#include "filesystem.h"
#include "../memory/memory.h"
#include <string.h>
#include <stdarg.h>

// Custom snprintf implementation
int custom_snprintf(char* str, size_t size, const char* format, ...);

// Global variables
static superblock_t superblock;
static inode_t inodes[MAX_FILES];
static uint8_t data_blocks[MAX_FILES * 12][FS_BLOCK_SIZE];
static dir_entry_t root_dir[MAX_FILES];

// Function to initialize the file system
void init_fs() {
    superblock.total_blocks = MAX_FILES * 12;
    superblock.free_blocks = superblock.total_blocks;
    superblock.block_size = FS_BLOCK_SIZE;

    // Initialize inodes and root directory
    for (int i = 0; i < MAX_FILES; i++) {
        inodes[i].size = 0;
        for (int j = 0; j < NUM_DIRECT_BLOCKS; j++) {
            inodes[i].blocks[j] = 0;
        }
        inodes[i].indirect_block = 0;
        root_dir[i].name[0] = '\0';
        root_dir[i].inode = 0;
    }
}

// Function to allocate a block
int allocate_block() {
    for (int i = 0; i < superblock.total_blocks; i++) {
        if (((uint8_t*)data_blocks)[i * FS_BLOCK_SIZE] == 0) {
            superblock.free_blocks--;
            return i;
        }
    }
    return -1; // No free blocks available
}

// Function to free a block
void free_block(int block_index) {
    ((uint8_t*)data_blocks)[block_index * FS_BLOCK_SIZE] = 0;
    superblock.free_blocks++;
}

// Function to add a block to an inode
int add_block_to_inode(inode_t *inode, int block_index) {
    if (block_index < NUM_DIRECT_BLOCKS) {
        inode->blocks[block_index] = allocate_block();
    } else if (block_index < NUM_DIRECT_BLOCKS + BLOCK_POINTERS_PER_BLOCK) {
        if (inode->indirect_block == 0) {
            inode->indirect_block = allocate_block();
        }
        uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];
        indirect_block[block_index - NUM_DIRECT_BLOCKS] = allocate_block();
    }
    return 0; // Success
}

// Function to check if a file exists
int file_exists(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            return 1; // File exists
        }
    }
    return 0; // File does not exist
}

// Function to create a file
int create_file(const char* name) {
    if (file_exists(name)) {
        return -1; // File already exists
    }
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_dir[i].name[0] == '\0') {
            strncpy(root_dir[i].name, name, 31);
            root_dir[i].name[31] = '\0'; // Ensure null termination
            root_dir[i].inode = i;
            inodes[i].size = 0;
            return 0; // Success
        }
    }
    return -2; // No space available
}

// Function to delete a file
int delete_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            root_dir[i].name[0] = '\0';
            for (int j = 0; j < NUM_DIRECT_BLOCKS; j++) {
                if (inodes[i].blocks[j] != 0) {
                    free_block(inodes[i].blocks[j]);
                }
            }
            if (inodes[i].indirect_block != 0) {
                uint32_t* indirect_block = (uint32_t*)&data_blocks[inodes[i].indirect_block];
                for (int j = 0; j < BLOCK_POINTERS_PER_BLOCK; j++) {
                    if (indirect_block[j] != 0) {
                        free_block(indirect_block[j]);
                    }
                }
                free_block(inodes[i].indirect_block);
            }
            inodes[i].indirect_block = 0;
            inodes[i].size = 0;
            return 0; // Success
        }
    }
    return -1; // File not found
}

// Function to write to a file
int write_file(const char* name, const uint8_t* data, uint32_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            inode_t* inode = &inodes[root_dir[i].inode];
            uint32_t remaining_size = size;
            uint32_t block_index = 0;

            // Write data to direct blocks
            while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
                if (inode->blocks[block_index] == 0) {
                    inode->blocks[block_index] = allocate_block();
                }
                uint32_t write_size = remaining_size < FS_BLOCK_SIZE ? remaining_size : FS_BLOCK_SIZE;
                memcpy(&data_blocks[inode->blocks[block_index]], data, write_size);
                data += write_size;
                remaining_size -= write_size;
                block_index++;
            }

            // Write data to indirect blocks if needed
            if (remaining_size > 0) {
                if (inode->indirect_block == 0) {
                    inode->indirect_block = allocate_block();
                }
                uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

                for (int j = 0; j < BLOCK_POINTERS_PER_BLOCK && remaining_size > 0; j++) {
                    if (indirect_block[j] == 0) {
                        indirect_block[j] = allocate_block();
                    }
                    uint32_t write_size = remaining_size < FS_BLOCK_SIZE ? remaining_size : FS_BLOCK_SIZE;
                    memcpy(&data_blocks[indirect_block[j]], data, write_size);
                    data += write_size;
                    remaining_size -= write_size;
                }
            }

            inode->size = size;
            return 0; // Success
        }
    }
    return -1; // File not found
}

// Function to read from a file
int read_file(const char* name, uint8_t* buffer, uint32_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            inode_t* inode = &inodes[root_dir[i].inode];
            uint32_t remaining_size = size < inode->size ? size : inode->size;
            uint32_t block_index = 0;
            uint32_t read_size = 0;

            // Read data from direct blocks
            while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
                if (inode->blocks[block_index] != 0) {
                    uint32_t copy_size = remaining_size < FS_BLOCK_SIZE ? remaining_size : FS_BLOCK_SIZE;
                    memcpy(buffer + read_size, &data_blocks[inode->blocks[block_index]], copy_size);
                    read_size += copy_size;
                    remaining_size -= copy_size;
                }
                block_index++;
            }

            // Read data from indirect blocks if needed
            if (remaining_size > 0 && inode->indirect_block != 0) {
                uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

                for (int j = 0; j < BLOCK_POINTERS_PER_BLOCK && remaining_size > 0; j++) {
                    if (indirect_block[j] != 0) {
                        uint32_t copy_size = remaining_size < FS_BLOCK_SIZE ? remaining_size : FS_BLOCK_SIZE;
                        memcpy(buffer + read_size, &data_blocks[indirect_block[j]], copy_size);
                        read_size += copy_size;
                        remaining_size -= copy_size;
                    }
                }
            }

            return read_size; // Number of bytes read
        }
    }
    return -1; // File not found
}

// Function to open a file
File* fs_open(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, filename, 32) == 0) {
            static File file;
            file.inode = root_dir[i].inode;
            file.pos = 0;
            return &file;
        }
    }
    return NULL; // File not found
}

// Function to read data from an open file
size_t fs_read(File* file, char* buffer, size_t size) {
    if (file == NULL) {
        return 0;
    }
    inode_t* inode = &inodes[file->inode];
    uint32_t remaining_size = size < inode->size - file->pos ? size : inode->size - file->pos;
    uint32_t block_index = file->pos / FS_BLOCK_SIZE;
    uint32_t block_offset = file->pos % FS_BLOCK_SIZE;
    size_t read_size = 0;

    while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
        if (inode->blocks[block_index] != 0) {
            uint32_t copy_size = remaining_size < FS_BLOCK_SIZE - block_offset ? remaining_size : FS_BLOCK_SIZE - block_offset;
            memcpy(buffer + read_size, &data_blocks[inode->blocks[block_index]] + block_offset, copy_size);
            read_size += copy_size;
            remaining_size -= copy_size;
            block_offset = 0;
        }
        block_index++;
    }

    // Handle indirect blocks if needed
    if (remaining_size > 0 && inode->indirect_block != 0) {
        uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

        for (int i = 0; i < BLOCK_POINTERS_PER_BLOCK && remaining_size > 0; i++) {
            if (indirect_block[i] != 0) {
                uint32_t copy_size = remaining_size < FS_BLOCK_SIZE ? remaining_size : FS_BLOCK_SIZE;
                memcpy(buffer + read_size, &data_blocks[indirect_block[i]], copy_size);
                read_size += copy_size;
                remaining_size -= copy_size;
            }
        }
    }

    file->pos += read_size;
    return read_size;
}

// Function to close an open file
void fs_close(File* file) {
    // In this simple implementation, there's nothing to clean up
}

// Function to list all files
char* list_files() {
    static char buffer[4096];
    int pos = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (root_dir[i].name[0] != '\0') {
            pos += custom_snprintf(buffer + pos, sizeof(buffer) - pos, "File: %s\n", root_dir[i].name);
        }
    }
    return buffer;
}

// Custom snprintf implementation (simplified)
int custom_snprintf(char* str, size_t size, const char* format, ...) {
    // This is a very basic implementation. You might want to expand it based on your needs.
    size_t written = 0;
    const char* ptr = format;
    va_list args;
    va_start(args, format);

    while (*ptr && written < size - 1) {
        if (*ptr == '%' && *(ptr + 1) == 's') {
            const char* s = va_arg(args, const char*);
            size_t len = strlen(s);
            if (written + len >= size - 1) {
                len = size - written - 1;
            }
            memcpy(str + written, s, len);
            written += len;
            ptr += 2;
        } else {
            str[written++] = *ptr++;
        }
    }

    va_end(args);
    str[written] = '\0';
    return written;
}