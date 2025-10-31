#pragma once
#include "../types/success.h"


Success lock_console_switch();
Success unlock_console_switch();
Success lock_signals();
Success set_signal(int sig, void (*func)(int));
