#include "memory.h"
#include <stdint.h>

#define HEAP_SIZE 0x100000 // 1 MB
#define BLOCK_SIZE 0x1000 // 4 KB

static uint8_t heap[HEAP_SIZE]; // The heap area for dynamic memory allocation
static uint32_t free_blocks = HEAP_SIZE / BLOCK_SIZE; // Total number of blocks in the heap

typedef struct Block {
    uint32_t size; // Number of blocks
    struct Block* next; // Pointer to the next block in the free list
} Block;

static Block* free_list = (Block*)heap; // Pointer to the start of the free list

// Initialize the memory system
void init_memory() {
    free_list->size = free_blocks; // Set the size of the initial free block
    free_list->next = NULL; // No other blocks in the list yet
}

// Allocate memory of a given size
void* kmalloc(size_t size) {
    Block* current = free_list;
    Block* prev = NULL;

    // Find a block that is large enough
    while (current) {
        // Check if current block is large enough
        if (current->size >= (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE) {
            // Split the block if it's larger than needed
            if (current->size > (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE) {
                Block* new_block = (Block*)((uint8_t*)current + size + sizeof(Block));
                new_block->size = current->size - (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE;
                new_block->next = current->next;
                current->size = (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE;
                current->next = new_block;
            }

            // Remove the block from the free list
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }

            // Return the memory address after the block header
            return (void*)((uint8_t*)current + sizeof(Block));
        }

        // Move to the next block
        prev = current;
        current = current->next;
    }

    return NULL; // No sufficient block found
}

// Free previously allocated memory
void kfree(void* ptr) {
    Block* block = (Block*)((uint8_t*)ptr - sizeof(Block)); // Get the block header
    block->next = free_list; // Insert the block at the beginning of the free list
    free_list = block;
}
