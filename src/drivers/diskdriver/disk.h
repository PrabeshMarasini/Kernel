#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define SECTOR_SIZE 512

// Functions
int disk_read(uint16_t lba, uint8_t *buffer);
int disk_write(uint16_t lba, const uint8_t *buffer);

#endif // DISK_H
