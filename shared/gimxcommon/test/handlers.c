/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <signal.h>

#ifdef WIN32
#include <windows.h>
#endif

volatile int done = 0;

static int is_done() {

    return done;
}

static void set_done() {

    done = 1;
}

static void terminate(int sig __attribute__((unused))) {

    done = 1;
}

#ifdef WIN32
static BOOL WINAPI ConsoleHandler(DWORD dwType) {

    switch (dwType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:

        done = 1; //signal the main thread to terminate

        //Returning would make the process exit!
        //We just make the handler sleep until the main thread exits,
        //or until the maximum execution time for this handler is reached.
        Sleep(10000);

        return TRUE;
    default:
        break;
    }
    return FALSE;
}
#endif

static void setup_handlers() {

    (void) signal(SIGINT, terminate);
    (void) signal(SIGTERM, terminate);
#ifndef WIN32
    (void) signal(SIGHUP, terminate);
#else
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == 0)
    {
      fprintf(stderr, "SetConsoleCtrlHandler failed\n");
      exit(-1);
    }
  #endif
}
