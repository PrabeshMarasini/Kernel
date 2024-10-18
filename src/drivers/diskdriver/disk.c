#include <stdint.h>  // For uint8_t, uint16_t

#define ATA_PRIMARY_IO        0x1F0   // Primary IO base
#define ATA_PRIMARY_CONTROL   0x3F6   // Primary control port
#define ATA_DRIVE_MASTER      0xA0    // Master drive
#define ATA_DRIVE_SLAVE       0xB0    // Slave drive
#define ATA_CMD_READ          0x20    // Read command
#define ATA_CMD_WRITE         0x30    // Write command
#define SECTOR_SIZE           512     // Sector size in bytes

// Port I/O functions for reading and writing bytes and words
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
    // Wait for an I/O operation to complete
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

void ata_wait_ready() {
    // Wait until the drive is not busy (bit 7 = 0)
    while (inb(ATA_PRIMARY_IO + 7) & 0x80);
}

void ata_select_drive(uint8_t drive) {
    // Select the master or slave drive
    if (drive == 0)
        outb(ATA_PRIMARY_IO + 6, ATA_DRIVE_MASTER);
    else
        outb(ATA_PRIMARY_IO + 6, ATA_DRIVE_SLAVE);
}

void ata_wait_for_drive_ready() {
    // Wait until the drive is ready to accept commands (bit 3 = 1)
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08));
}

void ata_send_command(uint8_t command) {
    outb(ATA_PRIMARY_IO + 7, command);
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_ready();

    // Select the drive (0 = master, 1 = slave)
    ata_select_drive(0);

    // Send the LBA (Logical Block Address) to the drive
    outb(ATA_PRIMARY_IO + 2, 1);                           // Sector count (1 sector)
    outb(ATA_PRIMARY_IO + 3, (uint8_t)(lba & 0xFF));       // LBA low byte
    outb(ATA_PRIMARY_IO + 4, (uint8_t)((lba >> 8) & 0xFF));// LBA mid byte
    outb(ATA_PRIMARY_IO + 5, (uint8_t)((lba >> 16) & 0xFF));// LBA high byte
    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));// Drive and LBA bits

    ata_send_command(ATA_CMD_READ);
    ata_wait_for_drive_ready();

    // Read data from the data port (sector size is 512 bytes)
    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        ((uint16_t*)buffer)[i] = inw(ATA_PRIMARY_IO);
    }
}

void ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    ata_wait_ready();

    // Select the drive (0 = master, 1 = slave)
    ata_select_drive(0);

    // Send the LBA (Logical Block Address) to the drive
    outb(ATA_PRIMARY_IO + 2, 1);                           // Sector count (1 sector)
    outb(ATA_PRIMARY_IO + 3, (uint8_t)(lba & 0xFF));       // LBA low byte
    outb(ATA_PRIMARY_IO + 4, (uint8_t)((lba >> 8) & 0xFF));// LBA mid byte
    outb(ATA_PRIMARY_IO + 5, (uint8_t)((lba >> 16) & 0xFF));// LBA high byte
    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));// Drive and LBA bits

    ata_send_command(ATA_CMD_WRITE);
    ata_wait_for_drive_ready();

    // Write data to the data port (sector size is 512 bytes)
    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        outw(ATA_PRIMARY_IO, ((uint16_t*)buffer)[i]);
    }

    // Flush the cache by sending the cache flush command
    ata_send_command(0xE7);
    ata_wait_for_drive_ready();
}
