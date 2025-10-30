#pragma once
#include <termios.h>

#include "bytes.h"

typedef struct {
    int32_t row; /* line */
    int32_t col; /* column */
} RowColPair;

RowColPair screen_term_size();
#define screen_clear() printf("\x1b[0m\x1b[2J")
void screen_clear_box(RowColPair top_left, RowColPair bottom_right);

typedef struct termios TermSettings;
TermSettings keyboard_no_echo();
void keyboard_yes_echo(TermSettings settings);
Bytes keyboard_ask_passwd(const char* prompt);
void keyboard_wait_for_enter(const char* prompt);
bool keyboard_stdin_empty();
Bytes keyboard_flush_stdin();


#define cursor_hide() printf("\x1b[?25l")
#define cursor_show() printf("\x1b[?25h")
#define cursor_bell() printf("\a")
void cursor_move(int32_t row, int32_t col);
