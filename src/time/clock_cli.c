#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "../io/cursor.h"
#include "clock_cli.h"


// Each Character is a list of [width height ...data]
// Where each value inside data must be between 0 and (1<<width)-1
const Font FONT0 = (Character[]) {
    (uint8_t[]){4,5,15, 9, 9, 9,15}, /* 0 */
    (uint8_t[]){4,5, 1, 1, 1, 1, 1}, /* 1 */
    (uint8_t[]){4,5,15, 1,15, 8,15}, /* 2 */
    (uint8_t[]){4,5,15, 1,15, 1,15}, /* 3 */
    (uint8_t[]){4,5, 9, 9,15, 1, 1}, /* 4 */
    (uint8_t[]){4,5,15, 8,15, 1,15}, /* 5 */
    (uint8_t[]){4,5,15, 8,15, 9,15}, /* 6 */
    (uint8_t[]){4,5,15, 1, 1, 1, 1}, /* 7 */
    (uint8_t[]){4,5,15, 9,15, 9,15}, /* 8 */
    (uint8_t[]){4,5,15, 9,15, 1,15}, /* 9 */
    (uint8_t[]){1,5, 0, 1, 0, 1, 0}, /* : */
    (uint8_t[]){1,5, 0, 0, 0, 0, 0}, /*   */
};

const Font FONT1 = (Character[]) {
    (uint8_t[]){4,5, 6, 9, 9, 9, 6}, /* 0 */
    (uint8_t[]){4,5, 2, 6, 2, 2, 7}, /* 1 */
    (uint8_t[]){4,5,15, 1,15, 8,15}, /* 2 */
    (uint8_t[]){4,5,15, 1, 7, 1,15}, /* 3 */
    (uint8_t[]){4,5, 9, 9, 7, 1, 1}, /* 4 */
    (uint8_t[]){4,5,15, 8,15, 1,15}, /* 5 */
    (uint8_t[]){4,5,15, 8,15, 9,15}, /* 6 */
    (uint8_t[]){4,5,15, 1, 2, 2, 2}, /* 7 */
    (uint8_t[]){4,5,15, 9,15, 9,15}, /* 8 */
    (uint8_t[]){4,5,15, 9,15, 1,15}, /* 9 */
    (uint8_t[]){1,5, 0, 0, 1, 0, 0}, /* : */
    (uint8_t[]){1,5, 0, 0, 0, 0, 0}, /*   */
};

const Font FONT2 = (Character[]) {
    (uint8_t[]){4,5, 6, 9, 9, 9, 6}, /* 0 */
    (uint8_t[]){4,5, 2, 6, 2, 2, 7}, /* 1 */
    (uint8_t[]){4,5,15, 1,15, 8,15}, /* 2 */
    (uint8_t[]){4,5,15, 1, 7, 1,15}, /* 3 */
    (uint8_t[]){4,5, 9, 9, 7, 1, 1}, /* 4 */
    (uint8_t[]){4,5,15, 8,15, 1,15}, /* 5 */
    (uint8_t[]){4,5,15, 8,15, 9,15}, /* 6 */
    (uint8_t[]){4,5,15, 1, 2, 4, 4}, /* 7 */
    (uint8_t[]){4,5,15, 9,15, 9,15}, /* 8 */
    (uint8_t[]){4,5,15, 9,15, 1,15}, /* 9 */
    (uint8_t[]){1,5, 0, 0, 1, 0, 0}, /* : */
    (uint8_t[]){1,5, 0, 0, 0, 0, 0}, /*   */
};

const Font FONT3 = (Character[]) {
    (uint8_t[]){4,5, 6, 9, 9, 9, 6}, /* 0 */
    (uint8_t[]){4,5, 2, 6, 2, 2, 7}, /* 1 */
    (uint8_t[]){4,5,14, 1, 6, 8, 7}, /* 2 */
    (uint8_t[]){4,5,14, 1, 6, 1,14}, /* 3 */
    (uint8_t[]){4,5, 9, 9, 7, 1, 1}, /* 4 */
    (uint8_t[]){4,5, 7, 8, 6, 1,14}, /* 5 */
    (uint8_t[]){4,5, 6, 8,14, 9, 6}, /* 6 */
    (uint8_t[]){4,5,14, 1, 2, 4, 4}, /* 7 */
    (uint8_t[]){4,5, 6, 9, 6, 9, 6}, /* 8 */
    (uint8_t[]){4,5, 6, 9, 7, 1, 1}, /* 9 */
    (uint8_t[]){1,5, 0, 1, 0, 1, 0}, /* : */
    (uint8_t[]){1,5, 0, 0, 0, 0, 0}, /*   */
};

const Font FONT4 = (Character[]) {
    (uint8_t[]){3,5,2,5,5,5,2}, /* 0 */
    (uint8_t[]){3,5,2,6,2,2,7}, /* 1 */
    (uint8_t[]){3,5,7,1,7,4,7}, /* 2 */
    (uint8_t[]){3,5,7,1,3,1,7}, /* 3 */
    (uint8_t[]){3,5,5,5,7,1,1}, /* 4 */
    (uint8_t[]){3,5,7,4,7,1,7}, /* 5 */
    (uint8_t[]){3,5,7,4,7,5,7}, /* 6 */
    (uint8_t[]){3,5,7,1,2,2,2}, /* 7 */
    (uint8_t[]){3,5,7,5,7,5,7}, /* 8 */
    (uint8_t[]){3,5,7,5,7,1,7}, /* 9 */
    (uint8_t[]){1,5,0,1,0,1,0}, /* : */
    (uint8_t[]){1,5,0,0,0,0,0}, /*   */
};

const Font FONT5 = (Character[]) {
    (uint8_t[]){6,5,12,51,51,51,12}, /* 0 */
    (uint8_t[]){6,5,12,60,12,12,63}, /* 1 */
    (uint8_t[]){6,5,63, 3,63,48,63}, /* 2 */
    (uint8_t[]){6,5,63, 3,15, 3,63}, /* 3 */
    (uint8_t[]){6,5,51,51,63, 3, 3}, /* 4 */
    (uint8_t[]){6,5,63,48,63, 3,63}, /* 5 */
    (uint8_t[]){6,5,63,48,63,51,63}, /* 6 */
    (uint8_t[]){6,5,63, 3,12,12,12}, /* 7 */
    (uint8_t[]){6,5,63,51,63,51,63}, /* 8 */
    (uint8_t[]){6,5,63,51,63, 3,63}, /* 9 */
    (uint8_t[]){2,5, 0, 3, 0, 3, 0}, /* : */
    (uint8_t[]){2,5, 0, 0, 0, 0, 0}, /*   */
};

Font FONTS[] = (Font[]) {FONT0, FONT1, FONT2, FONT3, FONT4, FONT5};


const char* FILLS[] = {
                        "#",
                        "░",
                        "▒",
                        "▓",
                        "█",
                      };

const char* ZERO_FILLS[] = {"0","1","2","3","4","5","6","7","8","9",":"};

void display_number(RowColPair pos, Character chr, const char* ch0,
                    const char* ch1, uint8_t scale) {
    RowColPair term_size = screen_term_size();
    uint8_t _mask = (uint8_t) (1 << (chr[0]-1));
    for (uint8_t r=0; r<chr[1]; r++) {
        for (uint8_t rs=0; rs<scale; rs++) {
            uint8_t row_data = chr[r+2]; // [width height ...data]
            int32_t row = pos.row + r*scale + rs;
            if ((row < 1) || (row > term_size.row)) { continue; } // Bounds
            cursor_move(row, pos.col);
            for (uint8_t c=0; c<chr[0]; c++) {
                for (uint8_t cs=0; cs<scale; cs++) {
                    int32_t col = pos.col + c*scale + cs;
                    if (col < 1) { continue; } // Bounds
                    if (col > term_size.col) { break; } // Bounds
                    printf("%s", (row_data&_mask)?ch1:ch0);
                }
                row_data = (uint8_t) (row_data << 1);
            }
        }
    }
}

char* time_now() {
    time_t now = time(NULL);
    if (now == (time_t)(-1)) { perror("time failed"); return NULL; }
    struct tm* local_time = localtime(&now);
    if (local_time == NULL) { perror("localtime failed"); return NULL; }
    char* output = (char*) calloc(1, 9*sizeof(char));
    snprintf(output, 9 /* 6numbers + 2colons + NULL */, "%02d:%02d:%02d",
             local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    return output;
}


RowColPair last_draw_start;
RowColPair last_draw_end;
uint8_t last_draw_colour;
char* last_draw_time = NULL;
const char* last_draw_fill = NULL;
Font last_draw_font = NULL;
void display_time_init() {
    if (last_draw_time != NULL) { free(last_draw_time); }
    last_draw_time = NULL;
}

/*
Draws a 24h clock from `fill` characters using `font`.
The `pos` is 1 based. 0 means center in that direction.
`scale` allows to scale the clock in both the horizontal
  and vertical directions
`colour` is an integer colour that is from ANSI escape codes
  (0:default 90:grey 91:red 92:lime 93:yellow 96:cyan)
Note: Assumes that fill won't be freed
*/
Success display_time(const char* fill, Font font, RowColPair pos,
                     uint8_t scale, uint8_t colour) {
    // Get term size/time string
    RowColPair term_size = screen_term_size();
    char* time_str = time_now();
    char* time_str_actual = time_str;
    if (time_str == NULL) { return false; }

    // Calculate size
    int32_t size_rows = 0;
    int32_t size_cols = 0;
    for (uint8_t i=0; time_str[i]!='\x00'; i++) {
        Character chr = font[time_str[i]-'0'];
        size_cols += chr[0]*scale;
        size_rows = (size_rows<chr[1]) ? chr[1] : size_rows;
        if (time_str[i+1] != '\x00') {
            size_cols += font[11][0]*scale; // Space
        }
    }
    size_rows *= scale;

    // Calculate row/col (after centering)
    int32_t row = pos.row;
    int32_t col = pos.col;
    if (!row) { row = ((term_size.row - size_rows) >> 1) + 1; }
    if (!col) { col = ((term_size.col - size_cols) >> 1) + 1; }

    // Calculate if we have to full_redraw
    bool full_redraw = (
                         (last_draw_start.row != row) ||
                         (last_draw_start.col != col) ||
                         (last_draw_end.row != row+size_rows) ||
                         (last_draw_end.col != col+size_cols) ||
                         (last_draw_colour != colour) ||
                         (last_draw_fill != fill) ||
                         (last_draw_font != font) ||
                         (last_draw_time == NULL)
                       );
    if (full_redraw) { screen_clear_box(last_draw_start, last_draw_end); }
    last_draw_start = (RowColPair) {row, col};
    last_draw_end = (RowColPair) {row+size_rows, col+size_cols};
    last_draw_colour = colour;
    last_draw_fill = fill;
    last_draw_font = font;

    // Calculate what we have to redraw
    if (!full_redraw) {
        char* last_t = last_draw_time;
        while (time_str[0] != '\x00') {
            if (last_t[0] != time_str[0]) { break; }
            // cursor_move(row, col);
            // printf("K\b");
            col += font[time_str[0]-'0'][0]*scale;
            if (time_str[1] != '\x00') {
                col += font[11][0]*scale; // Space
            }
            last_t += sizeof(char);
            time_str += sizeof(char);
        }
    }
    if (last_draw_time != NULL) { free(last_draw_time); }
    last_draw_time = time_str_actual;

    // Redraw
    printf("\x1b[%um", colour);
    for (uint8_t i=0; time_str[i]!='\x00'; i++) {
        Character chr = font[time_str[i]-'0'];

        const char* cfill = fill;
        if (fill[0] == '\x00') { cfill = ZERO_FILLS[time_str[i]-'0']; }

        display_number((RowColPair) {row, col}, chr, " ", cfill, scale);

        // cursor_move(row, col);
        // printf("R\b");

        col += chr[0] * scale;
        col += font[11][0] * scale; // Space
    }
    printf("\x1b[0m");
    return true;
}


/*
int main() {
    cursor_hide();
    screen_clear();
    display_time_init();

    while (true){
        display_time(FILLS[1], FONTS[5], (RowColPair) {0,0}, 1, 0);
        fflush(stdout);
        sleep(1);
    }

    return 0;
}
// */
