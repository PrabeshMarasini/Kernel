#include <stddef.h>
#include <stdint.h>

// Define memory pool size
#define MEMORY_POOL_SIZE 1024 * 1024  // 1 MB

// Memory block structure
typedef struct MemoryBlock {
    size_t size;                 // Size of the block
    int free;                    // Is the block free?
    struct MemoryBlock* next;    // Next block in the list
} MemoryBlock;

// Memory pool
static uint8_t memory_pool[MEMORY_POOL_SIZE];

// Head of the free list
static MemoryBlock* free_list = (MemoryBlock*)memory_pool;

// Initialize the memory manager
void init_memory_manager() {
    free_list->size = MEMORY_POOL_SIZE - sizeof(MemoryBlock);
    free_list->free = 1;
    free_list->next = NULL;
}

// Split a block into two if it's too large
void split_block(MemoryBlock* block, size_t size) {
    MemoryBlock* new_block = (MemoryBlock*)((uint8_t*)block + sizeof(MemoryBlock) + size);
    new_block->size = block->size - size - sizeof(MemoryBlock);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->free = 0;
    block->next = new_block;
}

// Allocate memory
void* kmalloc(size_t size) {
    MemoryBlock* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(MemoryBlock)) {
                split_block(current, size);
            } else {
                current->free = 0;
            }
            return (void*)((uint8_t*)current + sizeof(MemoryBlock));
        }
        current = current->next;
    }
    return NULL;  // No suitable block found
}

// Merge consecutive free blocks
void merge_blocks() {
    MemoryBlock* current = free_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(MemoryBlock);
            current->next = current->next->next;
        }
        current = current->next;
    }
}

// Free memory
void kfree(void* ptr) {
    if (!ptr) return;
    
    MemoryBlock* block = (MemoryBlock*)((uint8_t*)ptr - sizeof(MemoryBlock));
    block->free = 1;
    merge_blocks();
}

int main() {
    init_memory_manager();
    
    // Example usage
    void* ptr1 = kmalloc(256);
    void* ptr2 = kmalloc(512);
    
    // Use allocated memory for something
    // For example: storing data, creating data structures, etc.
    // Example: memset(ptr1, 0, 256);  // Initialize memory to zero (if you include <string.h>)
    
    // Free the allocated memory when done
    kfree(ptr1);
    
    void* ptr3 = kmalloc(128);

    // Use allocated memory for something
    // Example: strcpy(ptr3, "Hello, Kernel!");  // Copy string into allocated memory (if you include <string.h>)
    
    // Free the allocated memory when done
    kfree(ptr2);
    kfree(ptr3);

    return 0;
}
