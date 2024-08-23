#include "screen.h"
#include <stdlib.h>
#include <string.h>

// Define screen dimensions for each size
static const uint16_t screen_sizes[4][2] = {
    {320, 240},   // SCREEN_SIZE_SMALL
    {640, 480},   // SCREEN_SIZE_MEDIUM
    {1024, 768},  // SCREEN_SIZE_LARGE
    {1920, 1080}  // SCREEN_SIZE_XLARGE
};

// Function to initialize the screen
Screen* screen_init(void) {
    Screen* screen = (Screen*)malloc(sizeof(Screen));
    if (screen == NULL) {
        return NULL;
    }
    
    screen_detect_size(screen);
    return screen;
}

// Function to detect screen size (simulated for this example)
void screen_detect_size(Screen* screen) {
    // In a real implementation, you would use hardware-specific methods
    // to detect the actual screen size. For this example, we'll simulate
    // detection by choosing a random size.
    
    uint8_t detected_size = rand() % 4;
    screen->width = screen_sizes[detected_size][0];
    screen->height = screen_sizes[detected_size][1];
    screen->size_type = detected_size;
}

// Function to get the name of the screen size
const char* screen_get_size_name(const Screen* screen) {
    static const char* size_names[] = {
        "Small", "Medium", "Large", "Extra Large"
    };
    
    if (screen->size_type < 4) {
        return size_names[screen->size_type];
    }
    return "Unknown";
}

// Function to check if the screen is wider than it is tall
bool screen_is_wider_than_tall(const Screen* screen) {
    return screen->width > screen->height;
}

// Function to calculate the total number of pixels
uint32_t screen_get_total_pixels(const Screen* screen) {
    return (uint32_t)screen->width * (uint32_t)screen->height;
}