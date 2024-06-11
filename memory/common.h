#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define USER_SPACE_SIZE (1 << 30)  // 1 GB
#define KERNEL_SPACE_SIZE (1 << 30)  // 1 GB

typedef struct
{
    void* start_addr;
    size_t size;
} VirtualMemory;

#endif // COMMON_H
