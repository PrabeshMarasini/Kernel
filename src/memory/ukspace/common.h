#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

// Define VirtualMemory struct
typedef struct {
    void* start_addr;
    size_t size;
} VirtualMemory;

#endif // COMMON_H
