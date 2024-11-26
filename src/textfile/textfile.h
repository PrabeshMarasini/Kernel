#ifndef TEXTFILE_H
#define TEXTFILE_H

void display_textfile(const char *filename);
char print_get_char(int x, int y);
void scroll_screen();
// void display_loading_animation(const char *filename);
void clear_line(int line);
void print_line_with_color(int x, int y, const char *text, int fg_color, int bg_color);
// void display_textfile(const char *filename);

#endif
