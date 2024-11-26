#include "memory.h"
#include <stddef.h>

static uint8_t heap[HEAP_SIZE]; 
static uint32_t free_blocks = HEAP_SIZE / BLOCK_SIZE; 
static Block* free_list = (Block*)heap; 

void init_memory() {
    free_blocks = HEAP_SIZE / BLOCK_SIZE;

    free_list->size = free_blocks; 
    free_list->next = NULL; 
}

void* kmalloc(size_t size) {
    Block* current = free_list;
    Block* prev = NULL;
    size_t required_blocks = (size + sizeof(Block) + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (required_blocks > free_blocks) {
        return NULL; 
    }

    while (current) {
        if (current->size >= required_blocks) {
            if (current->size > required_blocks) {
                Block* new_block = (Block*)((uint8_t*)current + required_blocks * BLOCK_SIZE);
                new_block->size = current->size - required_blocks;
                new_block->next = current->next;
                current->size = required_blocks;
                current->next = new_block;
            }

            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }

            free_blocks -= required_blocks;
            return (void*)((uint8_t*)current + sizeof(Block));
        }

        prev = current;
        current = current->next;
    }

    return NULL; 
}

void kfree(void* ptr) {
    Block* block = (Block*)((uint8_t*)ptr - sizeof(Block)); 
    Block* current = free_list;
    Block* prev = NULL;

    while (current && current < block) {
        prev = current;
        current = current->next;
    }

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

    free_blocks += block->size;
}
