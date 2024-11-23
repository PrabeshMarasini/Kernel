#include <stdint.h>
#include "disk.h"

#define ATA_PRIMARY_IO        0x1F0   // Primary IO base
#define ATA_PRIMARY_CONTROL   0x3F6   // Primary control port
#define ATA_DRIVE_MASTER     0xA0    // Master drive
#define ATA_DRIVE_SLAVE      0xB0    // Slave drive
#define ATA_CMD_READ         0x20    // Read command
#define ATA_CMD_WRITE        0x30    // Write command
#define SECTOR_SIZE          512     // Sector size in bytes

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void io_wait() {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

void ata_wait_ready() {
    while (inb(ATA_PRIMARY_IO + 7) & 0x80);
}

void ata_select_drive(uint8_t drive) {
    if (drive == 0)
        outb(ATA_PRIMARY_IO + 6, ATA_DRIVE_MASTER);
    else
        outb(ATA_PRIMARY_IO + 6, ATA_DRIVE_SLAVE);
}

void ata_wait_for_drive_ready() {
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08));
}

void ata_send_command(uint8_t command) {
    outb(ATA_PRIMARY_IO + 7, command);
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    uint8_t status;
    int timeout = 10000;
    
    while (((status = inb(ATA_PRIMARY_IO + 7)) & 0x80) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return -1;

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    
    timeout = 10000;
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x40) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return -1;

    outb(ATA_PRIMARY_IO + 1, 0x00);
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));

    outb(ATA_PRIMARY_IO + 7, ATA_CMD_READ);
    
    timeout = 10000;
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08) && timeout > 0) {
        status = inb(ATA_PRIMARY_IO + 7);
        if (status & 0x01) return -1;
        timeout--;
    }
    if (timeout == 0) return -1;

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        ((uint16_t*)buffer)[i] = inw(ATA_PRIMARY_IO);
    }

    status = inb(ATA_PRIMARY_IO + 7);
    if (status & 0x01) return -1;

    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    uint8_t status;
    int timeout = 10000;
    
    while (((status = inb(ATA_PRIMARY_IO + 7)) & 0x80) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return -1;

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    
    timeout = 10000;
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x40) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return -1;

    outb(ATA_PRIMARY_IO + 1, 0x00);
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));

    outb(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE);
    
    timeout = 10000;
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08) && timeout > 0) {
        status = inb(ATA_PRIMARY_IO + 7);
        if (status & 0x01) return -1;
        timeout--;
    }
    if (timeout == 0) return -1;

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        outw(ATA_PRIMARY_IO, ((uint16_t*)buffer)[i]);
    }

    outb(ATA_PRIMARY_IO + 7, 0xE7);
    
    timeout = 10000;
    while ((inb(ATA_PRIMARY_IO + 7) & 0x80) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return -1;

    return 0;
}

void ata_wait_for_drive_ready_with_timeout() {
    int timeout = 10000;
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08)) {
        if (--timeout <= 0) return;
    }
}