#ifndef GFX_PRINT_H
#define GFX_PRINT_H

#include <stdint.h>

// Graphics-based printing interface (replaces VGA text mode)
void gfx_print_init(void);
void gfx_print_clear(void);
void gfx_print_char(char c);
void gfx_print_str(const char* str);
void gfx_print_set_color(uint32_t fg_color, uint32_t bg_color);
void gfx_print_set_cursor(uint32_t x, uint32_t y);
void gfx_print_get_cursor(uint32_t* x, uint32_t* y);

// Advanced features
void gfx_print_newline(void);
void gfx_print_scroll_up(void);

#endif