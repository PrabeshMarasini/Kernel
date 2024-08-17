#include "paging.h"
#include "memory.h"  // Include memory.h for kmalloc and kfree

pde_t *page_directory; // Removed __attribute__((aligned(4096)))
pte_t *first_page_table; // Removed __attribute__((aligned(4096)))

// Initialize paging
void init_paging() {
    // Dynamically allocate the page directory and first page table
    page_directory = (pde_t *)kmalloc(1024 * sizeof(pde_t));
    first_page_table = (pte_t *)kmalloc(1024 * sizeof(pte_t));

    if (!page_directory || !first_page_table) {
        // Handle memory allocation failure
        // Add your error handling code here (e.g., panic)
        return;
    }

    // Initialize page directory and page table entries
    for (int i = 0; i < 1024; i++) {
        page_directory[i].present = 0;
        page_directory[i].rw = 0;
        page_directory[i].user = 0;
        page_directory[i].reserved = 0;
        first_page_table[i].present = 0;
        first_page_table[i].rw = 0;
        first_page_table[i].user = 0;
        first_page_table[i].reserved = 0;
    }

    // Setup page directory entry
    page_directory[0].present = 1;
    page_directory[0].rw = 1;
    page_directory[0].user = 0;
    page_directory[0].reserved = (uint32_t)first_page_table >> 12;

    // Setup page table entries
    for (int i = 0; i < 1024; i++) {
        first_page_table[i].present = 1;
        first_page_table[i].rw = 1;
        first_page_table[i].user = 0;
        first_page_table[i].reserved = i;
    }

    // Load page directory register
    asm volatile("mov %0, %%cr3" :: "r" (page_directory));
    asm volatile("mov %cr0, %eax");
    asm volatile("or %eax, 0x80000000");
    asm volatile("mov %eax, %cr0");

    // Expand the paging capabilities as needed
    // You can add support for additional page tables and page directory entries
    // to handle more than 4 MB of memory
}

// Optional: If you're done with paging and need to clean up, you can free the page_directory and first_page_table
void cleanup_paging() {
    kfree(page_directory);
    kfree(first_page_table);
}
