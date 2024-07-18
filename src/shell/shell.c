#include "../intf/print.h"

void run_shell() {
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Kernel shell!\n");
    print_str("Shell> ");
    // Add your shell implementation here
    while (1) {
        // Read input and handle commands
    }
}
