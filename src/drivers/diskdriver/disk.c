#include "disk.h"
#include <stdint.h> // Include this header for uint8_t, uint16_t, etc.

// Inline functions for port I/O operations
static inline uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

static inline void port_byte_out(uint16_t port, uint8_t data) {
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

// Simple read function using BIOS interrupt
int disk_read(uint16_t lba, uint8_t *buffer) {
    asm volatile(
        "mov $0x02, %%ah;"        // Read function
        "mov $0x01, %%al;"        // Sectors to read
        "mov %0, %%bx;"           // Buffer
        "mov $0x00, %%dh;"        // Head
        "mov $0x00, %%dl;"        // Drive (0 = floppy, 0x80 = hard disk)
        "mov $0x00, %%ch;"        // Cylinder
        "mov $0x02, %%cl;"        // Sector (1-based)
        "int $0x13;"              // BIOS interrupt
        :
        : "m"(buffer)
        : "ax", "bx", "cx", "dx"
    );
    return 0; // Handle errors appropriately
}

// Simple write function using BIOS interrupt
int disk_write(uint16_t lba, const uint8_t *buffer) {
    asm volatile(
        "mov $0x03, %%ah;"        // Write function
        "mov $0x01, %%al;"        // Sectors to write
        "mov %0, %%bx;"           // Buffer
        "mov $0x00, %%dh;"        // Head
        "mov $0x00, %%dl;"        // Drive (0 = floppy, 0x80 = hard disk)
        "mov $0x00, %%ch;"        // Cylinder
        "mov $0x02, %%cl;"        // Sector (1-based)
        "int $0x13;"              // BIOS interrupt
        :
        : "m"(buffer)
        : "ax", "bx", "cx", "dx"
    );
    return 0; // Handle errors appropriately
}
