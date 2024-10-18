// unistd.h
#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>

void sleep(unsigned int seconds);
void usleep(unsigned int microseconds);
void yield(void);

#endif // UNISTD_H
