// sprintf.c
#include "sprintf.h"
#include <stdarg.h>
#include "snprintf.h"

static void append(char* buffer, size_t* offset, const char* str, size_t len) {
    while (*offset + len > BUFFER_SIZE) {
        len--;
    }
    if (*offset < BUFFER_SIZE) {
        if (len > BUFFER_SIZE - *offset) {
            len = BUFFER_SIZE - *offset;
        }
        for (size_t i = 0; i < len; i++) {
            buffer[*offset + i] = str[i];
        }
        *offset += len;
    }
}

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t offset = 0;

    const char* p = format;
    while (*p && offset < BUFFER_SIZE) {
        if (*p == '%') {
            p++;
            if (*p == 'd') {
                int value = va_arg(args, int);
                char temp[20];
                int len = 0;

                // Convert integer to string
                int temp_value = value;
                do {
                    temp[len++] = '0' + (temp_value % 10);
                    temp_value /= 10;
                } while (temp_value > 0);
                
                // Reverse the string
                for (int i = 0; i < len / 2; i++) {
                    char t = temp[i];
                    temp[i] = temp[len - 1 - i];
                    temp[len - 1 - i] = t;
                }
                
                append(buffer, &offset, temp, len);
            }
            // Add handling for other formats as needed
        } else {
            append(buffer, &offset, p, 1);
            p++;
        }
    }

    va_end(args);
    return (int)offset;
}
