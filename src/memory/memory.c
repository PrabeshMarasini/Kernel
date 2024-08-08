#include "memory.h"
#include <stddef.h>

static uint8_t heap[HEAP_SIZE]; // The heap area for dynamic memory allocation
static uint32_t free_blocks = HEAP_SIZE / BLOCK_SIZE; // Total number of blocks in the heap
static Block* free_list = (Block*)heap; // Pointer to the start of the free list

// Initialize the memory system
void init_memory() {
    // No need to modify HEAP_SIZE; it's a constant value from the macro
    free_blocks = HEAP_SIZE / BLOCK_SIZE;

    free_list->size = free_blocks; // Set the size of the initial free block
    free_list->next = NULL; // No other blocks in the list yet
}

// Allocate memory of a given size
void* kmalloc(size_t size) {
    Block* current = free_list;
    Block* prev = NULL;
    size_t required_blocks = (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Check if the requested size is larger than the available heap size
    if (required_blocks > free_blocks) {
        return NULL; // Unable to allocate the requested memory
    }

    // Find a block that is large enough
    while (current) {
        if (current->size >= required_blocks) {
            // Split the block if it's larger than needed
            if (current->size > required_blocks) {
                Block* new_block = (Block*)((uint8_t*)current + required_blocks * BLOCK_SIZE);
                new_block->size = current->size - required_blocks;
                new_block->next = current->next;
                current->size = required_blocks;
                current->next = new_block;
            }

            // Remove the block from the free list
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }

            // Update the available free blocks
            free_blocks -= required_blocks;

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
    Block* current = free_list;
    Block* prev = NULL;

    // Find the correct position to insert the block in the free list
    while (current && current < block) {
        prev = current;
        current = current->next;
    }

    // Merge the freed block with the adjacent free blocks, if possible
    if (prev && (uint8_t*)prev + prev->size * BLOCK_SIZE == (uint8_t*)block) {
        prev->size += block->size;
        prev->next = block->next;
    } else {
        block->next = current;
        if (prev) {
            prev->next = block;
        } else {
            free_list = block;
        }
    }

    // Update the available free blocks
    free_blocks += block->size;
}
