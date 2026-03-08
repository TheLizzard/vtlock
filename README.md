# vtlock

![`vtlock` showing the screensaver clock with "Press \[Enter\] to unlock" in the top-left](screenshot.png)

`vtlock` is a small, terminal-based utility for POSIX.1-2008 that locks a virtual terminal (TTY) until the correct password is entered. It’s designed to be simple, secure and self-contained.


## Features

- Locks the current TTY and prevents switching to other virtual terminals
- Disables exit signals like `SIGINT` (`Ctrl-C`) and `SIGTSTP` (`Ctrl-Z`)
- Displays a simple clock screensaver while locked
- Statically linked (no runtime dependencies required)


## Build

`vtlock` is written in pure C only requiring OpenSSL and can be compiled with both `gcc` and `clang` using `make`.


## Usage

```bash
Usage: ./vtlock [--pwdfile <path>] [--no-time] [--clock-fill <+ve integer>/<character>] [--clock-font <+ve integer>] [--clock-row <uinteger>] [--clock-col <uinteger>] [--clock-scale <+ve integer>] [--clock-fps <+ve integer>] [--clock-colour <uinteger>] [--clock-sleep-colour <uinteger>] [--clock-sleep-time <integer>] [--clock-sleep-fill <+ve integer>/<character>]
    --pwdfile               : The password file (default: vtlock-passwd)
    --no-clock              : Disable the clock
    --clock-font            : The font to use for clock (the numbers between 1 and 6) (default: 6)
    --clock-fill            : The character to use for the clock (1:# 2:░ 3:▒ 4:▓ 5:█) (default: 5)
    --clock-row             : Where to draw the clock (0 means center) (default: 0)
    --clock-col             : Where to draw the clock (0 means center) (default: 0)
    --clock-scale           : The scale of the clock (default: 3)
    --clock-fps             : How many times should the clock be redrawn per second (default: 10)
    --clock-colour          : The colour of the clock (using ANSI escape codes) (default: 0 => \x1b[0m)
    --clock-sleep-colour    : Same as --clock-colour but when sleeping (default: 90 => \x1b[90m)
    --clock-sleep-time      : How long the clock should wait to switch from --clock-colour to --clock-sleep-colour (in seconds) (-1 means never sleep) (default: 30)
    --clock-sleep-fill      : The fill when sleeping. Same options as --clock-fill (default: 2)
    -h --help               : Show this help message
```

1. Run `vtlock` in a tty
2. Enter the lock password to start.
3. Once locked, the terminal cannot be exited using typical signals.
4. To unlock, enter the correct password (number of incorrect attempts are reported).


## Security Notes
* Passwords are stored as salted SHA-512 hashes
* `vtlock` doesn't require root
