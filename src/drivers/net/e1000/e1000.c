#include "e1000.h"
#include <string.h>

// Base address for the E1000 device
static volatile void* e1000_base;

// Transmit and receive descriptor structs
struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
};

struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
};

// Descriptor arrays and buffers
#define E1000_NUM_TX_DESC 64
#define E1000_NUM_RX_DESC 64

static struct e1000_tx_desc tx_descs[E1000_NUM_TX_DESC] __attribute__((aligned(16)));
static struct e1000_rx_desc rx_descs[E1000_NUM_RX_DESC] __attribute__((aligned(16)));
static void* tx_buffers[E1000_NUM_TX_DESC];
static void* rx_buffers[E1000_NUM_RX_DESC];

// Define E1000 register offsets
#define REG_RDBAL    0x2800
#define REG_RDBAH    0x2804
#define REG_RDLEN    0x2808
#define REG_RDH      0x2810
#define REG_RDT      0x2818
#define REG_RCTL     0x1000

#define REG_TDBAL    0x3800
#define REG_TDBAH    0x3804
#define REG_TDLEN    0x3808
#define REG_TDH      0x3810
#define REG_TDT      0x3818
#define REG_TCTL     0x1000

// Define some bitmasks for the registers
#define E1000_RCTL_EN        0x00000002
#define E1000_RCTL_SBP       0x00000001
#define E1000_RCTL_UPE       0x00000004
#define E1000_RCTL_MPE       0x00000008
#define E1000_RCTL_LBM_NONE  0x00000000
#define E1000_RCTL_BAM       0x00008000
#define E1000_RCTL_SECRC     0x00004000
#define E1000_RCTL_BSIZE_2048 0x00000000

#define E1000_TCTL_EN        0x00000002
#define E1000_TCTL_PSP       0x00000008
#define E1000_TCTL_CT        0x00000FF0
#define E1000_TCTL_COLD      0x003FF000

static void e1000_init_rx(void);
static void e1000_init_tx(void);

static inline void e1000_write_reg(uint16_t reg, uint32_t value) {
    *(volatile uint32_t*)(uintptr_t)(e1000_base + reg) = value;
}

static inline uint32_t e1000_read_reg(uint16_t reg) {
    return *(volatile uint32_t*)(uintptr_t)(e1000_base + reg);
}

void e1000_init(uint32_t bar) {
    e1000_base = (void*)(uintptr_t)bar;
    e1000_init_rx();
    e1000_init_tx();
}

bool e1000_send_packet(const void* data, uint16_t length) {
    // Implement send packet logic here
    // You will need to set up the TX descriptors and initiate the transmission
    return true;
}

bool e1000_receive_packet(void* buffer, uint16_t* length) {
    // Implement receive packet logic here
    // You will need to check the RX descriptors for received packets and copy data to the buffer
    return true;
}

static void e1000_init_rx(void) {
    // Initialize receive descriptors
    memset(rx_descs, 0, sizeof(rx_descs));

    // Initialize receive buffers
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_buffers[i] = (void*)(uintptr_t)(0x200000 + i * 2048); // Use fixed memory addresses for buffers
        rx_descs[i].addr = (uint64_t)(uintptr_t)rx_buffers[i];
        rx_descs[i].status = 0;
    }

    // Set up the receive descriptor ring buffer
    e1000_write_reg(REG_RDBAL, (uint32_t)(uintptr_t)rx_descs);
    e1000_write_reg(REG_RDBAH, (uint32_t)((uintptr_t)rx_descs >> 32));
    e1000_write_reg(REG_RDLEN, E1000_NUM_RX_DESC * sizeof(struct e1000_rx_desc));

    // Set head and tail pointers
    e1000_write_reg(REG_RDH, 0);
    e1000_write_reg(REG_RDT, E1000_NUM_RX_DESC - 1);

    // Set up the receive control register
    uint32_t rctl = E1000_RCTL_EN | E1000_RCTL_SBP | E1000_RCTL_UPE | E1000_RCTL_MPE |
                    E1000_RCTL_LBM_NONE | E1000_RCTL_BAM | E1000_RCTL_SECRC |
                    E1000_RCTL_BSIZE_2048;
    e1000_write_reg(REG_RCTL, rctl);
}

static void e1000_init_tx(void) {
    // Initialize transmit descriptors
    memset(tx_descs, 0, sizeof(tx_descs));

    // Set up the transmit descriptor ring buffer
    e1000_write_reg(REG_TDBAL, (uint32_t)(uintptr_t)tx_descs);
    e1000_write_reg(REG_TDBAH, (uint32_t)((uintptr_t)tx_descs >> 32));
    e1000_write_reg(REG_TDLEN, E1000_NUM_TX_DESC * sizeof(struct e1000_tx_desc));

    // Set head and tail pointers
    e1000_write_reg(REG_TDH, 0);
    e1000_write_reg(REG_TDT, 0);

    // Set up the transmit control register
    uint32_t tctl = E1000_TCTL_EN | E1000_TCTL_PSP | (15 << 4) | (64 << 12);
    e1000_write_reg(REG_TCTL, tctl);
}
