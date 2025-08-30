#include "calculator.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"

// Define limits and constants for fixed-point arithmetic
#define LONG_MAX 2147483647L
#define LONG_MIN (-2147483647L - 1L)
#define PI_FIXED 3142  // π * 1000 (3.142)
#define E_FIXED 2718   // e * 1000 (2.718)

void kernel_main(void);

// Calculator state variables - using fixed-point arithmetic
static char display_buffer[CALC_MAX_INPUT + 1] = "0";
static char expression_buffer[CALC_MAX_INPUT + 10] = ""; // For showing full expression
static fixed_t current_value = 0;
static fixed_t stored_value = 0;
static calc_operation_t current_operation = OP_NONE;
static char current_operator = 0; // Store current operator for display
static int selected_row = 0;
static int selected_col = 0;
static int new_number = 1;
static int scientific_mode = 0; // 0 = basic, 1 = scientific

// Scientific calculator button layout
static calc_button_t sci_buttons[SCI_CALC_BUTTON_ROWS][SCI_CALC_BUTTON_COLS] = {
    // Row 0: Functions
    {{"sin", 's', 0, 0, 0, 1, SCI_SIN}, {"cos", 'o', 0, 0, 0, 1, SCI_COS}, {"tan", 't', 0, 0, 0, 1, SCI_TAN}, {"sqrt", 'r', 0, 0, 0, 1, SCI_SQRT}, {"x^2", '2', 0, 0, 0, 1, SCI_POW2}, {"x^3", '3', 0, 0, 0, 1, SCI_POW3}, {"n!", '!', 0, 0, 0, 1, SCI_FACTORIAL}, {"pi", 'p', 0, 0, 0, 1, SCI_PI}},
    // Row 1: Memory and operations
    {{"C", 'C', 0, 0, 1, 0, SCI_NONE}, {"CE", 'E', 0, 0, 1, 0, SCI_NONE}, {"<-", 'B', 0, 0, 1, 0, SCI_NONE}, {"/", '/', 0, 1, 0, 0, SCI_NONE}, {"^", '^', 0, 1, 0, 0, SCI_NONE}, {"log", 'L', 0, 1, 0, 0, SCI_NONE}, {"ln", 'N', 0, 1, 0, 0, SCI_NONE}, {"e", 'e', 0, 0, 0, 1, SCI_E}},
    // Row 2: Numbers and operations
    {{"7", '7', 1, 0, 0, 0, SCI_NONE}, {"8", '8', 1, 0, 0, 0, SCI_NONE}, {"9", '9', 1, 0, 0, 0, SCI_NONE}, {"*", '*', 0, 1, 0, 0, SCI_NONE}, {"(", '(', 0, 0, 1, 0, SCI_NONE}, {")", ')', 0, 0, 1, 0, SCI_NONE}, {"+/-", 'n', 0, 0, 1, 0, SCI_NONE}, {"%", '%', 0, 1, 0, 0, SCI_NONE}},
    // Row 3: Numbers and operations
    {{"4", '4', 1, 0, 0, 0, SCI_NONE}, {"5", '5', 1, 0, 0, 0, SCI_NONE}, {"6", '6', 1, 0, 0, 0, SCI_NONE}, {"-", '-', 0, 1, 0, 0, SCI_NONE}, {"1/x", 'i', 0, 0, 0, 1, SCI_NONE}, {"x^y", 'x', 0, 1, 0, 0, SCI_NONE}, {"mod", 'm', 0, 1, 0, 0, SCI_NONE}, {"abs", 'a', 0, 0, 0, 1, SCI_NONE}},
    // Row 4: Numbers and operations
    {{"1", '1', 1, 0, 0, 0, SCI_NONE}, {"2", '2', 1, 0, 0, 0, SCI_NONE}, {"3", '3', 1, 0, 0, 0, SCI_NONE}, {"+", '+', 0, 1, 0, 0, SCI_NONE}, {"EXP", 'X', 0, 0, 1, 0, SCI_NONE}, {"RAN", 'R', 0, 0, 1, 0, SCI_NONE}, {"MEM", 'M', 0, 0, 1, 0, SCI_NONE}, {"RCL", 'c', 0, 0, 1, 0, SCI_NONE}},
    // Row 5: Bottom row
    {{"0", '0', 1, 0, 0, 0, SCI_NONE}, {"00", 'Z', 1, 0, 0, 0, SCI_NONE}, {".", '.', 0, 0, 1, 0, SCI_NONE}, {"=", '=', 0, 0, 1, 0, SCI_NONE}, {"EXIT", 27, 0, 0, 1, 0, SCI_NONE}, {"BASIC", 'b', 0, 0, 1, 0, SCI_NONE}, {"HELP", 'h', 0, 0, 1, 0, SCI_NONE}, {"?", '?', 0, 0, 1, 0, SCI_NONE}}
};

// Basic calculator button layout (keep existing)
static calc_button_t basic_buttons[5][4] = {
    {{"C", 'C', 0, 0, 1, 0, SCI_NONE}, {"CE", 'E', 0, 0, 1, 0, SCI_NONE}, {"<-", 'B', 0, 0, 1, 0, SCI_NONE}, {"/", '/', 0, 1, 0, 0, SCI_NONE}},
    {{"7", '7', 1, 0, 0, 0, SCI_NONE}, {"8", '8', 1, 0, 0, 0, SCI_NONE}, {"9", '9', 1, 0, 0, 0, SCI_NONE}, {"*", '*', 0, 1, 0, 0, SCI_NONE}},
    {{"4", '4', 1, 0, 0, 0, SCI_NONE}, {"5", '5', 1, 0, 0, 0, SCI_NONE}, {"6", '6', 1, 0, 0, 0, SCI_NONE}, {"-", '-', 0, 1, 0, 0, SCI_NONE}},
    {{"1", '1', 1, 0, 0, 0, SCI_NONE}, {"2", '2', 1, 0, 0, 0, SCI_NONE}, {"3", '3', 1, 0, 0, 0, SCI_NONE}, {"+", '+', 0, 1, 0, 0, SCI_NONE}},
    {{"0", '0', 1, 0, 0, 0, SCI_NONE}, {".", '.', 0, 0, 1, 0, SCI_NONE}, {"=", '=', 0, 0, 1, 0, SCI_NONE}, {"SCI", 'S', 0, 0, 1, 0, SCI_NONE}}
};

// Helper functions
void calc_clear_screen(void);
void calc_draw_display(void);
void calc_draw_buttons(void);
void calc_draw_button(int row, int col, int selected);
void calc_update_display(const char* text);
void calc_handle_number(char digit);
void calc_handle_operation(char op);
void calc_handle_equals(void);
void calc_handle_clear(void);
void calc_handle_clear_entry(void);
void calc_handle_backspace(void);
void calc_handle_decimal(void);
fixed_t calc_string_to_fixed(const char* str);
void calc_fixed_to_string(fixed_t num, char* str);
void calc_handle_scientific_function(sci_function_t func);
int calc_strlen(const char* str);
void calc_strcpy(char* dest, const char* src);
void calc_strcat(char* dest, const char* src);

void run_calculator(void) {
    scientific_mode = 0; // Start in basic mode
    run_scientific_calculator();
}

void run_scientific_calculator(void) {
    // Initialize calculator
    calc_strcpy(display_buffer, "0");
    calc_strcpy(expression_buffer, "");
    current_value = 0;
    stored_value = 0;
    current_operation = OP_NONE;
    current_operator = 0;
    selected_row = 0;
    selected_col = 0;
    new_number = 1;
    
    calc_clear_screen();
    calc_draw_display();
    calc_draw_buttons();
    
    // Main calculator loop
    while (1) {
        unsigned char key = keyboard_get_char();
        
        if (key != 0) {
            if (key == 27) { // ESC key or ESC button
                // Clean exit - reset state before returning
                calc_strcpy(display_buffer, "0");
                calc_strcpy(expression_buffer, "");
                current_value = 0;
                stored_value = 0;
                current_operation = OP_NONE;
                current_operator = 0;
                selected_row = 0;
                selected_col = 0;
                new_number = 1;
                scientific_mode = 0;
                kernel_main();
                return;
            }
            else if (key == NAV_UP_ARROW) {
                if (selected_row > 0) {
                    selected_row--;
                    calc_draw_buttons();
                }
            }
            else if (key == NAV_DOWN_ARROW) {
                int max_rows = scientific_mode ? SCI_CALC_BUTTON_ROWS : 5;
                if (selected_row < max_rows - 1) {
                    selected_row++;
                    calc_draw_buttons();
                }
            }
            else if (key == NAV_LEFT_ARROW) {
                if (selected_col > 0) {
                    selected_col--;
                    calc_draw_buttons();
                }
            }
            else if (key == NAV_RIGHT_ARROW) {
                int max_cols = scientific_mode ? SCI_CALC_BUTTON_COLS : 4;
                if (selected_col < max_cols - 1) {
                    selected_col++;
                    calc_draw_buttons();
                }
            }
            else if (key == '\n' || key == ' ') { // Enter or Space to press button
                calc_button_t* btn;
                if (scientific_mode) {
                    btn = &sci_buttons[selected_row][selected_col];
                } else {
                    btn = &basic_buttons[selected_row][selected_col];
                }
                
                if (btn->value == 27) { // ESC button
                    // Clean exit - reset state before returning
                    calc_strcpy(display_buffer, "0");
                    calc_strcpy(expression_buffer, "");
                    current_value = 0;
                    stored_value = 0;
                    current_operation = OP_NONE;
                    current_operator = 0;
                    selected_row = 0;
                    selected_col = 0;
                    new_number = 1;
                    scientific_mode = 0;
                    kernel_main();
                    return;
                }
                else if (btn->value == 'S') { // Switch to Scientific
                    scientific_mode = 1;
                    selected_row = 0;
                    selected_col = 0;
                    calc_clear_screen();
                    calc_draw_display();
                    calc_draw_buttons();
                }
                else if (btn->value == 'b') { // Switch to Basic
                    scientific_mode = 0;
                    selected_row = 0;
                    selected_col = 0;
                    calc_clear_screen();
                    calc_draw_display();
                    calc_draw_buttons();
                }
                else if (btn->is_number) {
                    if (btn->value == 'Z') { // "00" button
                        calc_handle_number('0');
                        calc_handle_number('0');
                    } else {
                        calc_handle_number(btn->value);
                    }
                }
                else if (btn->is_operation) {
                    calc_handle_operation(btn->value);
                }
                else if (btn->value == '=') {
                    calc_handle_equals();
                }
                else if (btn->value == 'C') {
                    calc_handle_clear();
                }
                else if (btn->value == 'E') {
                    calc_handle_clear_entry();
                }
                else if (btn->value == 'B') {
                    calc_handle_backspace();
                }
                else if (btn->value == '.') {
                    calc_handle_decimal();
                }
                else if (btn->value == '2') { // x^2 button
                    calc_handle_scientific_function(SCI_POW2);
                }
                else if (btn->value == '3') { // x^3 button
                    calc_handle_scientific_function(SCI_POW3);
                }
                else if (btn->is_scientific) {
                    calc_handle_scientific_function(btn->sci_func);
                }
                
                calc_draw_display();
            }
            else if (key >= '0' && key <= '9') { // Direct number input
                calc_handle_number(key);
                calc_draw_display();
            }
            else if (key == '+' || key == '-' || key == '*' || key == '/') { // Direct operation input
                calc_handle_operation(key);
                calc_draw_display();
            }
            else if (key == '=') {
                calc_handle_equals();
                calc_draw_display();
            }
            else if (key == 'c' || key == 'C') {
                calc_handle_clear();
                calc_draw_display();
            }
            else if (key == '\b') {
                calc_handle_backspace();
                calc_draw_display();
            }
            else if (scientific_mode && (key == 's' || key == 'o' || key == 't' || key == 'r' || key == '!' || key == 'p' || key == 'e')) {
                // Handle scientific functions via direct keyboard
                sci_function_t func = SCI_NONE;
                switch (key) {
                    case 's': func = SCI_SIN; break;
                    case 'o': func = SCI_COS; break;
                    case 't': func = SCI_TAN; break;
                    case 'r': func = SCI_SQRT; break;
                    case '!': func = SCI_FACTORIAL; break;
                    case 'p': func = SCI_PI; break;
                    case 'e': func = SCI_E; break;
                }
                if (func != SCI_NONE) {
                    calc_handle_scientific_function(func);
                    calc_draw_display();
                }
            }
        } else {
            // Small delay to prevent excessive CPU usage
            for (volatile int i = 0; i < 1000; i++) {}
        }
    }
}

void calc_clear_screen(void) {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    
    // Draw title bar
    print_set_cursor(0, 0);
    for (int i = 0; i < CALC_SCREEN_WIDTH; i++) {
        print_char(' ');
    }
    
    if (scientific_mode) {
        print_set_cursor(25, 0);
        print_str("Scientific Calculator");
    } else {
        print_set_cursor(30, 0);
        print_str("Basic Calculator");
    }
    
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
}

void calc_draw_display(void) {
    // Draw display background
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    
    for (int y = 2; y < 2 + CALC_DISPLAY_HEIGHT; y++) {
        print_set_cursor(10, y);
        for (int x = 0; x < CALC_DISPLAY_WIDTH; x++) {
            print_char(' ');
        }
    }
    
    // Draw display border
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(9, 1);
    for (int i = 0; i < CALC_DISPLAY_WIDTH + 2; i++) {
        print_char('-');
    }
    print_set_cursor(9, 5);
    for (int i = 0; i < CALC_DISPLAY_WIDTH + 2; i++) {
        print_char('-');
    }
    
    for (int y = 2; y < 5; y++) {
        print_set_cursor(9, y);
        print_char('|');
        print_set_cursor(10 + CALC_DISPLAY_WIDTH, y);
        print_char('|');
    }
    
    // Draw display text (right-aligned)
    print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_WHITE);
    int text_len = calc_strlen(display_buffer);
    int start_x = 10 + CALC_DISPLAY_WIDTH - text_len - 2;
    if (start_x < 10) start_x = 10;
    
    print_set_cursor(start_x, 3);
    print_str(display_buffer);
    
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
}

void calc_draw_buttons(void) {
    if (scientific_mode) {
        for (int row = 0; row < SCI_CALC_BUTTON_ROWS; row++) {
            for (int col = 0; col < SCI_CALC_BUTTON_COLS; col++) {
                calc_draw_button(row, col, (row == selected_row && col == selected_col));
            }
        }
    } else {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 4; col++) {
                calc_draw_button(row, col, (row == selected_row && col == selected_col));
            }
        }
    }
}

void calc_draw_button(int row, int col, int selected) {
    calc_button_t* btn;
    int x, y;
    int button_width, button_height;
    
    if (scientific_mode) {
        btn = &sci_buttons[row][col];
        x = SCI_CALC_START_X + col * (SCI_CALC_BUTTON_WIDTH + 1);
        y = SCI_CALC_START_Y + row * (SCI_CALC_BUTTON_HEIGHT + 1);
        button_width = SCI_CALC_BUTTON_WIDTH;
        button_height = SCI_CALC_BUTTON_HEIGHT;
    } else {
        btn = &basic_buttons[row][col];
        x = 15 + col * (8 + 2); // Basic calculator positioning
        y = 8 + row * (2 + 1);
        button_width = 8;
        button_height = 2;
    }
    
    // Set colors based on selection and button type
    if (selected) {
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_CYAN);
    } else if (btn->is_operation || btn->value == '=') {
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLUE);
    } else if (btn->is_special) {
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_RED);
    } else if (btn->is_scientific) {
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_MAGENTA);
    } else {
        print_set_color(PRINT_COLOR_BLACK, PRINT_COLOR_LIGHT_GRAY);
    }
    
    // Draw button background
    for (int by = 0; by < button_height; by++) {
        print_set_cursor(x, y + by);
        for (int bx = 0; bx < button_width; bx++) {
            print_char(' ');
        }
    }
    
    // Draw button label (centered)
    int label_len = calc_strlen(btn->label);
    int label_x = x + (button_width - label_len) / 2;
    int label_y = y + button_height / 2;
    
    print_set_cursor(label_x, label_y);
    print_str(btn->label);
    
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
}

void calc_handle_number(char digit) {
    // Prevent buffer overflow
    if (calc_strlen(display_buffer) >= CALC_MAX_INPUT - 1) {
        return;
    }
    
    if (new_number) {
        // Starting a new number
        display_buffer[0] = digit;
        display_buffer[1] = '\0';
        new_number = 0;
        
        // If we have an operation pending, build the full expression
        if (current_operation != OP_NONE && current_operator != 0) {
            // Create full expression display: "stored_value operator current_number"
            char temp_buffer[CALC_MAX_INPUT + 10];
            fixed_to_string(stored_value, temp_buffer);
            calc_strcat(temp_buffer, " ");
            char op_str[2] = {current_operator, '\0'};
            calc_strcat(temp_buffer, op_str);
            calc_strcat(temp_buffer, " ");
            calc_strcat(temp_buffer, display_buffer);
            
            // Copy to display if it fits
            if (calc_strlen(temp_buffer) < CALC_MAX_INPUT) {
                calc_strcpy(display_buffer, temp_buffer);
            }
        }
    } else if (display_buffer[0] == '0' && calc_strlen(display_buffer) == 1) {
        // Replace single zero
        display_buffer[0] = digit;
        display_buffer[1] = '\0';
    } else {
        // Append to existing number
        if (current_operation != OP_NONE && current_operator != 0) {
            // We're building the second number in an expression
            // Find the last space and replace everything after it
            char temp_num[CALC_MAX_INPUT];
            int last_space = -1;
            int len = calc_strlen(display_buffer);
            
            // Find the last space
            for (int i = len - 1; i >= 0; i--) {
                if (display_buffer[i] == ' ') {
                    last_space = i;
                    break;
                }
            }
            
            if (last_space >= 0) {
                // Extract current number part
                int num_start = last_space + 1;
                int num_len = len - num_start;
                for (int i = 0; i < num_len; i++) {
                    temp_num[i] = display_buffer[num_start + i];
                }
                temp_num[num_len] = digit;
                temp_num[num_len + 1] = '\0';
                
                // Rebuild display
                display_buffer[last_space + 1] = '\0';
                calc_strcat(display_buffer, temp_num);
            }
        } else {
            // Simple number append
            int len = calc_strlen(display_buffer);
            if (len < CALC_MAX_INPUT - 1) {
                display_buffer[len] = digit;
                display_buffer[len + 1] = '\0';
            }
        }
    }
}

void calc_handle_operation(char op) {
    if (current_operation != OP_NONE && !new_number) {
        calc_handle_equals();
    }
    
    stored_value = string_to_fixed(display_buffer);
    current_operator = op;
    
    switch (op) {
        case '+': current_operation = OP_ADD; break;
        case '-': current_operation = OP_SUB; break;
        case '*': current_operation = OP_MUL; break;
        case '/': current_operation = OP_DIV; break;
        case '^': current_operation = OP_POW; break;
        case 'L': current_operation = OP_LOG; break;
        case 'N': current_operation = OP_LN; break;
    }
    
    // Build expression for display
    calc_strcpy(expression_buffer, display_buffer);
    calc_strcat(expression_buffer, " ");
    char op_str[2] = {op, '\0'};
    calc_strcat(expression_buffer, op_str);
    calc_strcat(expression_buffer, " ");
    
    new_number = 1;
}

void calc_handle_equals(void) {
    if (current_operation == OP_NONE) return;
    
    // Extract the current number from display if it's an expression
    if (current_operator != 0) {
        // Find the last number in the expression
        char temp_num[CALC_MAX_INPUT];
        int len = calc_strlen(display_buffer);
        int last_space = -1;
        
        // Find the last space
        for (int i = len - 1; i >= 0; i--) {
            if (display_buffer[i] == ' ') {
                last_space = i;
                break;
            }
        }
        
        if (last_space >= 0) {
            // Extract the number after the last space
            int j = 0;
            for (int i = last_space + 1; i < len; i++) {
                temp_num[j++] = display_buffer[i];
            }
            temp_num[j] = '\0';
            current_value = string_to_fixed(temp_num);
        } else {
            current_value = string_to_fixed(display_buffer);
        }
    } else {
        current_value = string_to_fixed(display_buffer);
    }
    
    fixed_t result = stored_value;
    
    switch (current_operation) {
        case OP_ADD: 
            result = stored_value + current_value;
            break;
        case OP_SUB: 
            result = stored_value - current_value;
            break;
        case OP_MUL: 
            result = fixed_multiply(stored_value, current_value);
            break;
        case OP_DIV: 
            if (current_value == 0) {
                calc_strcpy(display_buffer, "Error");
                current_operation = OP_NONE;
                current_operator = 0;
                new_number = 1;
                return;
            }
            result = fixed_divide(stored_value, current_value);
            break;
        case OP_POW:
            // For simplicity, only integer powers
            {
                int exp = current_value / FIXED_POINT_SCALE;
                result = fixed_power(stored_value, exp);
            }
            break;
        default: return;
    }
    
    fixed_to_string(result, display_buffer);
    current_operation = OP_NONE;
    current_operator = 0;
    new_number = 1;
}

void calc_handle_clear(void) {
    calc_strcpy(display_buffer, "0");
    calc_strcpy(expression_buffer, "");
    current_value = 0;
    stored_value = 0;
    current_operation = OP_NONE;
    current_operator = 0;
    new_number = 1;
}

void calc_handle_clear_entry(void) {
    calc_strcpy(display_buffer, "0");
    new_number = 1;
}

void calc_handle_backspace(void) {
    int len = calc_strlen(display_buffer);
    if (len > 1) {
        display_buffer[len - 1] = '\0';
    } else {
        calc_strcpy(display_buffer, "0");
        new_number = 1;
    }
}

void calc_handle_decimal(void) {
    // Re-enable decimal operations with fixed-point arithmetic
    if (new_number) {
        calc_strcpy(display_buffer, "0.");
        new_number = 0;
        return;
    }
    
    // Check if decimal already exists
    for (int i = 0; display_buffer[i]; i++) {
        if (display_buffer[i] == '.') {
            return; // Already has decimal
        }
    }
    
    if (calc_strlen(display_buffer) < CALC_MAX_INPUT - 1) {
        calc_strcat(display_buffer, ".");
    }
}

// Handle scientific functions
void calc_handle_scientific_function(sci_function_t func) {
    fixed_t value = string_to_fixed(display_buffer);
    fixed_t result = 0;
    
    switch (func) {
        case SCI_SIN:
            result = fixed_sin(value);
            break;
        case SCI_COS:
            result = fixed_cos(value);
            break;
        case SCI_TAN:
            result = fixed_tan(value);
            break;
        case SCI_SQRT:
            result = fixed_sqrt(value);
            break;
        case SCI_POW2:
            result = fixed_multiply(value, value);
            break;
        case SCI_POW3:
            result = fixed_multiply(fixed_multiply(value, value), value);
            break;
        case SCI_FACTORIAL:
            {
                int n = value / FIXED_POINT_SCALE;
                result = fixed_factorial(n);
            }
            break;
        case SCI_PI:
            result = PI_FIXED;
            break;
        case SCI_E:
            result = E_FIXED;
            break;
        default:
            return;
    }
    
    fixed_to_string(result, display_buffer);
    new_number = 1;
}

// Simple string to long conversion (integer-only for kernel safety)
long calc_string_to_long(const char* str) {
    long result = 0;
    int negative = 0;
    int i = 0;
    
    if (str[0] == '-') {
        negative = 1;
        i = 1;
    }
    
    for (; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        }
        // Skip decimal point and everything after for now (integer calc only)
        else if (str[i] == '.') {
            break;
        }
    }
    
    return negative ? -result : result;
}

// Simple long to string conversion
void calc_long_to_string(long num, char* str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        str[0] = '-';
        str++;
        num = -num;
    }
    
    char temp[32];
    int i = 0;
    
    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    for (int j = 0; j < i; j++) {
        str[j] = temp[i - 1 - j];
    }
    str[i] = '\0';
}

// Helper string functions
int calc_strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void calc_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void calc_strcat(char* dest, const char* src) {
    int dest_len = calc_strlen(dest);
    int i = 0;
    while (src[i]) {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
}

// ===== FIXED-POINT ARITHMETIC FUNCTIONS =====

// Convert string to fixed-point number (supports decimals)
fixed_t string_to_fixed(const char* str) {
    fixed_t result = 0;
    int negative = 0;
    int i = 0;
    int decimal_found = 0;
    int decimal_places = 0;
    
    if (str[0] == '-') {
        negative = 1;
        i = 1;
    }
    
    for (; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            if (decimal_found) {
                decimal_places++;
                if (decimal_places <= FIXED_POINT_DECIMALS) {
                    result = result * 10 + (str[i] - '0');
                }
            } else {
                result = result * 10 + (str[i] - '0');
            }
        } else if (str[i] == '.' && !decimal_found) {
            decimal_found = 1;
        }
    }
    
    // Scale to fixed-point format
    if (decimal_found) {
        // Pad with zeros if needed
        for (int j = decimal_places; j < FIXED_POINT_DECIMALS; j++) {
            result *= 10;
        }
    } else {
        result *= FIXED_POINT_SCALE;
    }
    
    return negative ? -result : result;
}

// Convert fixed-point number to string
void fixed_to_string(fixed_t num, char* str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        str[0] = '-';
        str++;
        num = -num;
    }
    
    // Extract integer and fractional parts
    fixed_t int_part = num / FIXED_POINT_SCALE;
    fixed_t frac_part = num % FIXED_POINT_SCALE;
    
    // Convert integer part
    char temp[32];
    int i = 0;
    
    if (int_part == 0) {
        str[0] = '0';
        str[1] = '\0';
    } else {
        while (int_part > 0) {
            temp[i++] = (int_part % 10) + '0';
            int_part /= 10;
        }
        
        for (int j = 0; j < i; j++) {
            str[j] = temp[i - 1 - j];
        }
        str[i] = '\0';
    }
    
    // Add fractional part if non-zero
    if (frac_part > 0) {
        int len = calc_strlen(str);
        str[len] = '.';
        len++;
        
        // Remove trailing zeros from fraction
        while (frac_part % 10 == 0) {
            frac_part /= 10;
        }
        
        char frac_str[10];
        i = 0;
        while (frac_part > 0) {
            frac_str[i++] = (frac_part % 10) + '0';
            frac_part /= 10;
        }
        
        for (int j = 0; j < i; j++) {
            str[len + j] = frac_str[i - 1 - j];
        }
        str[len + i] = '\0';
    }
}

// Fixed-point multiplication
fixed_t fixed_multiply(fixed_t a, fixed_t b) {
    // Use 64-bit arithmetic to prevent overflow during multiplication
    long long result = ((long long)a * (long long)b) / FIXED_POINT_SCALE;
    
    // Check for overflow
    if (result > LONG_MAX || result < LONG_MIN) {
        return 0; // Return 0 on overflow (caller should check)
    }
    
    return (fixed_t)result;
}

// Fixed-point division
fixed_t fixed_divide(fixed_t a, fixed_t b) {
    if (b == 0) {
        return 0; // Division by zero
    }
    
    // Use 64-bit arithmetic for precision
    long long result = ((long long)a * FIXED_POINT_SCALE) / b;
    
    // Check for overflow
    if (result > LONG_MAX || result < LONG_MIN) {
        return 0;
    }
    
    return (fixed_t)result;
}

// Fixed-point square root using Newton's method
fixed_t fixed_sqrt(fixed_t x) {
    if (x <= 0) return 0;
    
    fixed_t guess = x / 2;
    if (guess == 0) guess = FIXED_POINT_SCALE; // Start with 1.0
    
    for (int i = 0; i < 10; i++) { // 10 iterations should be enough
        fixed_t new_guess = (guess + fixed_divide(x, guess)) / 2;
        if (new_guess == guess) break; // Converged
        guess = new_guess;
    }
    
    return guess;
}

// Fixed-point power (integer exponent)
fixed_t fixed_power(fixed_t base, int exp) {
    if (exp == 0) return FIXED_POINT_SCALE; // base^0 = 1
    if (exp == 1) return base;
    if (exp < 0) return fixed_divide(FIXED_POINT_SCALE, fixed_power(base, -exp));
    
    fixed_t result = FIXED_POINT_SCALE;
    fixed_t current_power = base;
    
    while (exp > 0) {
        if (exp & 1) {
            result = fixed_multiply(result, current_power);
        }
        current_power = fixed_multiply(current_power, current_power);
        exp >>= 1;
    }
    
    return result;
}

// Simple trigonometric functions using Taylor series (limited precision)
fixed_t fixed_sin(fixed_t x) {
    // Normalize x to [0, 2π]
    fixed_t two_pi = 2 * PI_FIXED;
    while (x >= two_pi) x -= two_pi;
    while (x < 0) x += two_pi;
    
    // Use Taylor series: sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ...
    fixed_t result = x;
    fixed_t term = x;
    
    for (int i = 1; i < 5; i++) { // Limited terms for kernel
        term = fixed_multiply(term, fixed_multiply(x, x));
        term = fixed_divide(term, (2*i) * (2*i + 1) * FIXED_POINT_SCALE);
        
        if (i % 2 == 1) {
            result -= term;
        } else {
            result += term;
        }
    }
    
    return result;
}

fixed_t fixed_cos(fixed_t x) {
    // cos(x) = sin(x + π/2)
    return fixed_sin(x + (PI_FIXED / 2));
}

fixed_t fixed_tan(fixed_t x) {
    fixed_t cos_x = fixed_cos(x);
    if (cos_x == 0) return 0; // Undefined
    return fixed_divide(fixed_sin(x), cos_x);
}

// Factorial (limited to reasonable values)
fixed_t fixed_factorial(int n) {
    if (n < 0 || n > 10) return 0; // Limited range for kernel
    
    fixed_t result = FIXED_POINT_SCALE; // 1.0
    for (int i = 2; i <= n; i++) {
        result = fixed_multiply(result, i * FIXED_POINT_SCALE);
    }
    
    return result;
}