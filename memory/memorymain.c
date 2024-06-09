#include <stdio.h>
#include <stdlib.h>

#define USER_SPACE_SIZE (1 << 30)  // 1 GB
#define KERNEL_SPACE_SIZE (1 << 30)  // 1 GB

typedef struct
{
    void* start_addr;
    size_t size;
}
VirtualMemory;

// User space memory allocation
VirtualMemory allocate_user_space()
{
    VirtualMemory vm;

    vm.size = USER_SPACE_SIZE;
    vm.start_addr = malloc(vm.size);

    if (!vm.start_addr)
    {
        perror("Faile to allocate user space");
        exit(EXIT_FAILURE);
    }

    printf("User space allocated at address: %p, size: %zu bytes\n", vm.start_addr, vm.size);
    return vm;
}

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

int main()
{
    VirtualMemory user_space, kernel_space;

    user_space = allocate_user_space();

    kernel_space = allocate_kernel_space();

    *(int*)(user_space.start_addr) = 10;
    printf("Value at user space address: %d\n", *(int*)(user_space.start_addr));

    *(int*)(kernel_space.start_addr) = 20;
    printf("Value at kernel space address: %d\n", *(int*)(kernel_space.start_addr));

    free(user_space.start_addr);
    free(kernel_space.start_addr);

    return 0;
}


