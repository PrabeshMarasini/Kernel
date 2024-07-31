#ifndef PCI_H
#define PCI_H

#include <stdint.h> // Ensure to include this for standard types
#include <stdbool.h>

// Declarations for E1000 driver functions
void e1000_init(uint32_t bar);
bool e1000_send_packet(const void* data, uint16_t length); // Match definition
bool e1000_receive_packet(void* buffer, uint16_t* length); // Match definition

// Declarations for PCI functions
uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

#endif // PCI_H
