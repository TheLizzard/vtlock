#pragma once
#include <termios.h>
#include <stdint.h>

#include "../types/string.h"


String keyboard_ask_passwd(const char* prompt);
void keyboard_wait_for_enter(const char* prompt);
bool keyboard_stdin_empty();
Bytes keyboard_flush_stdin();
