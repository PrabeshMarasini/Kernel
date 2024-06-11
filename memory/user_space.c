#include <stdio.h>
#include <stdlib.h>
#include "user_space.h"

// User space memory allocation
VirtualMemory allocate_user_space()
{
    VirtualMemory vm;

    vm.size = USER_SPACE_SIZE;
    vm.start_addr = malloc(vm.size);

    if (!vm.start_addr)
    {
        perror("Failed to allocate user space");
        exit(EXIT_FAILURE);
    }

    printf("User space allocated at address: %p, size: %zu bytes\n", vm.start_addr, vm.size);
    return vm;
}

// Allocate multiple user spaces
VirtualMemory* allocate_multiple_user_spaces(size_t count)
{
    VirtualMemory* vm_array = (VirtualMemory*)malloc(count * sizeof(VirtualMemory));
    if (!vm_array)
    {
        perror("Failed to allocate memory for user space array");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < count; ++i)
    {
        vm_array[i] = allocate_user_space();
    }

    return vm_array;
}
