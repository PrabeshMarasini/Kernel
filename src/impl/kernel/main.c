#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/graphics/graphics.h"
#include "../drivers/graphics/gfx_print.h"
#include "../datetime/datetime.h"
#include "../calculator/calculator.h"
#include "../snake/snake.h"
#include <string.h>

void run_shell();
void display_welcome_animation_vga();

void fill_screen(char color) {
    // More efficient screen filling - direct VGA memory access
    unsigned char* video_memory = (unsigned char*)0xB8000;
    unsigned char screen_color = PRINT_COLOR_WHITE | (color << 4);
    
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = screen_color;
    }
    print_set_cursor(0, 0);
}

void reset_screen() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_clear();
}

void update_datetime() {
    static char last_buffer[20] = {0};
    struct tm current_time = get_rtc_time();
    adjust_time_for_nepal(&current_time);
    char buffer[20];
    simple_snprintf(buffer, current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday, 
                    current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    
    // Only update if time actually changed
    if (strcmp(buffer, last_buffer) != 0) {
        // Save current cursor position
        int saved_x, saved_y;
        print_get_cursor(&saved_x, &saved_y);
        
        // Update time display
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_set_cursor(80 - strlen(buffer), 0);
        print_str(buffer);
        strcpy(last_buffer, buffer);
        
        // Restore cursor position
        print_set_cursor(saved_x, saved_y);
        print_update_cursor();
    }
}

void display_font_demo() {
    // Try to initialize graphics mode
    graphics_init(0);
    graphics_info_t* gfx = graphics_get_info();
    
    if (gfx && gfx->initialized) {
        // We have graphics! Show off the new fonts
        graphics_clear(COLOR_BLACK);
        gfx_print_init();
        
        // Modern title with enhanced typography
        font_style_t title_style = {
            .size = FONT_SIZE_XLARGE,
            .weight = FONT_WEIGHT_BOLD,
            .anti_aliasing = true
        };
        
        graphics_draw_string_aa(50, 50, "SecureOS v2.0", COLOR_CYAN, COLOR_BLACK, &title_style);
        graphics_draw_string_aa(50, 90, "Modern Typography System", COLOR_WHITE, COLOR_BLACK, &title_style);
        
        // Clean subtitle with perfect spacing
        font_style_t subtitle_style = {
            .size = FONT_SIZE_LARGE,
            .weight = FONT_WEIGHT_NORMAL,
            .anti_aliasing = true
        };
        
        graphics_draw_string_aa(50, 150, "Enhanced Anti-Aliasing & Kerning", COLOR_GREEN, COLOR_BLACK, &subtitle_style);
        
        // Typography demonstration with modern fonts
        font_style_t demo_styles[] = {
            {FONT_SIZE_SMALL, FONT_WEIGHT_NORMAL, true},
            {FONT_SIZE_MEDIUM, FONT_WEIGHT_NORMAL, true},
            {FONT_SIZE_LARGE, FONT_WEIGHT_NORMAL, true},
            {FONT_SIZE_XLARGE, FONT_WEIGHT_NORMAL, true}
        };
        
        const char* demo_texts[] = {
            "Small Text (10x16) - Crisp & Clear",
            "Medium Text (12x18) - Perfect Readability", 
            "Large Text (16x24) - Bold Headlines",
            "Extra Large (20x30) - Impact Titles"
        };
        uint32_t colors[] = {COLOR_YELLOW, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE};
        
        for (int i = 0; i < 4; i++) {
            graphics_draw_string_aa(50, 210 + i * 45, demo_texts[i], colors[i], COLOR_BLACK, &demo_styles[i]);
        }
        
        // Enhanced bold demonstration with kerning
        font_style_t bold_style = {
            .size = FONT_SIZE_LARGE,
            .weight = FONT_WEIGHT_BOLD,
            .anti_aliasing = true
        };
        
        graphics_draw_string_aa(50, 420, "Bold Typography with Kerning", COLOR_RED, COLOR_BLACK, &bold_style);
        
        // Show comparison
        font_style_t normal_style = {
            .size = FONT_SIZE_MEDIUM,
            .weight = FONT_WEIGHT_NORMAL, 
            .anti_aliasing = true
        };
        
        graphics_draw_string_aa(50, 460, "AV WA TO - Kerned pairs look perfect!", COLOR_GRAY, COLOR_BLACK, &normal_style);
        
        // Wait for a moment to show the demo
        for (volatile int k = 0; k < 200000000; k++) {}
        
        // Switch to modern graphics-based terminal
        gfx_print_clear();
        gfx_print_set_color(COLOR_WHITE, COLOR_BLACK);
        gfx_print_str("SecureOS v2.0 - Modern Graphics Mode\n");
        gfx_print_str("Enhanced typography with anti-aliasing & kerning!\n\n");
        
    } else {
        // Fallback to VGA text mode
        display_welcome_animation_vga();
    }
}

void display_welcome_animation() {
    // Check if we have graphics mode available
    graphics_info_t* gfx = graphics_get_info();
    
    if (gfx && gfx->initialized) {
        display_font_demo();
    } else {
        display_welcome_animation_vga();
    }
}

void display_welcome_animation_vga() {
    print_set_color(PRINT_COLOR_CYAN, PRINT_COLOR_BLACK);
    print_clear();
    
    const char* welcome = "WELCOME TO SECURE OS v2.0";
    int welcome_len = strlen(welcome);
    int start_x = (80 - welcome_len) / 2;
    int start_y = 12; 

    print_set_cursor(start_x, start_y);
    print_str(welcome);

    for (int i = 0; i < 5; i++) {
        print_set_cursor(start_x + welcome_len, start_y);
        print_str("...");
        
        for (volatile int k = 0; k < 20000000; k++) {}
        
        print_set_cursor(start_x + welcome_len, start_y);
        print_str("   ");
       
        for (volatile int k = 0; k < 10000000; k++) {}
    }
    for (volatile int k = 0; k < 100000000; k++) {}

    print_clear();
}

void initialize_kernel_interface() {
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    fill_screen(PRINT_COLOR_BLUE);
    for (int x = 0; x < 80; x++) {
        print_set_cursor(x, 0);
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
        print_char(' ');
    }
    print_set_cursor(0, 0);
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    print_str("SecureOS Kernel v2.0 - Cybersecurity Research Platform");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    print_set_cursor(0, 1);
    print_str(">");
    
    // Ensure cursor is properly positioned and updated
    print_set_cursor(1, 1);
    print_update_cursor();
}

void kernel_main() {
    static int first_run = 1;

    if (first_run) {
        display_welcome_animation();
        first_run = 0;
    }

    // Ensure we're in proper VGA text mode
    graphics_info_t* gfx = graphics_get_info();
    if (gfx && gfx->initialized) {
        gfx->initialized = false;
    }
    
    initialize_kernel_interface();
    init_keyboard();

    int cursor_x = 1;
    int cursor_y = 1;
    int cursor_position = 0; // Current position in buffer
    char buffer[128];
    int buffer_index = 0;
    const char shell_command[] = "shell";
    const char calculator_command[] = "calculator";
    const char sci_calc_command[] = "scicalc";
    const char snake_command[] = "snake";

    print_set_cursor(cursor_x, cursor_y);
    print_enable_cursor(14, 15);
    print_update_cursor();

    int update_counter = 0;

    while (1) {
        // Update datetime less frequently
        if (update_counter == 0) {
            update_datetime();
        }
        update_counter = (update_counter + 1) % 5000; // Reduced frequency

        unsigned char c = keyboard_get_char();
        if (c != 0) {
            print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
            
            if (c == '\b') {
                if (cursor_position > 0) {
                    // Remove character at cursor position
                    for (int i = cursor_position - 1; i < buffer_index - 1; i++) {
                        buffer[i] = buffer[i + 1];
                    }
                    buffer_index--;
                    cursor_position--;
                    
                    // Redraw the line from prompt position
                    print_set_cursor(1, cursor_y);
                    for (int i = 0; i < buffer_index; i++) {
                        print_char(buffer[i]);
                    }
                    print_char(' '); // Clear the last character
                    
                    // Update cursor position
                    cursor_x = 1 + cursor_position;
                    print_set_cursor(cursor_x, cursor_y);
                    print_update_cursor();
                }
            } else if (c == '\n') {
                buffer[buffer_index] = '\0';
                if (strcmp(buffer, shell_command) == 0) {
                    reset_screen();
                    run_shell();
                    initialize_kernel_interface();
                    cursor_y = 1;
                } else if (strcmp(buffer, calculator_command) == 0) {
                    reset_screen();
                    run_calculator();
                    initialize_kernel_interface();
                    cursor_y = 1;
                } else if (strcmp(buffer, sci_calc_command) == 0) {
                    reset_screen();
                    run_scientific_calculator();
                    initialize_kernel_interface();
                    cursor_y = 1;
                } else if (strcmp(buffer, snake_command) == 0) {
                    reset_screen();
                    run_snake_game();
                    initialize_kernel_interface();
                    cursor_y = 1;
                }
                buffer_index = 0;
                cursor_position = 0;
                cursor_y++;
                cursor_x = 0;
                print_set_cursor(cursor_x, cursor_y);
                print_str(">");
                cursor_x = 1;
                print_set_cursor(cursor_x, cursor_y);
                print_update_cursor();
            } else if (c == NAV_LEFT_ARROW) {
                if (cursor_position > 0) {
                    cursor_position--;
                    cursor_x--;
                    print_set_cursor(cursor_x, cursor_y);
                }
            } else if (c == NAV_RIGHT_ARROW) {
                if (cursor_position < buffer_index) {
                    cursor_position++;
                    cursor_x++;
                    print_set_cursor(cursor_x, cursor_y);
                }
            } else if (c == NAV_HOME_KEY) {
                cursor_position = 0;
                cursor_x = 1;
                print_set_cursor(cursor_x, cursor_y);
            } else if (c == NAV_END_KEY) {
                cursor_position = buffer_index;
                cursor_x = 1 + cursor_position;
                print_set_cursor(cursor_x, cursor_y);
            } else if (c == NAV_UP_ARROW || c == NAV_DOWN_ARROW || c == NAV_PAGE_UP || c == NAV_PAGE_DOWN) {
                // Ignore other navigation keys in main interface
            } else if (c > 127) {
                // Unknown navigation key with high bit set - ignore it
            } else if (cursor_x < 79 && c >= 32 && c <= 126) {
                // Insert character at cursor position
                if (buffer_index < 127) { // Leave space for null terminator
                    for (int i = buffer_index; i > cursor_position; i--) {
                        buffer[i] = buffer[i - 1];
                    }
                    buffer[cursor_position] = c;
                    buffer_index++;
                    cursor_position++;
                    
                    // Redraw the line from prompt position
                    print_set_cursor(1, cursor_y);
                    for (int i = 0; i < buffer_index; i++) {
                        print_char(buffer[i]);
                    }
                    
                    // Update cursor position
                    cursor_x = 1 + cursor_position;
                    print_set_cursor(cursor_x, cursor_y);
                    print_update_cursor();
                }
            }
        } else {
            // Small delay when no input to reduce CPU usage
            for (volatile int i = 0; i < 1000; i++) {}
        }
    }
}