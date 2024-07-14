#include <stdio.h>
#include <stdlib.h>
#include "user_space.h"
#include "kernel_space.h"

#define USER_SPACE_SIZE (1 << 30)  // 1 GB

int main()
{
    size_t num_user_spaces;
    printf("Enter the number of user spaces to allocate: ");
    scanf("%zu", &num_user_spaces);

    VirtualMemory* user_spaces = allocate_multiple_user_spaces(num_user_spaces);
    VirtualMemory kernel_space = allocate_kernel_space();

    // Example: Assigning values to the first integer in each user space
    for (size_t i = 0; i < num_user_spaces; ++i)
    {
        // Check if allocation failed
        if (!user_spaces[i].start_addr)
        {
            fprintf(stderr, "Failed to allocate user space number %zu\n", i);
            continue;
        }

        *(int*)(user_spaces[i].start_addr) = (int)i;
        printf("Value at user space %zu address: %d\n", i, *(int*)(user_spaces[i].start_addr));
    }

    *(int*)(kernel_space.start_addr) = 20;
    printf("Value at kernel space address: %d\n", *(int*)(kernel_space.start_addr));

    // Free the allocated user spaces
    for (size_t i = 0; i < num_user_spaces; ++i)
    {
        if (user_spaces[i].start_addr)
            free(user_spaces[i].start_addr);
    }
    free(user_spaces);

    free(kernel_space.start_addr);

    return 0;
}
