#ifndef UTILS_H
#define UTILS_H

void clear_screen();
void move_cursor(int r, int c);
void print_left_header(const char *title);
void enable_ansi_if_windows();
void getDate(char *buffer);

#endif
