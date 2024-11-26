#include "pci.h"
#include "io.h"  

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static inline void pci_write_config_address(uint32_t address) {
    outl(PCI_CONFIG_ADDRESS, address);
}

static inline uint32_t pci_read_config_data(void) {
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_config_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)(bus << 16) | (uint32_t)(device << 11) | 
                       (uint32_t)(func << 8) | (offset & 0xFC) | (1 << 31);

    pci_write_config_address(address);
    return (uint16_t)(pci_read_config_data() >> ((offset & 2) * 8));
}

uint32_t pci_read_config_dword(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)(bus << 16) | (uint32_t)(device << 11) | 
                       (uint32_t)(func << 8) | (offset & 0xFC) | (1 << 31);

    pci_write_config_address(address);
    return pci_read_config_data();
}
