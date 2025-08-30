#ifndef SHELL_H
#define SHELL_H

#include "../drivers/graphics/graphics.h"

void handle_special_keys(char key);
void update_header(const char *filename, int input_length); 
void run_shell(); 
void switch_to_shell();
void start_shell();

// Font command functions
void font_demo_command();
void font_size_command(font_size_t size);
void font_weight_command(font_weight_t weight);
void font_antialiasing_command(int enabled);
void font_reset_command();
void help_command();

#endif
