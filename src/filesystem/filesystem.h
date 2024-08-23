#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

// Define the size of a file block
#define FS_BLOCK_SIZE 4096
#define MAX_FILES 128
#define NUM_DIRECT_BLOCKS 10
#define BLOCK_POINTERS_PER_BLOCK 1024

// Structure for inode
typedef struct {
    uint32_t size;
    uint32_t blocks[NUM_DIRECT_BLOCKS];
    uint32_t indirect_block;
} inode_t;

// Structure for a directory entry
typedef struct {
    char name[32];
    uint32_t inode;
} dir_entry_t;

// Structure for the superblock
typedef struct {
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t block_size;
} superblock_t;

// Structure for an open file
typedef struct {
    uint32_t inode;
    uint32_t pos;
} File;

// Function prototypes
void init_fs();
int create_file(const char* name);
int delete_file(const char* name);
int write_file(const char* name, const uint8_t* data, uint32_t size);
int read_file(const char* name, uint8_t* buffer, uint32_t size);
int file_exists(const char* name);
File* fs_open(const char* name);
size_t fs_read(File* file, char* buffer, size_t size);
size_t fs_write(File* file, const char* buffer, size_t size);
void fs_close(File* file);
char* list_files();
int save_file(const char* name, const uint8_t* data, uint32_t size);

#endif // FILESYSTEM_H
