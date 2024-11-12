#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define ATA_PRIMARY_IO 0x1F0      // Base I/O port for the primary ATA bus
#define ATA_PRIMARY_CTRL 0x3F6    // Control port for the primary ATA bus

#define ATA_CMD_READ  0x20        // Read command for PIO mode
#define ATA_CMD_WRITE 0x30        // Write command for PIO mode
#define ATA_SECTOR_SIZE 512       // Size of a sector in bytes

void ata_read_sector(uint32_t lba, uint8_t *buffer);
void ata_write_sector(uint32_t lba, const uint8_t *buffer);
void ata_wait_for_drive_ready(void);  // Function to wait for drive to be ready
void ata_wait_for_drive_ready_with_timeout(void); // Function to wait for drive with timeout
void ata_send_command(uint8_t command);  // Function to send ATA command
void ata_select_drive(uint8_t drive);   // Function to select the drive
void print_str(const char *str);  // Function to print string (for error handling)

#endif
