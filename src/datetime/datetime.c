#include "datetime.h"
#include "print.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint8_t read_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

struct tm get_rtc_time() {
    struct tm time;
    time.tm_sec = bcd_to_bin(read_rtc_register(0x00));
    time.tm_min = bcd_to_bin(read_rtc_register(0x02));
    time.tm_hour = bcd_to_bin(read_rtc_register(0x04));
    time.tm_mday = bcd_to_bin(read_rtc_register(0x07));
    time.tm_mon = bcd_to_bin(read_rtc_register(0x08)) - 1;
    time.tm_year = bcd_to_bin(read_rtc_register(0x09)) + 100; // Years since 1900
    return time;
}

void adjust_time_for_nepal(struct tm* time) {
    int nepal_minutes_offset = 345; // Nepal is UTC+5:45
    int total_minutes = time->tm_hour * 60 + time->tm_min + nepal_minutes_offset;
    time->tm_hour = (total_minutes / 60) % 24;
    time->tm_min = total_minutes % 60;
    // Adjust day, month, and year if needed (simplified, not handling all edge cases)
    if (time->tm_hour >= 24) {
        time->tm_hour -= 24;
        time->tm_mday++;
        // You would need additional logic to handle month and year rollover
    }
}

int print_num(char* buffer, int num, int width) {
    int n = num;
    int len = 0;
    while (n > 0) {
        len++;
        n /= 10;
    }
    for (int i = len; i < width; i++) {
        *buffer++ = '0';
    }
    char temp[10];
    int idx = 0;
    do {
        temp[idx++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0);
    for (int i = idx - 1; i >= 0; i--) {
        *buffer++ = temp[i];
    }
    return width > len ? width : len;
}

void simple_snprintf(char* buffer, int year, int month, int day, int hour, int minute, int second) {
    char* p = buffer;
    p += print_num(p, year, 4);
    *p++ = '-';
    p += print_num(p, month, 2);
    *p++ = '-';
    p += print_num(p, day, 2);
    *p++ = ' ';
    p += print_num(p, hour, 2);
    *p++ = ':';
    p += print_num(p, minute, 2);
    *p++ = ':';
    p += print_num(p, second, 2);
    *p = '\0';
}

void print_time(const struct tm* time) {
    char buffer[256];
    int year = time->tm_year + 1900;
    int month = time->tm_mon + 1;
    simple_snprintf(buffer, year, month, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
    print_str(buffer);
}