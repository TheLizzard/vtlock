/*
This file was partially taken from
    git: https://github.com/viric/vlock.git
    commit: a648caf87c4c25b6df742b83ae383cebe6548f6b
    file: /src/console_switch.c
*/
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    #include <sys/consio.h>
#else
    #include <sys/vt.h>
#endif

#include "console_switch.h"


typedef struct vt_mode VTMode;
typedef struct sigaction Signal;
typedef int Sig;


static void  no_vt_switch(Sig sig) {
    (void) sig;
    ioctl(0, VT_RELDISP,         0);
}
static void ack_vt_switch(Sig sig) {
    (void) sig;
    ioctl(0, VT_RELDISP, VT_ACKACQ);
}

#define _new_signal(handler) ({ \
    Signal _tmp_signal; \
    sigemptyset(&_tmp_signal.sa_mask); \
    _tmp_signal.sa_handler = handler; \
    _tmp_signal.sa_flags = SA_RESTART; \
    _tmp_signal; \
})


static VTMode old_vtm;

Success lock_console_switch() {
    Success success = (ioctl(0, VT_GETMODE, &old_vtm) == 0);
    if (!success) { perror("get_current(vt_mode) failed"); return success; }

    VTMode new_vtm = old_vtm;
    new_vtm.mode = VT_PROCESS;
    new_vtm.relsig = SIGUSR1;
    new_vtm.acqsig = SIGUSR2;
    new_vtm.frsig = SIGHUP;

    {
        Signal signal = _new_signal(no_vt_switch);
        sigaction(SIGUSR1, &signal, NULL);
    }
    {
        Signal signal = _new_signal(ack_vt_switch);
        sigaction(SIGUSR2, &signal, NULL);
    }

    success &= (ioctl(0, VT_SETMODE, &new_vtm) == 0);
    if (!success) { perror("set(vt_mode) failed"); }
    return success;
}

Success unlock_console_switch() {
    Success success = (ioctl(0, VT_SETMODE, &old_vtm) == 0);
    if (!success) { perror("set(vt_mode) failed"); }
    return success;
}


Success set_signal(int sig, void (*func)(int)) {
    bool success = false;
    if (signal(sig,func) != SIG_ERR) {
        success = true;
    }
    Signal sa = _new_signal(func);
    if (sigaction(sig, &sa, NULL) != 0) {
        printf("sigaction(<signal>) failed on signal: %i", sig);
        perror("");
        if (!success) { return false; }
    }
    return true;
}

int SIGNALS[] = {
                  SIGTSTP, SIGTTIN, SIGTTOU, /* Job control */
                  SIGINT, SIGQUIT, /* Interactive signals */
                  SIGTERM, SIGHUP, /* Kill signals */
                  SIGPIPE, SIGALRM, /* Disruptive signals */
                  SIGCHLD, /* Note really needed but */
                  SIGKILL, /* Sentinel value */
                };

Success lock_signals() {
    Success success = true;
    for (int i=0; SIGNALS[i]!=SIGKILL; i++) {
        success &= set_signal(SIGNALS[i], SIG_IGN);
    }
    return success;
}
