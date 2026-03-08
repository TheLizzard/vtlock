#pragma once
#include <stdint.h>


typedef struct {
    int32_t row; /* line */
    int32_t col; /* column */
} RowColPair;

RowColPair screen_term_size();
#define screen_clear() printf("\x1b[0m\x1b[1J\x1b[H\x1b[J\x1b[2J\x1b[3J")
void screen_clear_box(RowColPair top_left, RowColPair bottom_right);
