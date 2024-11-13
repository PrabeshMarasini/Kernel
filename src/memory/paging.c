#include "paging.h"
#include "memory.h"

pde_t *page_directory;
pte_t *first_page_table;

void init_paging() {
    page_directory = (pde_t *)kmalloc(1024 * sizeof(pde_t));
    first_page_table = (pte_t *)kmalloc(1024 * sizeof(pte_t));

    if (!page_directory || !first_page_table) {
        return;
    }

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

    page_directory[0].present = 1;
    page_directory[0].rw = 1;
    page_directory[0].user = 0;
    page_directory[0].reserved = (uint32_t)first_page_table >> 12;

    for (int i = 0; i < 1024; i++) {
        first_page_table[i].present = 1;
        first_page_table[i].rw = 1;
        first_page_table[i].user = 0;
        first_page_table[i].reserved = i;
    }

    asm volatile("mov %0, %%cr3" :: "r" (page_directory));
    asm volatile("mov %cr0, %eax");
    asm volatile("or %eax, 0x80000000");
    asm volatile("mov %eax, %cr0");
}

void cleanup_paging() {
    kfree(page_directory);
    kfree(first_page_table);
}
