#include <stdio.h>
#include <time.h>
#include <string.h>
#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

void clear_screen() { printf("\033[2J\033[H"); }
void move_cursor(int r, int c) { printf("\033[%d;%dH", r, c); }
void print_left_header(const char *title) {
    printf("=== Retail Store System === %s\n", title);
    printf("------------------------------------------------------------\n");
}
void enable_ansi_if_windows(){
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    mode |= 0x0004;
    SetConsoleMode(hOut, mode);
#endif
}
void getDate(char *buffer) {
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    sprintf(buffer,"%04d-%02d-%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}
