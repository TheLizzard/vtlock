#pragma once
#include <termios.h>


typedef struct termios TermSettings;

#define cursor_hide() printf("\x1b[?25l")
#define cursor_show() printf("\x1b[?25h")
#define cursor_bell() printf("\a")
void cursor_move(int32_t row, int32_t col);
TermSettings cursor_no_echo();
void cursor_yes_echo(TermSettings settings);
