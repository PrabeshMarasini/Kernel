#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <stdbool.h>

// Define screen sizes
#define SCREEN_SIZE_SMALL    0
#define SCREEN_SIZE_MEDIUM   1
#define SCREEN_SIZE_LARGE    2
#define SCREEN_SIZE_XLARGE   3

// Screen structure
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t size_type;
} Screen;

// Function prototypes
Screen* screen_init(void);
void screen_detect_size(Screen* screen);
const char* screen_get_size_name(const Screen* screen);
bool screen_is_wider_than_tall(const Screen* screen);
uint32_t screen_get_total_pixels(const Screen* screen);

#endif // SCREEN_H