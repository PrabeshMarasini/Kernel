#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE 0x100000 // 1 MB
#define BLOCK_SIZE 0x1000 // 4 KB

typedef struct Block {
    uint32_t size; 
    struct Block* next; 
} Block;

void init_memory();
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif 