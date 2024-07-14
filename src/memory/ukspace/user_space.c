#include <stdio.h>
#include <stdlib.h>
#include "user_space.h"

VirtualMemory allocate_user_space(size_t space_size)
{
    VirtualMemory vm;

    vm.size = space_size;
    vm.start_addr = malloc(vm.size);

    if (!vm.start_addr)
    {
        perror("Failed to allocate user space");
        exit(EXIT_FAILURE);
    }

    printf("User space allocated at address: %p, size: %zu bytes\n", vm.start_addr, vm.size);
    return vm;
}

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
        size_t space_size;
        printf("Enter the size of user space %zu in bytes: ", i);
        scanf("%zu", &space_size);

        vm_array[i] = allocate_user_space(space_size);
    }

    return vm_array;
}
