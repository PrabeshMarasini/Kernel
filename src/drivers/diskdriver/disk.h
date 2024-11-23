#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define ATA_PRIMARY_IO 0x1F0      
#define ATA_PRIMARY_CTRL 0x3F6    

#define ATA_CMD_READ  0x20        
#define ATA_CMD_WRITE 0x30        
#define ATA_SECTOR_SIZE 512       

int ata_read_sector(uint32_t lba, uint8_t *buffer);
int ata_write_sector(uint32_t lba, const uint8_t *buffer);
void ata_wait_for_drive_ready(void);  
void ata_wait_for_drive_ready_with_timeout(void); 
void ata_send_command(uint8_t command);  
void ata_select_drive(uint8_t drive);   
void print_str(const char *str);  
void itoaa(int num, char* str, int base);

#endif
