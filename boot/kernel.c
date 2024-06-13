// kernel.c

// Function prototypes
void clear_screen();
void print_string(const char *str);

void kernel_main() {
    clear_screen();
    print_string("Hello from kernel!\n");

    while (1) {
        // Do something
    }
}

void clear_screen() {
    // Clear screen implementation (assuming text mode buffer)
    char *vidptr = (char *)0xB8000;  // Video memory begins at address 0xB8000
    unsigned int i = 0;
    unsigned int j = 0;

    // Loop through each character cell in text mode
    while (j < 80 * 25 * 2) {
        vidptr[j] = ' ';   // Character
        vidptr[j + 1] = 0x07; // Attribute-byte - white on black screen
        j = j + 2;
    }
}

void print_string(const char *str) {
    // Print string implementation (assuming text mode buffer)
    char *vidptr = (char *)0xB8000;  // Video memory begins at address 0xB8000
    unsigned int i = 0;
    unsigned int j = 0;

    // Find end of video memory
    while (vidptr[j] != 0) {
        ++j;
    }

    // Print string to video memory
    while (str[i] != '\0') {
        vidptr[j] = str[i];
        vidptr[j + 1] = 0x07; // Attribute-byte - white on black screen
        ++i;
        j = j + 2;
    }
}
