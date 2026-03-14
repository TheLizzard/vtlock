#pragma once
#include <termios.h>
#include <stdint.h>

#include "../types/string.h"


String keyboard_ask_passwd(const char* prompt);
void keyboard_wait_for_keypress(const char* prompt, const char chr);
bool keyboard_stdin_empty();
Bytes keyboard_flush_stdin();
