#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

// Font size enumeration - Modern scaling
typedef enum {
    FONT_SIZE_SMALL = 10,   // 10x16 - Excellent for small text
    FONT_SIZE_MEDIUM = 12,  // 12x18 - Perfect for body text
    FONT_SIZE_LARGE = 16,   // 16x24 - Great for headers
    FONT_SIZE_XLARGE = 20   // 20x30 - Bold titles
} font_size_t;

// Font weight enumeration
typedef enum {
    FONT_WEIGHT_NORMAL = 0,
    FONT_WEIGHT_BOLD = 1
} font_weight_t;

// Font style structure
typedef struct {
    font_size_t size;
    font_weight_t weight;
    bool anti_aliasing;
} font_style_t;

typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    bool initialized;
} graphics_info_t;

// Core graphics functions
void graphics_init(void* multiboot_info);
void graphics_clear(uint32_t color);
void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t graphics_get_pixel(uint32_t x, uint32_t y);

// Drawing primitives
void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void graphics_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color);

// Text rendering
void graphics_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color);
void graphics_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg_color, uint32_t bg_color);

// Advanced font rendering
void graphics_draw_char_aa(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color, font_style_t* style);
void graphics_draw_string_aa(uint32_t x, uint32_t y, const char* str, uint32_t fg_color, uint32_t bg_color, font_style_t* style);
uint32_t graphics_get_char_width(font_style_t* style);
uint32_t graphics_get_char_height(font_style_t* style);

// Utility functions
uint32_t graphics_rgb(uint8_t r, uint8_t g, uint8_t b);
graphics_info_t* graphics_get_info(void);

// Color constants
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_RED       0xFF0000
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_YELLOW    0xFFFF00
#define COLOR_CYAN      0x00FFFF
#define COLOR_MAGENTA   0xFF00FF
#define COLOR_GRAY      0x808080
#define COLOR_DARK_GRAY 0x404040

#endif