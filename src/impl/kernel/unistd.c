// unistd.c
#include "unistd.h"

// A basic delay function to create a busy wait
void delay_loop(unsigned int iterations) {
    volatile int count;
    for (unsigned int i = 0; i < iterations; i++) {
        count++;  // Just waste some CPU cycles
    }
}

void sleep(unsigned int seconds) {
    for (unsigned int i = 0; i < seconds; i++) {
        usleep(1000000);  // Sleep for 1 million microseconds (1 second)
    }
}

void usleep(unsigned int microseconds) {
    // Assuming 1 microsecond takes roughly 1 CPU cycle, adjust the count for your CPU speed
    // This is a simple approximation. You may need to fine-tune this based on your CPU's speed.
    delay_loop(microseconds * 10);  // Multiply by a factor to account for CPU speed
}

void yield(void) {
    // Yield control to the scheduler (if implemented)
    // This could be an empty implementation or a context switch call
}
