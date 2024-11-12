#include "filesystem.h"
#include "../memory/memory.h"
#include <string.h>
#include <stdarg.h>
#include "../drivers/diskdriver/disk.h"

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
typedef struct {
    char filename[FILENAME_LENGTH];
    uint32_t size;
    uint32_t start_sector;
} FileEntry;

// In-memory file table buffer
FileEntry file_table[MAX_FILES];

// Read the file table into memory (assuming 1 file entry per sector for simplicity)
void load_file_table() {
    for (int i = 0; i < MAX_FILES; i++) {
        ata_read_sector(FILE_TABLE_START + i, (uint8_t*)&file_table[i]);
    }
}

// Save the file table to disk
void save_file_table() {
    for (int i = 0; i < MAX_FILES; i++) {
        ata_write_sector(FILE_TABLE_START + i, (uint8_t*)&file_table[i]);
    }
}

// Create a file by saving it to disk with the given name and content
int create_file(const char* filename, const uint8_t* content, uint32_t size) {
    load_file_table();

    // Check if the file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            return -1; // File already exists
        }
    }

    // Find an empty entry in the file table
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].filename[0] == '\0') { // Empty entry if filename is empty
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        return -2; // No space in file table
    }

    // Calculate required sectors
    uint32_t required_sectors = (size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;

    // Find a free sector for the new file
    uint32_t start_sector = DATA_START_SECTOR;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].filename[0] != '\0') { // Skip used entries
            start_sector = file_table[i].start_sector + (file_table[i].size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
        }
    }

    // Set file entry metadata
    strncpy(file_table[file_index].filename, filename, FILENAME_LENGTH);
    file_table[file_index].size = size;
    file_table[file_index].start_sector = start_sector;

    // Save the file table back to disk
    save_file_table();

    // Write file content to disk
    for (uint32_t i = 0; i < required_sectors; i++) {
        ata_write_sector(start_sector + i, content + i * ATA_SECTOR_SIZE);
    }

    return 0; // Success
}

// Function to delete a file
int delete_file(const char* filename) {
    load_file_table();  // Load the file table from the disk

    // Search for the file in the file table
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(file_table[i].filename, filename, FILENAME_LENGTH) == 0) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        return -1;  // File not found
    }

    // Free the sectors occupied by the file
    uint32_t total_sectors = (file_table[file_index].size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;
    for (uint32_t i = 0; i < total_sectors; i++) {
        free_block(file_table[file_index].start_sector + i);  // Free each sector occupied by the file
    }

    // Clear the file table entry
    file_table[file_index].filename[0] = '\0';  // Mark the filename as empty
    file_table[file_index].size = 0;
    file_table[file_index].start_sector = 0;

    // Save the updated file table back to the disk
    save_file_table();  // Write the updated file table back to disk

    return 0;  // Success
}


// Function to write to a file
int write_file(const char* name, const uint8_t* data, uint32_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            inode_t* inode = &inodes[root_dir[i].inode];
            uint32_t remaining_size = size;
            uint32_t block_index = 0;
            uint32_t data_index = 0;

            // Write data to direct blocks
            while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
                if (inode->blocks[block_index] == 0) {
                    inode->blocks[block_index] = allocate_block();
                }
                uint32_t write_size = FS_BLOCK_SIZE - (data_index % FS_BLOCK_SIZE);
                if (write_size > remaining_size) {
                    write_size = remaining_size;
                }
                memcpy(&data_blocks[inode->blocks[block_index]] + (data_index % FS_BLOCK_SIZE), data + data_index, write_size);
                data_index += write_size;
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
                    uint32_t write_size = FS_BLOCK_SIZE;
                    if (write_size > remaining_size) {
                        write_size = remaining_size;
                    }
                    memcpy(&data_blocks[indirect_block[j]], data + data_index, write_size);
                    data_index += write_size;
                    remaining_size -= write_size;
                }
            }

            inode->size = size;
            return 0; // Success
        }
    }
    return -1; // File not found
}

// Function to save a file's content
int save_file(const char* name, const uint8_t* data, uint32_t size) {
    // Check if the file exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (strncmp(root_dir[i].name, name, 32) == 0) {
            inode_t* inode = &inodes[root_dir[i].inode];
            uint32_t remaining_size = size;
            uint32_t block_index = 0;
            uint32_t data_index = 0;

            // Save data to direct blocks
            while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
                if (inode->blocks[block_index] == 0) {
                    inode->blocks[block_index] = allocate_block();
                }
                uint32_t write_size = FS_BLOCK_SIZE - (data_index % FS_BLOCK_SIZE);
                if (write_size > remaining_size) {
                    write_size = remaining_size;
                }
                memcpy(&data_blocks[inode->blocks[block_index]] + (data_index % FS_BLOCK_SIZE), data + data_index, write_size);
                data_index += write_size;
                remaining_size -= write_size;
                block_index++;
            }

            // Save data to indirect blocks if needed
            if (remaining_size > 0) {
                if (inode->indirect_block == 0) {
                    inode->indirect_block = allocate_block();
                }
                uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

                for (int j = 0; j < BLOCK_POINTERS_PER_BLOCK && remaining_size > 0; j++) {
                    if (indirect_block[j] == 0) {
                        indirect_block[j] = allocate_block();
                    }
                    uint32_t write_size = FS_BLOCK_SIZE;
                    if (write_size > remaining_size) {
                        write_size = remaining_size;
                    }
                    memcpy(&data_blocks[indirect_block[j]], data + data_index, write_size);
                    data_index += write_size;
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
            uint32_t to_read = FS_BLOCK_SIZE - block_offset;
            if (to_read > remaining_size) {
                to_read = remaining_size;
            }
            memcpy(buffer + read_size, &data_blocks[inode->blocks[block_index]] + block_offset, to_read);
            read_size += to_read;
            remaining_size -= to_read;
            block_index++;
            block_offset = 0;
        }
    }

    if (remaining_size > 0 && inode->indirect_block != 0) {
        uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

        while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS + BLOCK_POINTERS_PER_BLOCK) {
            if (indirect_block[block_index - NUM_DIRECT_BLOCKS] != 0) {
                uint32_t to_read = FS_BLOCK_SIZE;
                if (to_read > remaining_size) {
                    to_read = remaining_size;
                }
                memcpy(buffer + read_size, &data_blocks[indirect_block[block_index - NUM_DIRECT_BLOCKS]], to_read);
                read_size += to_read;
                remaining_size -= to_read;
                block_index++;
            }
        }
    }

    file->pos += read_size;
    return read_size;
}

// Function to write data to an open file
size_t fs_write(File* file, const char* buffer, size_t size) {
    if (file == NULL) {
        return 0;
    }
    inode_t* inode = &inodes[file->inode];
    uint32_t remaining_size = size;
    uint32_t block_index = file->pos / FS_BLOCK_SIZE;
    uint32_t block_offset = file->pos % FS_BLOCK_SIZE;
    size_t written_size = 0;

    while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS) {
        if (inode->blocks[block_index] == 0) {
            inode->blocks[block_index] = allocate_block();
        }
        uint32_t to_write = FS_BLOCK_SIZE - block_offset;
        if (to_write > remaining_size) {
            to_write = remaining_size;
        }
        memcpy(&data_blocks[inode->blocks[block_index]] + block_offset, buffer + written_size, to_write);
        written_size += to_write;
        remaining_size -= to_write;
        block_index++;
        block_offset = 0;
    }

    if (remaining_size > 0) {
        if (inode->indirect_block == 0) {
            inode->indirect_block = allocate_block();
        }
        uint32_t* indirect_block = (uint32_t*)&data_blocks[inode->indirect_block];

        while (remaining_size > 0 && block_index < NUM_DIRECT_BLOCKS + BLOCK_POINTERS_PER_BLOCK) {
            if (indirect_block[block_index - NUM_DIRECT_BLOCKS] == 0) {
                indirect_block[block_index - NUM_DIRECT_BLOCKS] = allocate_block();
            }
            uint32_t to_write = FS_BLOCK_SIZE;
            if (to_write > remaining_size) {
                to_write = remaining_size;
            }
            memcpy(&data_blocks[indirect_block[block_index - NUM_DIRECT_BLOCKS]], buffer + written_size, to_write);
            written_size += to_write;
            remaining_size -= to_write;
            block_index++;
        }
    }

    if (file->pos + size > inode->size) {
        inode->size = file->pos + size;
    }

    file->pos += written_size;
    return written_size;
}

// Function to list all files
// Function to list all files from disk
char* list_files() {
    static char buffer[4096];
    int pos = 0;
    buffer[0] = '\0'; // Ensure buffer is initially empty

    // Load the file table from disk
    load_file_table();  // This function should read the file table from disk using ata_read_sector

    // Iterate over the file table to list all files
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].filename[0] != '\0') { // Only include valid files
            // Append file name to the buffer
            pos += custom_snprintf(buffer + pos, sizeof(buffer) - pos, "File: %s\n", file_table[i].filename);
        }
    }

    return buffer;
}


// Custom snprintf implementation
int custom_snprintf(char* str, size_t size, const char* format, ...) {
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
