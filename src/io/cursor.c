#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "cursor.h"


void cursor_move(int32_t row, int32_t col) {
    printf("\x1b[%"PRIu32";%"PRIu32"H", row, col);
    printf("\x1b[%"PRIu32";%"PRIu32"f", row, col);
}

TermSettings cursor_no_echo() {
    TermSettings oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    TermSettings newt = oldt;
    newt.c_iflag &= (unsigned int) ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                                            | INLCR | IGNCR | ICRNL | IXON);
    newt.c_oflag &= (unsigned int) ~OPOST;
    newt.c_lflag &= (unsigned int) ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    newt.c_cflag &= (unsigned int) ~(CSIZE | PARENB);
    newt.c_cflag |= CS8;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    return oldt;
}

void cursor_yes_echo(TermSettings settings) {
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}
