#pragma once
#include "bytes.h"

Success lock_console_switch();
Success unlock_console_switch();
Success lock_signals();
Success set_signal(int sig, void (*func)(int));
