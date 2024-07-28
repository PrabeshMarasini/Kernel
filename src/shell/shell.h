#ifndef SHELL_H
#define SHELL_H

// Function prototypes for the shell
void handle_special_keys(char key); // Handles special key inputs
void update_header(const char *filename, int input_length); // Updates the header with the filename and input length
void run_shell(); // Main function to run the shell
void switch_to_shell();
void start_shell();

#endif // SHELL_H
