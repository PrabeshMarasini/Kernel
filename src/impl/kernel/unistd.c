#include "unistd.h"

void delay_loop(unsigned int iterations) {
    volatile int count;
    for (unsigned int i = 0; i < iterations; i++) {
        count++; 
}

void sleep(unsigned int seconds) {
    for (unsigned int i = 0; i < seconds; i++) {
        usleep(1000000);  
    }
}

void usleep(unsigned int microseconds) {
    delay_loop(microseconds * 10);
}
}
