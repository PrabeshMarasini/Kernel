#ifndef SHELL_H
#define SHELL_H

void handle_special_keys(char key);
void update_header(const char *filename, int input_length); 
void run_shell(); 
void switch_to_shell();
void start_shell();

#endif
