#include "paging.h"

pde_t page_directory[1024] __attribute__((aligned(4096)));
pte_t first_page_table[1024] __attribute__((aligned(4096)));

void init_paging() {
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
