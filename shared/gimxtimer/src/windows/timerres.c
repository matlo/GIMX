/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "timerres.h"
#include <windows.h>
#include <gimxcommon/include/gerror.h>

static void (__stdcall *pNtQueryTimerResolution)(PULONG, PULONG, PULONG) = NULL;
static void (__stdcall *pNtSetTimerResolution)(ULONG, BOOL, PULONG) = NULL;

static int nb_users = 0;
static HANDLE hTimer = INVALID_HANDLE_VALUE;
static GPOLL_REGISTER_SOURCE fp_register = NULL;
static GPOLL_REMOVE_SOURCE fp_remove = NULL;
static int (*timer_callback)(unsigned int) = NULL;
static ULONG currentResolution = 0;
static LARGE_INTEGER last = {};
static LARGE_INTEGER next = {};

static inline LARGE_INTEGER timerres_get_time() {
  FILETIME ftime;
  GetSystemTimeAsFileTime(&ftime);
  LARGE_INTEGER li = { .HighPart = ftime.dwHighDateTime, .LowPart = ftime.dwLowDateTime };
  return li;
}

void timerres_init(void) __attribute__((constructor));
void timerres_init(void) {

    HMODULE hNtdll = GetModuleHandle("ntdll.dll");
    if (hNtdll == INVALID_HANDLE_VALUE) {
        PRINT_ERROR_GETLASTERROR("GetModuleHandle ntdll.dll")
        exit(-1);
    }
    pNtQueryTimerResolution = (void (__stdcall *)(PULONG, PULONG, PULONG)) GetProcAddress(hNtdll,
            "NtQueryTimerResolution");
    if (pNtQueryTimerResolution == NULL) {
        PRINT_ERROR_GETLASTERROR("GetProcAddress NtQueryTimerResolution")
        exit(-1);
    }
    pNtSetTimerResolution = (void (__stdcall *)(ULONG, BOOL, PULONG)) GetProcAddress(hNtdll, "NtSetTimerResolution");
    if (pNtSetTimerResolution == NULL) {
        PRINT_ERROR_GETLASTERROR("GetProcAddress NtSetTimerResolution")
        exit(-1);
    }

    hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (hTimer == INVALID_HANDLE_VALUE) {
        PRINT_ERROR_GETLASTERROR("CreateWaitableTimer")
        exit(-1);
    }
}

void timerres_quit(void) __attribute__((destructor));
void timerres_quit(void) {

    CloseHandle(hTimer);
}

static int close_callback(int unused __attribute__((unused))) {

    return -1;
}

#define unlikely(x)    __builtin_expect(!!(x), 0)

// Warning: preemption may happen anytime, and timer_callback may take some time.
// Reset the timer until no period elapsed.
static int read_callback(int unused __attribute__((unused))) {

    int ret = 0;
    unsigned int nexp = (timerres_get_time().QuadPart - last.QuadPart) / currentResolution;
    do {
        int lret = timer_callback(nexp);
        if (lret < 0) {
            ret = -1;
        } else if (ret != -1 && lret) {
            ret = 1;
        }
        last.QuadPart += nexp * currentResolution;
        next.QuadPart += nexp * currentResolution;
        if (unlikely(!SetWaitableTimer(hTimer, &next, 0, NULL, NULL, FALSE))) {
            PRINT_ERROR_GETLASTERROR("SetWaitableTimer")
            return -1;
        }
    } while((nexp = (timerres_get_time().QuadPart - last.QuadPart) / currentResolution) > 0);

    return ret;
}

static int start_timer() {

    last = timerres_get_time();

    next.QuadPart = last.QuadPart + currentResolution;

    if (!SetWaitableTimer(hTimer, &next, 0, NULL, NULL, FALSE)) {
        PRINT_ERROR_GETLASTERROR("SetWaitableTimer")
        return -1;
    }

    GPOLL_CALLBACKS callbacks = {
      .fp_read = read_callback,
      .fp_write = NULL,
      .fp_close = close_callback,
    };
    int ret = fp_register(hTimer, 0, &callbacks);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

void timerres_end() {

    if (nb_users > 0) {
        --nb_users;
        if (nb_users == 0) {
            pNtSetTimerResolution(0, FALSE, &currentResolution);
            fp_remove(hTimer);
        }
    }
}

#define XSTR(s) STR(s)
#define STR(s) #s

#define CHECK_FUNCTION(FUNCTION) \
    if (FUNCTION == NULL) { \
        PRINT_ERROR_OTHER(XSTR(FUNCTION)" is NULL") \
        return -1; \
    }

int timerres_begin(const GPOLL_INTERFACE * poll_interface, TIMERRES_CALLBACK timer_cb) {

    CHECK_FUNCTION (poll_interface->fp_register)
    CHECK_FUNCTION (poll_interface->fp_remove)
    CHECK_FUNCTION (timer_cb)

    // TODO MLA: warn if register / remove functions change
    // on Windows function pointers to gpoll_register/remove inside and outside the dll do not match.

    int ret = 0;

    ++nb_users;

    if (nb_users == 1) {

        ULONG minimumResolution, maximumResolution;
        pNtQueryTimerResolution(&minimumResolution, &maximumResolution, &currentResolution);

        pNtSetTimerResolution(maximumResolution, TRUE, &currentResolution);

        printf("Timer resolution: min=%lu max=%lu current=%lu\n", minimumResolution, maximumResolution, currentResolution);

        timer_callback = timer_cb;
        fp_register = poll_interface->fp_register;
        fp_remove = poll_interface->fp_remove;
        if (start_timer() < 0) {
            ret = -1;
        }
    }

    if (ret == -1) {
        timerres_end();
    }

    return currentResolution;
}
