/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "timerres.h"
#include <windows.h>
#include <gerror.h>

static void (__stdcall *pNtQueryTimerResolution)(PULONG, PULONG, PULONG) = NULL;
static void (__stdcall *pNtSetTimerResolution)(ULONG, BOOL, PULONG) = NULL;

static int nb_users = 0;
static HANDLE hTimer = INVALID_HANDLE_VALUE;
static int (*register_handle)(HANDLE handle, int id, int (*fp_read)(int user), int (*fp_write)(int user),
        int (*fp_close)(int user)) = NULL;
static void (*remove_handle)(HANDLE handle);
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

static int close_callback(int unused) {

    return -1;
}

#define unlikely(x)    __builtin_expect(!!(x), 0)

// Warning: preemption may happen anytime, and timer_callback may take some time.
// Reset the timer until no period elapsed.
static int read_callback(int unused) {

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

    int ret = register_handle(hTimer, 0, read_callback, NULL, close_callback);
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
            remove_handle(hTimer);
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

int timerres_begin(TIMERRES_REGISTER_HANDLE fp_register, TIMERRES_REMOVE_HANDLE fp_remove, TIMERRES_CALLBACK timer_cb) {

    CHECK_FUNCTION (fp_register)
    CHECK_FUNCTION (fp_remove)
    CHECK_FUNCTION (timer_cb)

    int ret = 0;

    ++nb_users;

    if (nb_users == 1) {

        ULONG minimumResolution, maximumResolution;
        pNtQueryTimerResolution(&minimumResolution, &maximumResolution, &currentResolution);

        pNtSetTimerResolution(maximumResolution, TRUE, &currentResolution);

        printf("Timer resolution: min=%lu max=%lu current=%lu\n", minimumResolution, maximumResolution, currentResolution);

        timer_callback = timer_cb;
        register_handle = fp_register;
        remove_handle = fp_remove;
        if (start_timer() < 0) {
            ret = -1;
        }
    }

    if (ret == -1) {
        timerres_end();
    }

    return currentResolution;
}
