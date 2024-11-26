#ifndef PCI_H
#define PCI_H

#include <stdint.h> 
#include <stdbool.h>

void e1000_init(uint32_t bar);
bool e1000_send_packet(const void* data, uint16_t length); 
bool e1000_receive_packet(void* buffer, uint16_t* length); 

uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

#endif // PCI_H
