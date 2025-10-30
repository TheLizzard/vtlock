#pragma once
#include "bytes.h"


typedef uint8_t* Character;
typedef Character* Font;

extern Font FONTS[]; /* 0 1 2 3 4 5 */
extern const char* FILLS[]; /* 0:# 1:░ 2:▒ 3:▓ 4:█ */

void display_time_init();
Success display_time(const char* fill, Font font, RowColPair pos, uint8_t scale,
                     uint8_t colour);
