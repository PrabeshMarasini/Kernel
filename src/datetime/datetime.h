#ifndef DATETIME_H
#define DATETIME_H

#include <stdint.h>

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
};

void adjust_time_for_nepal(struct tm* time);
struct tm get_rtc_time();
void print_time(const struct tm* time);
void simple_snprintf(char* buffer, int year, int month, int day, int hour, int minute, int second);

#endif // DATETIME_H