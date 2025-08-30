#include "snake.h"
#include "../intf/print.h"
#include "../drivers/keyboard/keyboard.h"

void kernel_main(void);

// Simple random number generator for kernel environment
static unsigned long snake_rand_seed = 1;

int snake_random(int min, int max) {
    snake_rand_seed = (snake_rand_seed * 1103515245 + 12345) & 0x7fffffff;
    return min + (snake_rand_seed % (max - min + 1));
}

void snake_delay(int milliseconds) {
    // Simple delay loop for kernel environment
    for (volatile int i = 0; i < milliseconds * 1000; i++) {
        // Busy wait
    }
}

void snake_clear_screen(void) {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
}

void snake_draw_border(void) {
    print_set_color(WALL_COLOR, BACKGROUND_COLOR);
    
    // Draw top and bottom borders
    for (int x = 0; x < GAME_WIDTH + 2; x++) {
        print_set_cursor(x, 0);
        print_char('#');
        print_set_cursor(x, GAME_HEIGHT + 1);
        print_char('#');
    }
    
    // Draw left and right borders
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        print_set_cursor(0, y);
        print_char('#');
        print_set_cursor(GAME_WIDTH + 1, y);
        print_char('#');
    }
}

void snake_draw_score(int score) {
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_set_cursor(5, GAME_HEIGHT + 3);
    print_str("Score: ");
    
    // Convert score to string
    char score_str[10];
    int i = 0;
    int temp = score;
    
    if (score == 0) {
        score_str[i++] = '0';
    } else {
        while (temp > 0) {
            score_str[i++] = (temp % 10) + '0';
            temp /= 10;
        }
        
        // Reverse the string
        for (int j = 0; j < i / 2; j++) {
            char swap = score_str[j];
            score_str[j] = score_str[i - 1 - j];
            score_str[i - 1 - j] = swap;
        }
    }
    
    score_str[i] = '\0';
    print_str(score_str);
    
    print_set_cursor(25, GAME_HEIGHT + 3);
    print_str("Controls: Arrow Keys, ESC=Exit, P=Pause");
}

void snake_show_start_screen(void) {
    print_set_color(PRINT_COLOR_CYAN, PRINT_COLOR_BLACK);
    print_set_cursor(GAME_WIDTH / 2 - 6, GAME_HEIGHT / 2 - 3);
    print_str("SNAKE GAME");
    
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_set_cursor(GAME_WIDTH / 2 - 12, GAME_HEIGHT / 2 - 1);
    print_str("Use Arrow Keys to Control");
    
    print_set_cursor(GAME_WIDTH / 2 - 8, GAME_HEIGHT / 2 + 1);
    print_str("P = Pause/Resume");
    
    print_set_cursor(GAME_WIDTH / 2 - 6, GAME_HEIGHT / 2 + 3);
    print_str("ESC = Exit");
    
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_set_cursor(GAME_WIDTH / 2 - 10, GAME_HEIGHT / 2 + 6);
    print_str("Press any key to start!");
}

void snake_show_game_over(int score) {
    print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
    print_set_cursor(GAME_WIDTH / 2 - 5, GAME_HEIGHT / 2 - 2);
    print_str("GAME OVER!");
    
    print_set_cursor(GAME_WIDTH / 2 - 8, GAME_HEIGHT / 2);
    print_str("Final Score: ");
    
    // Convert score to string
    char score_str[10];
    int i = 0;
    int temp = score;
    
    if (score == 0) {
        score_str[i++] = '0';
    } else {
        while (temp > 0) {
            score_str[i++] = (temp % 10) + '0';
            temp /= 10;
        }
        
        // Reverse the string
        for (int j = 0; j < i / 2; j++) {
            char swap = score_str[j];
            score_str[j] = score_str[i - 1 - j];
            score_str[i - 1 - j] = swap;
        }
    }
    
    score_str[i] = '\0';
    print_str(score_str);
    
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_set_cursor(GAME_WIDTH / 2 - 12, GAME_HEIGHT / 2 + 3);
    print_str("Press SPACE to restart");
    
    print_set_cursor(GAME_WIDTH / 2 - 10, GAME_HEIGHT / 2 + 5);
    print_str("Press ESC to exit");
}

void snake_restart_game(snake_game_t* game) {
    // Reset snake position to center
    int center_x = GAME_WIDTH / 2;
    int center_y = GAME_HEIGHT / 2;
    
    game->snake_length = INITIAL_SNAKE_LENGTH;
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        game->snake[i].pos.x = center_x - i;
        game->snake[i].pos.y = center_y;
    }
    
    // Reset game variables
    game->direction = DIR_RIGHT;
    game->next_direction = DIR_RIGHT;
    game->score = 0;
    game->last_score = -1;
    game->state = GAME_RUNNING;
    game->game_speed = 2000; // Reset to initial speed
    
    // Clear and reset screen buffers
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        for (int x = 0; x < GAME_WIDTH + 2; x++) {
            if (x == 0 || x == GAME_WIDTH + 1 || y == 0 || y == GAME_HEIGHT + 1) {
                game->screen_buffer[y][x] = '#';
                game->prev_buffer[y][x] = ' '; // Force redraw
            } else {
                game->screen_buffer[y][x] = ' ';
                game->prev_buffer[y][x] = '#'; // Force redraw
            }
        }
    }
    
    // Generate new food
    snake_generate_food(game);
    
    // Clear the screen and redraw borders
    snake_clear_screen();
    print_set_color(WALL_COLOR, BACKGROUND_COLOR);
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        for (int x = 0; x < GAME_WIDTH + 2; x++) {
            if (x == 0 || x == GAME_WIDTH + 1 || y == 0 || y == GAME_HEIGHT + 1) {
                print_set_cursor(x, y);
                print_char('#');
            }
        }
    }
}

void snake_init_game(snake_game_t* game) {
    // Initialize snake in the center
    int center_x = GAME_WIDTH / 2;
    int center_y = GAME_HEIGHT / 2;
    
    game->snake_length = INITIAL_SNAKE_LENGTH;
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        game->snake[i].pos.x = center_x - i;
        game->snake[i].pos.y = center_y;
    }
    
    game->direction = DIR_RIGHT;
    game->next_direction = DIR_RIGHT;
    game->score = 0;
    game->last_score = -1;
    game->state = GAME_START;
    game->game_speed = 2000; // Much slower initial speed (4x slower than before)
    
    // Initialize buffers
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        for (int x = 0; x < GAME_WIDTH + 2; x++) {
            if (x == 0 || x == GAME_WIDTH + 1 || y == 0 || y == GAME_HEIGHT + 1) {
                game->screen_buffer[y][x] = '#';
                game->prev_buffer[y][x] = '#';
            } else {
                game->screen_buffer[y][x] = ' ';
                game->prev_buffer[y][x] = ' ';
            }
        }
    }
    
    // Initialize random seed with a simple method
    snake_rand_seed = 12345;
    
    snake_generate_food(game);
}

void snake_generate_food(snake_game_t* game) {
    int attempts = 0;
    do {
        game->food.x = snake_random(1, GAME_WIDTH);
        game->food.y = snake_random(1, GAME_HEIGHT);
        attempts++;
        
        // Check if food spawned on snake
        int on_snake = 0;
        for (int i = 0; i < game->snake_length; i++) {
            if (game->snake[i].pos.x == game->food.x && 
                game->snake[i].pos.y == game->food.y) {
                on_snake = 1;
                break;
            }
        }
        
        if (!on_snake || attempts > 100) break;
        
    } while (1);
}

int snake_check_collision(snake_game_t* game) {
    position_t head = game->snake[0].pos;
    
    // Check wall collision
    if (head.x < 1 || head.x > GAME_WIDTH || head.y < 1 || head.y > GAME_HEIGHT) {
        return 1;
    }
    
    // Check self collision
    for (int i = 1; i < game->snake_length; i++) {
        if (head.x == game->snake[i].pos.x && head.y == game->snake[i].pos.y) {
            return 1;
        }
    }
    
    return 0;
}

int snake_check_food_collision(snake_game_t* game) {
    position_t head = game->snake[0].pos;
    return (head.x == game->food.x && head.y == game->food.y);
}

void snake_move_snake(snake_game_t* game) {
    // Update direction
    game->direction = game->next_direction;
    
    // Move body segments
    for (int i = game->snake_length - 1; i > 0; i--) {
        game->snake[i].pos = game->snake[i - 1].pos;
    }
    
    // Move head
    switch (game->direction) {
        case DIR_UP:
            game->snake[0].pos.y--;
            break;
        case DIR_DOWN:
            game->snake[0].pos.y++;
            break;
        case DIR_LEFT:
            game->snake[0].pos.x--;
            break;
        case DIR_RIGHT:
            game->snake[0].pos.x++;
            break;
    }
}

void snake_update_game(snake_game_t* game) {
    if (game->state != GAME_RUNNING) return;
    
    snake_move_snake(game);
    
    // Check collisions
    if (snake_check_collision(game)) {
        game->state = GAME_OVER;
        return;
    }
    
    // Check food collision
    if (snake_check_food_collision(game)) {
        // Grow snake
        if (game->snake_length < MAX_SNAKE_LENGTH - 1) {
            game->snake_length++;
            // New segment is added at the tail automatically by move function
        }
        
        game->score += 10;
        snake_generate_food(game);
        
        // Increase speed very slightly
        if (game->game_speed > 200) {
            game->game_speed -= 1; // Much smaller speed increase
        }
    }
}

void snake_render_game(snake_game_t* game) {
    // Clear the game area in buffer
    for (int y = 1; y <= GAME_HEIGHT; y++) {
        for (int x = 1; x <= GAME_WIDTH; x++) {
            game->screen_buffer[y][x] = ' ';
        }
    }
    
    // Add snake to buffer
    for (int i = 0; i < game->snake_length; i++) {
        int x = game->snake[i].pos.x;
        int y = game->snake[i].pos.y;
        if (x >= 1 && x <= GAME_WIDTH && y >= 1 && y <= GAME_HEIGHT) {
            if (i == 0) {
                game->screen_buffer[y][x] = 'O'; // Head
            } else {
                game->screen_buffer[y][x] = 'o'; // Body
            }
        }
    }
    
    // Add food to buffer
    if (game->food.x >= 1 && game->food.x <= GAME_WIDTH && 
        game->food.y >= 1 && game->food.y <= GAME_HEIGHT) {
        game->screen_buffer[game->food.y][game->food.x] = '*';
    }
    
    // Batch update approach - collect all changes first
    static position_t changes[MAX_SNAKE_LENGTH + 10]; // Static to avoid stack issues
    static char change_chars[MAX_SNAKE_LENGTH + 10];
    int change_count = 0;
    
    // Find all changes
    for (int y = 0; y < GAME_HEIGHT + 2; y++) {
        for (int x = 0; x < GAME_WIDTH + 2; x++) {
            if (game->screen_buffer[y][x] != game->prev_buffer[y][x]) {
                if (change_count < MAX_SNAKE_LENGTH + 9) {
                    changes[change_count].x = x;
                    changes[change_count].y = y;
                    change_chars[change_count] = game->screen_buffer[y][x];
                    change_count++;
                }
                game->prev_buffer[y][x] = game->screen_buffer[y][x];
            }
        }
    }
    
    // Apply all changes in one go to reduce flickering
    for (int i = 0; i < change_count; i++) {
        print_set_cursor(changes[i].x, changes[i].y);
        
        // Set appropriate color
        if (change_chars[i] == '#') {
            print_set_color(WALL_COLOR, BACKGROUND_COLOR);
        } else if (change_chars[i] == 'O' || change_chars[i] == 'o') {
            print_set_color(SNAKE_COLOR, BACKGROUND_COLOR);
        } else if (change_chars[i] == '*') {
            print_set_color(FOOD_COLOR, BACKGROUND_COLOR);
        } else {
            print_set_color(PRINT_COLOR_WHITE, BACKGROUND_COLOR);
        }
        
        print_char(change_chars[i]);
    }
    
    // Force food rendering if it's not being updated (debug food visibility)
    if (change_count == 0 || game->score != game->last_score) {
        print_set_color(FOOD_COLOR, BACKGROUND_COLOR);
        print_set_cursor(game->food.x, game->food.y);
        print_char('*');
    }
    
    // Update score only when it changes
    if (game->score != game->last_score) {
        snake_draw_score(game->score);
        game->last_score = game->score;
    }
}

void snake_handle_input(snake_game_t* game, unsigned char key) {
    if (key == 27) { // ESC
        game->state = GAME_EXIT;
        return;
    }
    
    // Handle start screen - any key starts the game
    if (game->state == GAME_START) {
        game->state = GAME_RUNNING;
        // Clear the screen and redraw borders when starting the game
        snake_clear_screen();
        
        // Redraw borders
        print_set_color(WALL_COLOR, BACKGROUND_COLOR);
        for (int y = 0; y < GAME_HEIGHT + 2; y++) {
            for (int x = 0; x < GAME_WIDTH + 2; x++) {
                if (x == 0 || x == GAME_WIDTH + 1 || y == 0 || y == GAME_HEIGHT + 1) {
                    print_set_cursor(x, y);
                    print_char('#');
                }
            }
        }
        
        // Reset the screen buffers to ensure clean rendering
        for (int y = 0; y < GAME_HEIGHT + 2; y++) {
            for (int x = 0; x < GAME_WIDTH + 2; x++) {
                if (x == 0 || x == GAME_WIDTH + 1 || y == 0 || y == GAME_HEIGHT + 1) {
                    game->screen_buffer[y][x] = '#';
                    game->prev_buffer[y][x] = ' '; // Force redraw by making different
                } else {
                    game->screen_buffer[y][x] = ' ';
                    game->prev_buffer[y][x] = '#'; // Force redraw by making different
                }
            }
        }
        return;
    }
    
    if (key == 'p' || key == 'P') { // Pause
        if (game->state == GAME_RUNNING) {
            game->state = GAME_PAUSED;
        } else if (game->state == GAME_PAUSED) {
            game->state = GAME_RUNNING;
            // Clear the paused message area when resuming
            print_set_color(PRINT_COLOR_WHITE, BACKGROUND_COLOR);
            print_set_cursor(GAME_WIDTH / 2 - 3, GAME_HEIGHT / 2);
            print_str("      "); // Clear "PAUSED" text
        }
        return;
    }
    
    if (game->state != GAME_RUNNING) return;
    
    // Handle arrow keys for direction change
    direction_t new_direction = game->next_direction;
    
    if (key == NAV_UP_ARROW && game->direction != DIR_DOWN) {
        new_direction = DIR_UP;
    } else if (key == NAV_DOWN_ARROW && game->direction != DIR_UP) {
        new_direction = DIR_DOWN;
    } else if (key == NAV_LEFT_ARROW && game->direction != DIR_RIGHT) {
        new_direction = DIR_LEFT;
    } else if (key == NAV_RIGHT_ARROW && game->direction != DIR_LEFT) {
        new_direction = DIR_RIGHT;
    }
    
    game->next_direction = new_direction;
}

void run_snake_game(void) {
    snake_game_t game;
    
    snake_clear_screen();
    snake_init_game(&game);
    
    // Don't draw borders initially - they'll be drawn when game starts
    
    int move_counter = 0;
    int render_counter = 0;
    const int moves_per_second = 1; // Base speed
    const int fps = 13; // Frame rate
    const int frames_per_move = fps * 4 / moves_per_second; // 4x slower: 52 frames per move
    const int frames_per_render = 2; // Render every 2 frames to reduce flickering
    
    while (game.state != GAME_EXIT) {
        // Handle input (check multiple times per move for responsiveness)
        unsigned char key = keyboard_get_char();
        if (key != 0) {
            if (game.state == GAME_OVER) {
                if (key == ' ') { // Space to restart
                    snake_restart_game(&game);
                } else if (key == 27) { // ESC to exit
                    game.state = GAME_EXIT;
                }
            } else {
                snake_handle_input(&game, key);
            }
        }
        
        // Update game logic at controlled intervals with direction-based timing
        if (game.state == GAME_RUNNING) {
            move_counter++;
            
            // Different frame requirements based on direction to normalize visual speed
            int required_frames = frames_per_move;
            if (game.direction == DIR_UP || game.direction == DIR_DOWN) {
                required_frames = frames_per_move * 2; // Vertical moves take 2x longer
            }
            
            if (move_counter >= required_frames) {
                snake_update_game(&game);
                move_counter = 0;
            }
        }
        
        // Render game at reduced frequency to minimize flickering
        render_counter++;
        if (render_counter >= frames_per_render) {
            if (game.state == GAME_START) {
                snake_show_start_screen();
            } else if (game.state == GAME_RUNNING || game.state == GAME_PAUSED) {
                snake_render_game(&game);
                
                if (game.state == GAME_PAUSED) {
                    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
                    print_set_cursor(GAME_WIDTH / 2 - 3, GAME_HEIGHT / 2);
                    print_str("PAUSED");
                }
            } else if (game.state == GAME_OVER) {
                snake_show_game_over(game.score);
            }
            render_counter = 0;
        }
        
        // Much longer delay for 4x slower gameplay
        snake_delay(100); // Increased from 75ms to 100ms for additional slowdown
    }
    
    // Return to main
    kernel_main();
}