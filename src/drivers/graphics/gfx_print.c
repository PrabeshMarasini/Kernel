#include "gfx_print.h"
#include "graphics.h"

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint32_t fg_color = COLOR_WHITE;
static uint32_t bg_color = COLOR_BLACK;
static uint32_t char_width = 16;
static uint32_t char_height = 16;
static uint32_t max_cols = 0;
static uint32_t max_rows = 0;

// Modern terminal font style - clean and readable
static font_style_t terminal_font = {
    .size = FONT_SIZE_MEDIUM,      // 12x18 for excellent readability
    .weight = FONT_WEIGHT_NORMAL,
    .anti_aliasing = true          // Smooth, modern appearance
};

void gfx_print_init(void) {
    // Update character dimensions based on modern font
    char_width = graphics_get_char_width(&terminal_font) + 1; // Add spacing
    char_height = graphics_get_char_height(&terminal_font) + 2; // Add line spacing
    
    graphics_info_t* gfx = graphics_get_info();
    if (gfx && gfx->initialized) {
        max_cols = gfx->width / char_width;
        max_rows = gfx->height / char_height;
    } else {
        max_cols = 64; // fallback for larger font
        max_rows = 48;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    fg_color = COLOR_WHITE;
    bg_color = COLOR_BLACK;
}

void gfx_print_clear(void) {
    graphics_clear(bg_color);
    cursor_x = 0;
    cursor_y = 0;
}

void gfx_print_char(char c) {
    if (c == '\n') {
        gfx_print_newline();
        return;
    }
    
    if (c == '\b') {
        // Backspace
        if (cursor_x > 0) {
            cursor_x--;
            graphics_draw_char_aa(cursor_x * char_width, cursor_y * char_height, ' ', fg_color, bg_color, &terminal_font);
        }
        return;
    }
    
    // Check if we need to wrap to next line
    if (cursor_x >= max_cols) {
        gfx_print_newline();
    }
    
    // Draw the character with modern anti-aliased font
    graphics_draw_char_aa(cursor_x * char_width, cursor_y * char_height, c, fg_color, bg_color, &terminal_font);
    cursor_x++;
}

void gfx_print_str(const char* str) {
    while (*str) {
        gfx_print_char(*str);
        str++;
    }
}

void gfx_print_set_color(uint32_t new_fg_color, uint32_t new_bg_color) {
    fg_color = new_fg_color;
    bg_color = new_bg_color;
}

void gfx_print_set_cursor(uint32_t x, uint32_t y) {
    if (x < max_cols && y < max_rows) {
        cursor_x = x;
        cursor_y = y;
    }
}

void gfx_print_get_cursor(uint32_t* x, uint32_t* y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

void gfx_print_newline(void) {
    cursor_x = 0;
    cursor_y++;
    
    // Check if we need to scroll
    if (cursor_y >= max_rows) {
        gfx_print_scroll_up();
        cursor_y = max_rows - 1;
    }
}

void gfx_print_scroll_up(void) {
    graphics_info_t* gfx = graphics_get_info();
    if (!gfx || !gfx->initialized) return;
    
    // Move all lines up by one character height
    for (uint32_t y = char_height; y < gfx->height; y++) {
        for (uint32_t x = 0; x < gfx->width; x++) {
            uint32_t pixel = graphics_get_pixel(x, y);
            graphics_put_pixel(x, y - char_height, pixel);
        }
    }
    
    // Clear the bottom line
    graphics_fill_rect(0, gfx->height - char_height, gfx->width, char_height, bg_color);
}