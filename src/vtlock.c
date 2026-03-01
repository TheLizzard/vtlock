#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "console/console_switch.h"
#include "io/keyboard.h"
#include "io/cursor.h"
#include "passwd.h"
#include "clock.h"


// #define DEBUG


#define ExitCode int
#define RESET "\x1b[0m"
#define RED "\x1b[91m"
#define GREEN "\x1b[92m"
#define YELLOW "\x1b[93m"
#define CYAN "\x1b[96m"
#define error(fmt, ...) fprintf(stderr, RED fmt RESET "\n", ##__VA_ARGS__)


#if defined(DEBUG)
    const char* DEFAULT_PASSWD_FILE = "vtlock-passwd";
#else
    const char* DEFAULT_PASSWD_FILE = "/vtlock-passwd";
#endif


#define str_eq(str1, str2) (!strcmp(str1, str2))
#define str_startswith(str1, str2) (strncmp(str1, str2, strlen(str1)) == 0)

// Argument macros
#define MAIN_ADD_ARGUMENT_BEGIN \
    const char* arg; \
    uint8_t i = 1; \
    while (i<argc) { \
        arg = argv[i++];

#define MAIN_ADD_ARGUMENT_END \
        goto _main_unknown_arg; \
        _main_arg_done: ; \
    }

#define MAIN_ADD_ARGUMENT_VALUE(argument_name, save_func) \
    if (str_startswith(argument_name, arg)) { \
        const char* val = arg + strlen(argument_name)*sizeof(char); \
        if (val[0] == '=') { \
            val += sizeof(char); \
            { save_func } \
            goto _main_arg_done; \
        } else if (val[0] == '\x00') { \
            if (i == argc) { goto _main_unfinished_arg; } \
            val = argv[i++]; \
            if (val[0] == '-') { \
                if ((val[1] < '0') || (val[1] > '9')) { \
                    goto _main_unfinished_arg; \
                } \
            } \
            { save_func } \
            goto _main_arg_done; \
        } \
    }

#define MAIN_ADD_ARGUMENT_FLAG(argument_name, save_func) \
    if (str_startswith(arg, argument_name)) { \
        const char* val = arg + strlen(argument_name)*sizeof(char); \
        if (val[0] == '\x00') { \
            { save_func } \
            goto _main_arg_done; \
        } \
    }

// toint* functions
#define toint(string) ({ \
    int _tmp_result = 0; \
    while (string[0] != '\x00') { \
        _tmp_result *= 10; \
        if ((string[0] < '0') || (string[0] > '9')) { \
            goto _main_invalid_value; \
        } \
        _tmp_result += string[0] - '0'; \
        string += sizeof(char); \
    } \
    _tmp_result; \
})

#define _toint(x, chk) ({ \
    int _tmp_x_val = toint(x); \
    if (!(chk)) { goto _main_invalid_value; } \
    _tmp_x_val; \
})

#define toint_min_max(x, min, max) \
    _toint(x, \
           ( (_tmp_x_val >= (min)) && (_tmp_x_val <= (max)) ) \
          )

#define toint_min(x, min) \
    _toint(x, \
           (_tmp_x_val >= (min)) \
          )

#define toint_max(x, max) \
    _toint(x, \
           (_tmp_x_val <= (max)) \
          )


// Parsing fill helper
#define _ARG_PARSE_SET_FILL(target) \
            { \
                if (target == NULL) { \
                    continue; \
                } \
                target = val; \
                if (val[0] == '\x00') { \
                    continue; \
                } \
                String val_str = string_from_charp(val, ENCODING_UTF8); \
                size_t val_len = val_str.len; \
                string_free(&val_str); \
                if (val_len != 1) { \
                    error("%s must not be more than 1 character", arg); \
                    goto _main_bad; \
                    break; \
                } \
                puts("switch"); \
                switch (val[0]) { \
                    case '1': \
                        target = FILLS[0]; break; \
                    case '2': \
                        target = FILLS[1]; break; \
                    case '3': \
                        target = FILLS[2]; break; \
                    case '4': \
                        target = FILLS[3]; break; \
                    case '5': \
                        target = FILLS[4]; break; \
                    default: \
                        break; \
                } \
            }


typedef struct {
    const char* __file__;
    const char* passwd_file;
    Font clock_font;
    const char* clock_fill; /* NULL if no clock */
    RowColPair clock_pos;
    uint8_t clock_scale;
    uint8_t clock_fps;
    uint8_t clock_colour;
    int32_t clock_sleep_time; /* -1 to disable */
    uint8_t clock_sleep_colour;
    const char* clock_sleep_fill;
} Args;

void usage(Args* args) {
    printf("Usage: "GREEN"%s"RESET" "
           "["GREEN"--pwdfile"RESET" <path>] "
           "["GREEN"--no-time"RESET"] "
           "["GREEN"--clock-fill"RESET" <+ve integer>/<character>] "
           "["GREEN"--clock-font"RESET" <+ve integer>] "
           "["GREEN"--clock-row"RESET" <uinteger>] "
           "["GREEN"--clock-col"RESET" <uinteger>] "
           "["GREEN"--clock-scale"RESET" <+ve integer>] "
           "["GREEN"--clock-fps"RESET" <+ve integer>] "
           "["GREEN"--clock-colour"RESET" <uinteger>] "
           "["GREEN"--clock-sleep-colour"RESET" <uinteger>] "
           "["GREEN"--clock-sleep-time"RESET" <integer>] "
           "["GREEN"--clock-sleep-fill"RESET" <+ve integer>/<character>]\n",
           args->__file__);

    printf("    "GREEN"--pwdfile"RESET"               : "
           "The password file "
           "(default: "GREEN"%s"RESET")\n", DEFAULT_PASSWD_FILE);

    printf("    "GREEN"--no-clock"RESET"              : "
           "Disable the clock\n");

    printf("    "GREEN"--clock-font"RESET"            : "
           "The font to use for clock "
           "(the numbers between "GREEN"1"RESET" and "GREEN"6"RESET") "
           "(default: "GREEN"6"RESET")\n");

    printf("    "GREEN"--clock-fill"RESET"            : "
           "The character to use for the clock "
           "("GREEN"1"RESET":# "GREEN"2"RESET":░ "GREEN"3"RESET":▒ "
                                GREEN"4"RESET":▓ "GREEN"5"RESET":█) "
           "(default: "GREEN"5"RESET")\n");

    printf("    "GREEN"--clock-row"RESET"             : "
           "Where to draw the clock "
           "("GREEN"0"RESET" means center) "
           "(default: "GREEN"0"RESET")\n");

    printf("    "GREEN"--clock-col"RESET"             : "
           "Where to draw the clock "
           "("GREEN"0"RESET" means center) "
           "(default: "GREEN"0"RESET")\n");

    printf("    "GREEN"--clock-scale"RESET"           : "
           "The scale of the clock "
           "(default: "GREEN"3"RESET")\n");

    printf("    "GREEN"--clock-fps"RESET"             : "
           "How many times should the clock be redrawn per second "
           "(default: "GREEN"10"RESET")\n");

    printf("    "GREEN"--clock-colour"RESET"          : "
           "The colour of the clock (using ANSI escape codes) "
           "(default: "GREEN"0"RESET" => \\x1b[0m)\n");

    printf("    "GREEN"--clock-sleep-colour"RESET"    : "
           "Same as "GREEN"--clock-colour"RESET" but when sleeping "
           "(default: "GREEN"90"RESET" => \\x1b[90m)\n");

    printf("    "GREEN"--clock-sleep-time"RESET"      : "
           "How long the clock should wait to switch from "
           GREEN"--clock-colour"RESET" to "GREEN"--clock-sleep-colour"RESET
           " (in seconds) "
           "("GREEN"-1"RESET" means never sleep) "
           "(default: "GREEN"30"RESET")\n");

    printf("    "GREEN"--clock-sleep-fill"RESET"      : "
           "The fill when sleeping. Same options as "GREEN"--clock-fill"RESET" "
           "(default: "GREEN"2"RESET")\n");

    printf("    "GREEN"-h"RESET" "GREEN"--help"RESET"               : "
           "Show this help message\n");
}

ExitCode actual_main(Args* args);


ExitCode main(int argc, const char** argv) {
    if (argc == 0) { error("Invalid command line argument/s"); return 1; }

    // Defaults
    Args* args = (Args*) calloc(1, sizeof(Args));
    args->__file__ = argv[0];
    args->passwd_file = DEFAULT_PASSWD_FILE;
    args->clock_font = FONTS[5];
    args->clock_fill = FILLS[4];
    args->clock_pos = (RowColPair) {0, 0};
    args->clock_scale = 3;
    args->clock_fps = 1;
    args->clock_colour = 0; /* Default term colour (usually white) */
    args->clock_sleep_time = 30;
    args->clock_sleep_colour = 90; /* Dark grey */
    args->clock_sleep_fill = FILLS[1];

    ExitCode exit_code = 0;
    // goto _main_usage_good; /* For debug */

    MAIN_ADD_ARGUMENT_BEGIN

        MAIN_ADD_ARGUMENT_VALUE("--pwdfile",
          {args->passwd_file = val;})

        MAIN_ADD_ARGUMENT_VALUE("--clock-fill",
            _ARG_PARSE_SET_FILL(args->clock_fill))

        MAIN_ADD_ARGUMENT_VALUE("--clock-sleep-fill",
            _ARG_PARSE_SET_FILL(args->clock_sleep_fill))

        MAIN_ADD_ARGUMENT_VALUE("--clock-font",
          {args->clock_font = FONTS[toint_min_max(val, 1, 6)-1];})

        MAIN_ADD_ARGUMENT_VALUE("--clock-row",
          {args->clock_pos.row = toint_min(val, 0);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-col",
          {args->clock_pos.col = toint_min(val, 0);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-scale",
          {args->clock_scale = (uint8_t)toint_min_max(val, 1, 255);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-fps",
          {args->clock_fps = (uint8_t)toint_min_max(val, 1, 255);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-colour",
          {args->clock_colour = (uint8_t)toint_min_max(val, 1, 255);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-sleep-colour",
          {args->clock_sleep_colour = (uint8_t)toint_min_max(val, 1, 255);})

        MAIN_ADD_ARGUMENT_VALUE("--clock-sleep-time",
          {args->clock_sleep_time = toint_min(val, -1);})

        MAIN_ADD_ARGUMENT_FLAG("-h",
          {goto _main_usage_good;})

        MAIN_ADD_ARGUMENT_FLAG("--h",
          {goto _main_usage_good;})

        MAIN_ADD_ARGUMENT_FLAG("--help",
          {goto _main_usage_good;})

        MAIN_ADD_ARGUMENT_FLAG("--no-clock",
          {args->clock_fill = NULL;})

    MAIN_ADD_ARGUMENT_END


    // printf("__file__=!%s!\n", args->__file__);
    // printf("pwdfile=!%s!\n", args->passwd_file);
    // printf("clock-fill=!%s!\n", args->clock_fill);
    // printf("clock-pos=!%i,%i!\n", args->clock_pos.row,
    //                               args->clock_pos.col);
    // printf("clock-font=!%i!\n", args->clock_font[0][0]);
    // printf("clock-scale=!%u!\n", args->clock_scale);
    // printf("clock-colour=!%u!\n", args->clock_colour);
    // printf("clock-sleep-colour=!%u!\n", args->clock_sleep_colour);
    // printf("clock-sleep-time=!%i!\n", args->clock_sleep_time);
    // printf("clock-fps=!%u!\n", args->clock_fps);

    exit_code = actual_main(args);
    goto _main_cleanup;

    _main_usage_good: /* -h/--help */
        usage(args);
        goto _main_cleanup;

    _main_unknown_arg: /* Unknown argument */
        error("Unknown argument: "GREEN"%s"RED, arg);
        goto _main_bad;

    _main_invalid_value: /* Known argument with bad value */
        error("Invalid value for: "GREEN"%s"RED" argument", arg);
        goto _main_bad;

    _main_unfinished_arg: /* Non-flag argument without value */
        error("Unfinished argument: "GREEN"%s"RED" argument", arg);
        goto _main_bad;

    _main_bad: /* General problem with argument */
        error("Run "GREEN"%s"RED" "GREEN"--help"RED" for help", args->__file__);
        exit_code = 1;
        goto _main_cleanup;

    _main_cleanup: /* Cleanup and exit */
        free(args);
        return exit_code;
}


bool _not_pressed_ctrl_c;
void _pressed_ctrl_c(int signo) { (void)signo; _not_pressed_ctrl_c = false; }

void sleep_milli(unsigned int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000L * 1000L;
    nanosleep(&ts, NULL);
}


// This is where the actual main code is:
ExitCode actual_main(Args* args) {
    // Password
    if (!chk_passwd_set(args->passwd_file)) {
        if (!set_passwd("New password: ", args->passwd_file)) {
            error("Couldn't set password.\n"
                  "Bailing out without locking.");
            return 1;
        }
        if (!chk_passwd_set(args->passwd_file)) {
            error("Set password worked but password still invalid.\n"
                  "Bailing out without locking.");
            return 2;
        }
    }
    puts(YELLOW"Make sure there are no background jobs on this tty."RESET);
    puts(CYAN"To stop you from locking yourself out, please enter your " \
         "password"RESET);
    if (!chk_passwd("Unlock password: ", args->passwd_file)) {
        error("Password incorrect.\n"
              "Bailing out without locking.");
        return 3;
    }

    // Lock
    if (!lock_console_switch()) {
        error("Can't lock console switching.\n"
              "Bailing out without locking.");
        #if !defined(DEBUG)
            return 4;
        #endif
    }
    if (!lock_signals()) {
        error("Can't block signals.\n"
              "Bailing out without locking.");
        return 5;
    }
    bool _always_pressed_ctrl_c = !set_signal(SIGINT, _pressed_ctrl_c);

    // Read loop
    ExitCode exit_code = 0;
    uint32_t incorrect_pwds = 0;
    while (true) {
        // Wait for Control-c, displaying the clock in the meantime
        _not_pressed_ctrl_c = true;
        screen_clear();
        display_time_init();
        uint8_t colour = args->clock_colour;
        int32_t timer = args->clock_sleep_time * args->clock_fps;
        const char* fill = args->clock_fill;
        while (_not_pressed_ctrl_c && (!_always_pressed_ctrl_c)) {
            if (args->clock_fill != NULL) {
                if (timer == 0) {
                    colour = args->clock_sleep_colour;
                    fill = args->clock_sleep_fill;
                    timer++; // Cancel out the ++ bellow
                }
                timer--;
                Bytes keypresses = keyboard_flush_stdin();
                if (keypresses.len != 0) {
                    timer = args->clock_sleep_time * args->clock_fps;
                    colour = args->clock_colour;
                    fill = args->clock_fill;
                }
                // For initramfs tty (Control-C doesn't work)
                for (size_t i=0; i<keypresses.len; i++) {
                    if (keypresses.data[i] == 3) { // Control-C
                        _not_pressed_ctrl_c = false;
                    } else if (keypresses.data[i] == 10) { // "\n"
                        _not_pressed_ctrl_c = false;
                    } else if (keypresses.data[i] == 27) { // "\x1b"
                        if (i+1 == keypresses.len) { // Last char => Esc
                            _not_pressed_ctrl_c = false;
                        } else if (keypresses.data[i+1] != 91) { // Escape
                            _not_pressed_ctrl_c = false;
                        }
                    }
                }
                bytes_free(&keypresses);
                display_time(fill, args->clock_font, args->clock_pos,
                             args->clock_scale, colour);
            }
            cursor_move(1, 1);
            printf("\x1b[K\x1b[0K\x1b[2K");
            printf(CYAN"Press [Enter] to unlock. "RESET);
            fflush(stdout);
            sleep_milli((unsigned int) 1000/args->clock_fps);
        }

        cursor_move(1, 1);
        screen_clear();
        if (!chk_passwd_set(args->passwd_file)) {
            error("Password was set and working but now is invalid.\n"
                  "Bailing out after unlocking.");
            exit_code = 6;
            break;
        }
        if (chk_passwd("Password: ", args->passwd_file)) { break; }
        incorrect_pwds++;
        error("Incorrect password!");
        keyboard_wait_for_enter(YELLOW"Press [Enter] to try again. "RESET);
    }

    // Inform user of the number of incorrect password tries
    if (incorrect_pwds) {
        printf(YELLOW"%i incorrect password tries."RESET"\n", incorrect_pwds);
    }

    // Unlock
    if (!unlock_console_switch()) {
        error("Sorry, can't unlock console switching.\n"
              "Good luck "GREEN":D"RED"\n"
              "Continuing without unlocking.");
        exit_code = 7;
    }
    return exit_code;
}
