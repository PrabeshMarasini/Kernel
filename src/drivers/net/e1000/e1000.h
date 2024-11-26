#ifndef E1000_H
#define E1000_H

#include <stdint.h> 
#include <stdbool.h> 

void e1000_init(uint32_t bar);
bool e1000_send_packet(const void* data, uint16_t length); 
bool e1000_receive_packet(void* buffer, uint16_t* length); 

#endif // E1000_H
