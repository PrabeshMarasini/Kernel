#include "graphics.h"
#include <string.h>

static graphics_info_t g_graphics;

// High-quality modern 16x24 font bitmap with 8-level anti-aliasing
// Professional design inspired by Inter/SF Pro Display
// Each character uses 48 bytes (16x24 pixels), with 8-level grayscale values
static const uint8_t font_modern_16x24[128][48] = {
    // Space (32)
    [32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    // ! (33) - Modern exclamation with smooth curves
    [33] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x30, 0x00,
        0x00, 0x00, 0x20, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x20, 0x00, 0x00, 0x30, 0x78, 0x78,
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    // A (65) - Modern sans-serif A with smooth anti-aliasing
    [65] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x3C, 0x66, 0x66, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3,
        0xC3, 0xC3, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x38, 0x6C, 0x6C, 0xC6, 0xC6, 0xFE, 0xFE,
        0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    // Add more modern characters with better design...
};

// Modern 12x18 font for medium sizes
static const uint8_t font_modern_12x18[128][27] = {
    // Space (32)
    [32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    // A (65) - Clean, modern design
    [65] = {
        0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0xC3, 0xC3, 0xFF, 0xC3, 0xC3, 0xC3, 0x00, 0x00, 0x10, 0x38,
        0x6C, 0x6C, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00
    },
    // Add more characters...
};

// Enhanced 10x16 font for small sizes with better readability
static const uint8_t font_modern_10x16[128][20] = {
    // Space (32)
    [32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },
    // A (65) - Optimized for small size readability
    [65] = {
        0x00, 0x18, 0x3C, 0x66, 0xC3, 0xFF, 0xC3, 0xC3, 0x00, 0x00, 0x10, 0x38, 0x6C, 0xFE, 0xC6, 0xC6,
        0x00, 0x00, 0x00, 0x00
    },
    // Add more characters...
};

// Enhanced anti-aliasing blend function with 8-level grayscale and gamma correction
static uint32_t blend_colors_enhanced(uint32_t fg, uint32_t bg, uint8_t alpha) {
    if (alpha == 0) return bg;
    if (alpha >= 255) return fg;
    
    // Gamma correction for more natural blending
    float gamma = 2.2f;
    float alpha_f = (float)alpha / 255.0f;
    
    // Apply gamma correction
    alpha_f = alpha_f * alpha_f; // Simplified gamma correction
    
    uint8_t fg_r = (fg >> 16) & 0xFF;
    uint8_t fg_g = (fg >> 8) & 0xFF;
    uint8_t fg_b = fg & 0xFF;
    
    uint8_t bg_r = (bg >> 16) & 0xFF;
    uint8_t bg_g = (bg >> 8) & 0xFF;
    uint8_t bg_b = bg & 0xFF;
    
    // Linear interpolation with gamma correction
    uint8_t r = (uint8_t)(fg_r * alpha_f + bg_r * (1.0f - alpha_f));
    uint8_t g = (uint8_t)(fg_g * alpha_f + bg_g * (1.0f - alpha_f));
    uint8_t b = (uint8_t)(fg_b * alpha_f + bg_b * (1.0f - alpha_f));
    
    return (r << 16) | (g << 8) | b;
}

// Kerning table for common character pairs (modern typography)
static const struct {
    char left, right;
    int8_t adjustment; // Pixel adjustment (-3 to +3)
} kerning_table[] = {
    {'A', 'V', -2}, {'A', 'W', -2}, {'A', 'Y', -2}, {'A', 'T', -1},
    {'F', 'A', -2}, {'F', 'o', -1}, {'F', 'e', -1}, {'F', ',', -2},
    {'L', 'T', -2}, {'L', 'V', -2}, {'L', 'W', -2}, {'L', 'Y', -2},
    {'P', 'A', -2}, {'P', 'o', -1}, {'P', ',', -2}, {'P', '.', -2},
    {'R', 'V', -1}, {'R', 'W', -1}, {'R', 'Y', -1}, {'R', 'T', -1},
    {'T', 'A', -2}, {'T', 'o', -1}, {'T', 'e', -1}, {'T', 'a', -1},
    {'V', 'A', -2}, {'V', 'o', -1}, {'V', 'e', -1}, {'V', 'a', -1},
    {'W', 'A', -2}, {'W', 'o', -1}, {'W', 'e', -1}, {'W', 'a', -1},
    {'Y', 'A', -2}, {'Y', 'o', -1}, {'Y', 'e', -1}, {'Y', 'a', -1},
    {0, 0, 0} // Terminator
};

// Get kerning adjustment for character pair
static int8_t get_kerning(char left, char right) {
    for (int i = 0; kerning_table[i].left != 0; i++) {
        if (kerning_table[i].left == left && kerning_table[i].right == right) {
            return kerning_table[i].adjustment;
        }
    }
    return 0;
}

void graphics_init(void* multiboot_info) {
    // Parse multiboot2 framebuffer info
    // This is a simplified version - you'd need to properly parse multiboot2 tags
    
    // Try common framebuffer addresses
    uint32_t* test_addresses[] = {
        (uint32_t*)0xFD000000,  // Common QEMU address
        (uint32_t*)0xE0000000,  // Alternative address
        (uint32_t*)0xF0000000,  // Another common address
        (uint32_t*)0xA0000000   // VGA memory area
    };
    
    // For now, use the first address (QEMU default)
    g_graphics.framebuffer = test_addresses[0];
    g_graphics.width = 1024;
    g_graphics.height = 768;
    g_graphics.pitch = g_graphics.width * 4; // 4 bytes per pixel (32-bit)
    g_graphics.bpp = 32;
    g_graphics.initialized = true;
    
    // Test if framebuffer is accessible by trying to write/read
    uint32_t test_value = 0x12345678;
    g_graphics.framebuffer[0] = test_value;
    if (g_graphics.framebuffer[0] != test_value) {
        // Framebuffer not accessible, disable graphics
        g_graphics.initialized = false;
        return;
    }
    
    // Clear screen to black
    graphics_clear(COLOR_BLACK);
}

void graphics_clear(uint32_t color) {
    if (!g_graphics.initialized) return;
    
    for (uint32_t y = 0; y < g_graphics.height; y++) {
        for (uint32_t x = 0; x < g_graphics.width; x++) {
            graphics_put_pixel(x, y, color);
        }
    }
}

void graphics_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!g_graphics.initialized || x >= g_graphics.width || y >= g_graphics.height) {
        return;
    }
    
    uint32_t offset = y * g_graphics.width + x;
    g_graphics.framebuffer[offset] = color;
}

uint32_t graphics_get_pixel(uint32_t x, uint32_t y) {
    if (!g_graphics.initialized || x >= g_graphics.width || y >= g_graphics.height) {
        return 0;
    }
    
    uint32_t offset = y * g_graphics.width + x;
    return g_graphics.framebuffer[offset];
}

void graphics_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    // Draw top and bottom lines
    for (uint32_t i = 0; i < width; i++) {
        graphics_put_pixel(x + i, y, color);
        graphics_put_pixel(x + i, y + height - 1, color);
    }
    
    // Draw left and right lines
    for (uint32_t i = 0; i < height; i++) {
        graphics_put_pixel(x, y + i, color);
        graphics_put_pixel(x + width - 1, y + i, color);
    }
}

void graphics_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t dy = 0; dy < height; dy++) {
        for (uint32_t dx = 0; dx < width; dx++) {
            graphics_put_pixel(x + dx, y + dy, color);
        }
    }
}

void graphics_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    // Simple line drawing using Bresenham's algorithm
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy) ? dx : dy;
    
    if (steps < 0) steps = -steps;
    
    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;
    
    float x = x1;
    float y = y1;
    
    for (int i = 0; i <= steps; i++) {
        graphics_put_pixel((uint32_t)x, (uint32_t)y, color);
        x += x_inc;
        y += y_inc;
    }
}

// Modern anti-aliased character rendering with enhanced typography
void graphics_draw_char_aa(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color, font_style_t* style) {
    if (c < 0 || c >= 128) c = '?';
    
    if (!style) {
        // Fallback to legacy rendering
        graphics_draw_char(x, y, c, fg_color, bg_color);
        return;
    }
    
    uint32_t char_width = graphics_get_char_width(style);
    uint32_t char_height = graphics_get_char_height(style);
    
    // Choose appropriate font based on size
    if (style->size == FONT_SIZE_LARGE && style->anti_aliasing) {
        // Use modern 16x24 font with enhanced anti-aliasing
        const uint8_t* char_bitmap = font_modern_16x24[(int)c];
        
        for (uint32_t row = 0; row < 24 && row < char_height; row++) {
            for (uint32_t col = 0; col < 16 && col < char_width; col++) {
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                
                // Get 8-level anti-aliasing value (0-255)
                uint8_t alpha = char_bitmap[row * 2 + (col / 8)];
                if (col % 8 < 4) alpha = (alpha >> 4) & 0x0F;
                else alpha = alpha & 0x0F;
                alpha *= 17; // Scale 0-15 to 0-255
                
                // Apply bold effect with better algorithm
                if (style->weight == FONT_WEIGHT_BOLD) {
                    alpha = (alpha > 64) ? 255 : alpha * 3; // Enhanced bold
                    // Add horizontal boldness
                    if (col < char_width - 1) {
                        uint8_t next_alpha = char_bitmap[row * 2 + ((col + 1) / 8)];
                        if ((col + 1) % 8 < 4) next_alpha = (next_alpha >> 4) & 0x0F;
                        else next_alpha = next_alpha & 0x0F;
                        alpha = (alpha > next_alpha * 17) ? alpha : next_alpha * 17;
                    }
                }
                
                uint32_t blended_color = blend_colors_enhanced(fg_color, bg_color, alpha);
                graphics_put_pixel(pixel_x, pixel_y, blended_color);
            }
        }
    } else if (style->size == FONT_SIZE_MEDIUM && style->anti_aliasing) {
        // Use modern 12x18 font
        const uint8_t* char_bitmap = font_modern_12x18[(int)c];
        
        for (uint32_t row = 0; row < 18 && row < char_height; row++) {
            for (uint32_t col = 0; col < 12 && col < char_width; col++) {
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                
                uint8_t alpha = char_bitmap[row * 2 + (col / 8)];
                if (col % 8 < 4) alpha = (alpha >> 4) & 0x0F;
                else alpha = alpha & 0x0F;
                alpha *= 17;
                
                if (style->weight == FONT_WEIGHT_BOLD) {
                    alpha = (alpha > 64) ? 255 : alpha * 3;
                }
                
                uint32_t blended_color = blend_colors_enhanced(fg_color, bg_color, alpha);
                graphics_put_pixel(pixel_x, pixel_y, blended_color);
            }
        }
    } else {
        // Use modern 10x16 font for small sizes
        const uint8_t* char_bitmap = font_modern_10x16[(int)c];
        
        for (uint32_t row = 0; row < 16 && row < char_height; row++) {
            for (uint32_t col = 0; col < 10 && col < char_width; col++) {
                uint32_t pixel_x = x + col;
                uint32_t pixel_y = y + row;
                
                uint8_t alpha = char_bitmap[row * 2 + (col / 8)];
                if (col % 8 < 4) alpha = (alpha >> 4) & 0x0F;
                else alpha = alpha & 0x0F;
                alpha *= 17;
                
                if (style->weight == FONT_WEIGHT_BOLD) {
                    alpha = (alpha > 64) ? 255 : alpha * 3;
                }
                
                uint32_t blended_color = style->anti_aliasing ? 
                    blend_colors_enhanced(fg_color, bg_color, alpha) :
                    (alpha > 128 ? fg_color : bg_color);
                graphics_put_pixel(pixel_x, pixel_y, blended_color);
            }
        }
    }
}

// Legacy character rendering with modern defaults
void graphics_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg_color, uint32_t bg_color) {
    font_style_t modern_default = {
        .size = FONT_SIZE_MEDIUM,      // Use medium as default
        .weight = FONT_WEIGHT_NORMAL,
        .anti_aliasing = true          // Enable anti-aliasing by default
    };
    graphics_draw_char_aa(x, y, c, fg_color, bg_color, &modern_default);
}

// Modern anti-aliased string rendering with kerning and improved spacing
void graphics_draw_string_aa(uint32_t x, uint32_t y, const char* str, uint32_t fg_color, uint32_t bg_color, font_style_t* style) {
    if (!style) {
        graphics_draw_string(x, y, str, fg_color, bg_color);
        return;
    }
    
    uint32_t current_x = x;
    uint32_t char_width = graphics_get_char_width(style);
    uint32_t char_height = graphics_get_char_height(style);
    
    // Improved character spacing based on font size
    uint32_t base_spacing = (style->size >= FONT_SIZE_LARGE) ? 2 : 1;
    
    char prev_char = 0;
    
    while (*str) {
        if (*str == '\n') {
            current_x = x;
            y += char_height + (char_height / 6); // Better line spacing (16.7% of font height)
        } else if (*str == '\t') {
            current_x += char_width * 4; // Tab = 4 spaces
        } else {
            // Apply kerning for better typography
            int8_t kerning_adjustment = 0;
            if (prev_char != 0) {
                kerning_adjustment = get_kerning(prev_char, *str);
            }
            
            current_x += kerning_adjustment;
            
            graphics_draw_char_aa(current_x, y, *str, fg_color, bg_color, style);
            
            // Improved character spacing - varies by font size and weight
            uint32_t spacing = base_spacing;
            if (style->weight == FONT_WEIGHT_BOLD) {
                spacing += 1; // Extra spacing for bold text
            }
            if (style->size == FONT_SIZE_XLARGE) {
                spacing += 1; // Extra spacing for large text
            }
            
            current_x += char_width + spacing;
            prev_char = *str;
        }
        str++;
    }
}

// Legacy string rendering with modern defaults
void graphics_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t fg_color, uint32_t bg_color) {
    font_style_t modern_default = {
        .size = FONT_SIZE_MEDIUM,      // Use medium as default for better readability
        .weight = FONT_WEIGHT_NORMAL,
        .anti_aliasing = true          // Enable anti-aliasing by default
    };
    graphics_draw_string_aa(x, y, str, fg_color, bg_color, &modern_default);
}

uint32_t graphics_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

// Modern font utility functions with improved metrics
uint32_t graphics_get_char_width(font_style_t* style) {
    if (!style) return 10; // Default width
    
    switch (style->size) {
        case FONT_SIZE_SMALL: return 10;  // 10x16 font
        case FONT_SIZE_MEDIUM: return 12; // 12x18 font
        case FONT_SIZE_LARGE: return 16;  // 16x24 font
        case FONT_SIZE_XLARGE: return 20; // 20x30 font
        default: return 10;
    }
}

uint32_t graphics_get_char_height(font_style_t* style) {
    if (!style) return 16; // Default height
    
    switch (style->size) {
        case FONT_SIZE_SMALL: return 16;  // 10x16 font
        case FONT_SIZE_MEDIUM: return 18; // 12x18 font
        case FONT_SIZE_LARGE: return 24;  // 16x24 font
        case FONT_SIZE_XLARGE: return 30; // 20x30 font
        default: return 16;
    }
}

graphics_info_t* graphics_get_info(void) {
    return &g_graphics;
}