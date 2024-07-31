#ifndef E1000_H
#define E1000_H

#include <stdint.h> // Ensure to include this for standard types
#include <stdbool.h> // Ensure to include this for bool type

void e1000_init(uint32_t bar);
bool e1000_send_packet(const void* data, uint16_t length); // Match definition
bool e1000_receive_packet(void* buffer, uint16_t* length); // Match definition

#endif // E1000_H
