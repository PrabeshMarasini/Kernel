#ifndef SNPRINTF_H
#define SNPRINTF_H

#include <stddef.h>

#define BUFFER_SIZE 1024

int snprintf(char* buffer, size_t size, const char* format, ...);

#endif // SNPRINTF_H
