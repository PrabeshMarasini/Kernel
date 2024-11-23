#ifndef IO_H
#define IO_H

static inline unsigned char inb(unsigned short port) {
    unsigned char data;
    __asm__ __volatile__("inb %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ __volatile__("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline unsigned short inw(unsigned short port) {
    unsigned short data;
    __asm__ __volatile__("inw %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void outw(unsigned short port, unsigned short data) {
    __asm__ __volatile__("outw %0, %1" : : "a" (data), "Nd" (port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int data;
    __asm__ __volatile__("inl %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void outl(unsigned short port, unsigned int data) {
    __asm__ __volatile__("outl %0, %1" : : "a" (data), "Nd" (port));
}

#endif 
