#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

#include "screen.h"
#include "cursor.h"


RowColPair screen_term_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return (RowColPair) {w.ws_row, w.ws_col};
}

void screen_clear_box(RowColPair top_left, RowColPair bottom_right) {
    RowColPair termsize = screen_term_size();
    for (int32_t row=top_left.row; row<=bottom_right.row; row++) {
        if (row < 1) { continue; }
        if (row > termsize.row) { break; }
        cursor_move(row, top_left.col);
        for (int32_t col=top_left.col; col<=bottom_right.col; col++) {
            if (col < 1) { continue; }
            if (col > termsize.col) { break; }
            printf(" ");
        }
    }
}
