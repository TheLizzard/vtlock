#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#include "../types/list.h"
#include "keyboard.h"
#include "cursor.h"


void keyboard_wait_for_enter(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    while (true) {
        int ch = getchar();
        if (ch == EOF) { puts(""); clearerr(stdin); break; }
        else if (ch == '\n') { break; }
    }
}


#define _ignore_escape_seq() { \
    while (true) { \
        if (('a' <= ch) && (ch <= 'z')) { break; } \
        if (('A' <= ch) && (ch <= 'Z')) { break; } \
        if (ch == '~') { break; } \
        ch = (uint8_t) getchar(); \
    } \
}

bool keyboard_stdin_empty() {
    struct timeval tv = {0, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // select() returns:
    //   >0  if data available,
    //   0   if no data available,
    //   -1  on error
    int retval = select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv);
    if (retval < 0) { perror("select"); abort(); return false; } /* Error */
    else if (retval == 0) { return true; } /* Timeout => empty */
    else { return false; } /* Not empty */
}


#define STDIN_NON_BLOCKING(func) { \
    int _tmp_flags = fcntl(STDIN_FILENO, F_GETFL, 0); \
    fcntl(STDIN_FILENO, F_SETFL, _tmp_flags|O_NONBLOCK); \
    { func } \
    fcntl(STDIN_FILENO, F_SETFL, _tmp_flags); \
}

Bytes keyboard_flush_stdin() {
    LIST_BEGIN(uint8_t, size, buf_size, buf, false)
    STDIN_NON_BLOCKING({
        TermSettings settings = cursor_no_echo();
        while (true) {
            int chr = getchar();
            if (chr == EOF) { break; }
            buf[size++] = (uint8_t) chr;
            LIST_APPENDED(uint8_t, size, buf_size, buf, false)
        }
        cursor_yes_echo(settings);
    })
    return bytes_from_heap_data(buf, size);
}

String keyboard_ask_passwd(const char* prompt) {
    // Ignore everything in stdin up to now
    Bytes data = keyboard_flush_stdin();
    bytes_free(&data);
    // Set up some variables and print prompt
    size_t cursor = 0;
    printf("%s", prompt);
    TermSettings settings = cursor_no_echo();

    /*
    // https://www.xfree86.org/current/ctlseqs.html
    Ctrl+2          0
    Ctrl+A          1
    Ctrl+B          2
    Ctrl+C          3
    Ctrl+D          4
    ...           ...
    Ctrl+Z         26
    Escape         27  # This overlaps with \x1b
    Ctrl+3         27  # This overlaps with \x1b
    Ctrl+4         28
    Ctrl+5         29
    Ctrl+6         30
    Ctrl+7         31
    Ctrl+8        127  # This overlaps with Delete?
    \n             10  # This overlaps with Ctrl-k
    \r             13  # This overlaps with Ctrl-n
    Backspace     127
    Delete         27 91 51 126
    Up             27 91 65
    Down           27 91 66
    Right          27 91 67
    Left           27 91 68
    End            27 91 70
    Home           27 91 72
    Break          27 91 80
    PgUp           27 91 53 126
    PgDown         27 91 54 126
    Shift+Left     27 91 <49 59 50> 68
    Alt+Left       27 91 <49 59 51> 68
    Ctrl+Left      27 91 <49 59 53> 68
    Win+Home       27 91 <49 126>
    Alt-<Key>      27 <Key>  # Where key is in [33,64], [91,96], [123,126]
    Alt-<Ctrl-Key> 27 <Ctrl-Key>
    */
    LIST_BEGIN(uint8_t, size, capacity, passwd, true)
    bool done = false;
    while (!done) {
        // bool alt = false;
        // bool ctrl = false;
        // bool unknown_mod = false;
        uint8_t ch = (uint8_t) getchar();

        switch (ch) {
            case 0:   // <Null>
            case 4:   // <EOF> [Control-D]
            case 10:  // "\n"
            case 13:  // "\r" [Enter]
                done = true;
                break;
            case 255: // Error (EOF on non-echo)
                perror("getchar() failed");
                done = true;
                break;
            case 27: // Escape sequence ("\x1b")
                STDIN_NON_BLOCKING({
                    ch = (uint8_t) getchar();
                })
                if (ch == (uint8_t) EOF) { done = true; break; } /* Escape */

                if ((1 <= ch) && (ch <= 26)) {
                    /* alt = true; ctrl = true; */
                    break;
                }
                if (('a' <= ch) && (ch <= 'z')) { /* alt = true; */ break; }
                if (('A' <= ch) && (ch <= 'Z')) { /* alt = true; */ break; }
                if ((33 <= ch) && (ch <= 64)) { /* alt = true; */ break; }
                if ((92 <= ch) && (ch <= 96)) { /* alt = true; */ break; }
                if ((123 <= ch) && (ch <= 126)) { /* alt = true; */ break; }
                if (ch != 91) { _ignore_escape_seq(); break; }
                ch = (uint8_t) getchar(); // CSI ("\x1b[")

                /*
                if (ch == 49) { // With a modifier (ctrl/alt)
                    if (getchar() != 59) {
                        _ignore_escape_seq();
                        break;
                    }
                    ch = (uint8_t) getchar();
                    if (ch == 51) { alt = true; }
                    else if (ch == 53) { ctrl = true; }
                    else { unknown_mod = true; }
                    ch = (uint8_t) getchar();
                }
                */

                switch (ch) {
                    case 51: // Delete? ("\x1b[3")
                        ch = (uint8_t) getchar();
                        if (ch == '~') { // Delete ("\x1b[3~")
                            if (cursor == size) { cursor_bell(); break; }
                            size--;
                            for (size_t i=cursor; i<size; i++) {
                                passwd[i] = passwd[i+1];
                            }
                            passwd[size] = 0;
                        } else {
                            _ignore_escape_seq();
                        }
                        break;
                    case 65: // Down ("\x1b[A")
                    case 72: // Home ("\x1b[H")
                        cursor = 0;
                        break;
                    case 66: // Up ("\x1b[B")
                    case 70: // End ("\x1b[F")
                        cursor = size;
                        break;
                    case 67: // Right ("\x1b[C")
                        if (cursor < size) { cursor++; }
                        else { cursor_bell(); }
                        break;
                    case 68: // Left ("\x1b[D")
                        if (cursor > 0) { cursor--; }
                        else { cursor_bell(); }
                        break;
                    default:
                        _ignore_escape_seq();
                }
                break;
            case 127: // Backspace
                if (!cursor) { cursor_bell(); break; }
                cursor--;
                for (size_t i=cursor; i<size-1; i++) {
                    passwd[i] = passwd[i+1];
                }
                passwd[size-1] = 0;
                size--;
                break;
            default:
                if (!iscntrl(ch)) { // Check not control character
                    size_t i = size;
                    while (true) {
                        passwd[i+1] = passwd[i];
                        if (i == 0) { break; }
                        if (i == cursor) { break; }
                        i--;
                    }
                    passwd[cursor++] = ch;
                    size++;
                    LIST_APPENDED(uint8_t, size, capacity, passwd, true)
                }
                break;
        }

        // Display line
        printf("\x1b[2K\x1b[1K\r\x1b[0K\x1b[K\r%s", prompt);
        for (size_t i=0; i<size; i++) { printf("*"); }
        // printf("%s", passwd);
        if (size > cursor) { printf("\x1b[%luD", size-cursor); }
        fflush(stdout);
    }
    cursor_yes_echo(settings);

    // Clear line to hide password length
    printf("\x1b[2K\x1b[1K\r\x1b[0K\x1b[K\r" \
           "                                        \r");
    clearerr(stdin);
    fflush(stdout);

    passwd[size++] = '\0';
    return string_from_bytes(bytes_from_heap_data(passwd, size), ENCODING_UTF8);
}


/*
int main() {
    Bytes bytes = keyboard_ask_passwd("Prompt: ");
    printf("Got length=%lu character/s (without null)\n", bytes.len-1);
    printf("Got '%s'\n", bytes.data);
    return 0;
}
// */
