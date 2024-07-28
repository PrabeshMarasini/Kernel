#ifndef IO_H
#define IO_H

// Define I/O port access functions
static inline unsigned char inb(unsigned short port) {
    unsigned char data;
    __asm__ __volatile__("inb %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

#endif // IO_H
