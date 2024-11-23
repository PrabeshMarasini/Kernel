#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include "string.h"
#include "../memory/memory.h"

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t nmemb, size_t size);

static char heap[HEAP_SIZE];
static Block *free_list = (Block *)heap;
static size_t heap_index = 0;

void *malloc(size_t size) {
    if (size == 0) return NULL;

    size = (size + sizeof(Block) - 1) & ~(sizeof(Block) - 1); 
    Block *prev = NULL;
    Block *curr = free_list;
    while (curr) {
        if (curr->size >= size) {
            if (curr->size > size + sizeof(Block)) {
                Block *next = (Block *)((char *)curr + size + sizeof(Block));
                next->size = curr->size - size - sizeof(Block);
                next->next = curr->next;
                curr->next = next;
                curr->size = size;
            }
            if (prev) {
                prev->next = curr->next;
            } else {
                free_list = curr->next;
            }
            return (char *)curr + sizeof(Block);
        }
        prev = curr;
        curr = curr->next;
    }

    if ((heap_index + size + sizeof(Block)) <= HEAP_SIZE) {
        Block *new_block = (Block *)(heap + heap_index);
        new_block->size = size;
        heap_index += size + sizeof(Block);
        return (char *)new_block + sizeof(Block);
    }

    return NULL; 
}

void free(void *ptr) {
    if (ptr == NULL) return;

    Block *block = (Block *)((char *)ptr - sizeof(Block));
    block->next = free_list;
    free_list = block;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    Block *old_block = (Block *)((char *)ptr - sizeof(Block));
    size_t old_size = old_block->size;

    void *new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL;  
    }

    size_t copy_size = (size < old_size) ? size : old_size;
    memcpy(new_ptr, ptr, copy_size);

    free(ptr);
    return new_ptr;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

#endif 
