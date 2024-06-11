#include <stdio.h>
#include <stdlib.h>
#include "kernel_space.h"

// Kernel space memory allocation
VirtualMemory allocate_kernel_space()
{
    VirtualMemory vm;

    vm.size = KERNEL_SPACE_SIZE;
    vm.start_addr = malloc(vm.size);

    if (!vm.start_addr)
    {
        perror("Failed to allocate kernel space");
        exit(EXIT_FAILURE);
    }

    printf("Kernel space allocated at address: %p, size: %zu bytes\n", vm.start_addr, vm.size);
    return vm;
}
