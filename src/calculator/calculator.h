#ifndef CALCULATOR_H
#define CALCULATOR_H

// Calculator interface function
void run_calculator(void);
void run_scientific_calculator(void);

// Calculator constants
#define CALC_SCREEN_WIDTH 80
#define CALC_SCREEN_HEIGHT 25
#define CALC_DISPLAY_WIDTH 60
#define CALC_DISPLAY_HEIGHT 3
#define CALC_MAX_INPUT 32

// Scientific calculator layout
#define SCI_CALC_BUTTON_ROWS 6
#define SCI_CALC_BUTTON_COLS 8
#define SCI_CALC_BUTTON_WIDTH 8
#define SCI_CALC_BUTTON_HEIGHT 2
#define SCI_CALC_START_X 5
#define SCI_CALC_START_Y 8

// Fixed-point arithmetic (3 decimal places)
#define FIXED_POINT_SCALE 1000
#define FIXED_POINT_DECIMALS 3

// Calculator operations
typedef enum {
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW,
    OP_ROOT,
    OP_LOG,
    OP_LN
} calc_operation_t;

// Scientific functions
typedef enum {
    SCI_NONE,
    SCI_SIN,
    SCI_COS,
    SCI_TAN,
    SCI_SQRT,
    SCI_POW2,
    SCI_POW3,
    SCI_FACTORIAL,
    SCI_PI,
    SCI_E
} sci_function_t;

// Button structure
typedef struct {
    char label[6];
    char value;
    int is_number;
    int is_operation;
    int is_special;
    int is_scientific;
    sci_function_t sci_func;
} calc_button_t;

// Fixed-point number type
typedef long fixed_t;

// Fixed-point arithmetic functions
fixed_t fixed_multiply(fixed_t a, fixed_t b);
fixed_t fixed_divide(fixed_t a, fixed_t b);
fixed_t fixed_sqrt(fixed_t x);
fixed_t fixed_power(fixed_t base, int exp);
fixed_t fixed_sin(fixed_t x);
fixed_t fixed_cos(fixed_t x);
fixed_t fixed_tan(fixed_t x);
fixed_t fixed_factorial(int n);

// Conversion functions
fixed_t string_to_fixed(const char* str);
void fixed_to_string(fixed_t num, char* str);

#endif