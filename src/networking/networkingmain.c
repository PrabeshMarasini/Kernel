// // kernel/src/network/network_main.c

// #include "../drivers/net/e1000/e1000.h"
// #include "../drivers/net/e1000/pci.h"
// #include "../intf/io.h"
// #include "../intf/print.h"
// #include "../memory/memory.h"
// #include "../intf/string.h"

// #define TEST_PACKET_SIZE 64
// #define TEST_PACKET_COUNT 5

// // Global variables for network interface
// static uint8_t test_packet[TEST_PACKET_SIZE];

// // Initialize network interface
// void network_init() {
//     print("Initializing network interface...\n");
    
//     e1000_init();
    
//     // Additional network setup
//     e1000_configure();
//     e1000_initialize_tx_ring();
//     e1000_initialize_rx_ring();
    
//     // Print confirmation
//     print("Network interface initialized.\n");
// }

// // Send a test packet
// void send_test_packet() {
//     // Prepare a simple test packet
//     memset(test_packet, 0xAB, TEST_PACKET_SIZE); // Fill with pattern 0xAB
//     test_packet[0] = 0x55; // Set first byte to a specific value for identification

//     print("Sending test packet...\n");
//     e1000_send_packet(test_packet, TEST_PACKET_SIZE);

//     // Add a small delay to ensure the packet is sent
//     for (volatile int i = 0; i < 1000000; i++);
// }

// // Receive and print a test packet
// void receive_test_packet() {
//     print("Receiving test packet...\n");
    
//     if (e1000_receive_packet(test_packet, TEST_PACKET_SIZE)) {
//         // Check if packet was received and print the contents
//         print("Packet received: ");
//         for (int i = 0; i < TEST_PACKET_SIZE; i++) {
//             print_hex(test_packet[i]);
//             print(" ");
//         }
//         print("\n");
//     } else {
//         print("No packet received.\n");
//     }
// }

// int main() {
//     print("Starting network test...\n");
    
//     // Initialize network interface
//     network_init();
    
//     // Send and receive test packets
//     for (int i = 0; i < TEST_PACKET_COUNT; i++) {
//         send_test_packet();
//         receive_test_packet();
        
//         // Add a delay between tests
//         for (volatile int j = 0; j < 5000000; j++);
//     }
    
//     print("Network test completed.\n");
    
//     while (1) {
//         // Main loop to keep the kernel running
//     }

//     return 0;
// }
