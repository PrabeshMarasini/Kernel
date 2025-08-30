#ifndef SNAKE_H
#define SNAKE_H

// Snake game interface function
void run_snake_game(void);

// Game constants
#define GAME_WIDTH 78
#define GAME_HEIGHT 23
#define MAX_SNAKE_LENGTH 300
#define INITIAL_SNAKE_LENGTH 3

// Game colors
#define SNAKE_COLOR PRINT_COLOR_GREEN
#define FOOD_COLOR PRINT_COLOR_RED
#define WALL_COLOR PRINT_COLOR_WHITE
#define BACKGROUND_COLOR PRINT_COLOR_BLACK

// Direction constants
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

// Game state
typedef enum {
    GAME_START,
    GAME_RUNNING,
    GAME_OVER,
    GAME_PAUSED,
    GAME_EXIT
} game_state_t;

// Position structure
typedef struct {
    int x;
    int y;
} position_t;

// Snake segment structure
typedef struct {
    position_t pos;
} snake_segment_t;

// Game structure
typedef struct {
    snake_segment_t snake[MAX_SNAKE_LENGTH];
    int snake_length;
    direction_t direction;
    direction_t next_direction;
    position_t food;
    int score;
    game_state_t state;
    int game_speed;
    // Screen buffer to prevent flickering
    char screen_buffer[GAME_HEIGHT + 2][GAME_WIDTH + 2];
    char prev_buffer[GAME_HEIGHT + 2][GAME_WIDTH + 2];
    int last_score;
} snake_game_t;

// Function declarations
void snake_init_game(snake_game_t* game);
void snake_restart_game(snake_game_t* game);
void snake_update_game(snake_game_t* game);
void snake_render_game(snake_game_t* game);
void snake_handle_input(snake_game_t* game, unsigned char key);
void snake_generate_food(snake_game_t* game);
int snake_check_collision(snake_game_t* game);
int snake_check_food_collision(snake_game_t* game);
void snake_move_snake(snake_game_t* game);
void snake_draw_border(void);
void snake_draw_score(int score);
void snake_show_start_screen(void);
void snake_show_game_over(int score);
void snake_clear_screen(void);

// Utility functions
int snake_random(int min, int max);
void snake_delay(int milliseconds);

#endif