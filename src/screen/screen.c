#include "screen.h"
#include "../drivers/graphics/graphics.h"
#include <stdlib.h>
#include <string.h>

static const uint16_t screen_sizes[4][2] = {
    {320, 240},   // SCREEN_SIZE_SMALL
    {640, 480},   // SCREEN_SIZE_MEDIUM
    {1024, 768},  // SCREEN_SIZE_LARGE
    {1920, 1080}  // SCREEN_SIZE_XLARGE
};

Screen* screen_init(void) {
    Screen* screen = (Screen*)malloc(sizeof(Screen));
    if (screen == NULL) {
        return NULL;
    }
    
    screen_detect_size(screen);
    return screen;
}

void screen_detect_size(Screen* screen) {
    // Get actual graphics info instead of random
    graphics_info_t* gfx = graphics_get_info();
    
    if (gfx && gfx->initialized) {
        screen->width = gfx->width;
        screen->height = gfx->height;
        
        // Determine size type based on resolution
        if (screen->width <= 320) {
            screen->size_type = SCREEN_SIZE_SMALL;
        } else if (screen->width <= 640) {
            screen->size_type = SCREEN_SIZE_MEDIUM;
        } else if (screen->width <= 1024) {
            screen->size_type = SCREEN_SIZE_LARGE;
        } else {
            screen->size_type = SCREEN_SIZE_XLARGE;
        }
    } else {
        // Fallback to default if graphics not initialized
        screen->width = 1024;
        screen->height = 768;
        screen->size_type = SCREEN_SIZE_LARGE;
    }
}

const char* screen_get_size_name(const Screen* screen) {
    static const char* size_names[] = {
        "Small", "Medium", "Large", "Extra Large"
    };
    
    if (screen->size_type < 4) {
        return size_names[screen->size_type];
    }
    return "Unknown";
}

bool screen_is_wider_than_tall(const Screen* screen) {
    return screen->width > screen->height;
}

uint32_t screen_get_total_pixels(const Screen* screen) {
    return (uint32_t)screen->width * (uint32_t)screen->height;
}