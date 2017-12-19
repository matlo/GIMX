/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GIMX_H_
#define GIMX_H_

#include <gimxinput/include/ginput.h>
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

typedef enum {
    E_GIMX_STATUS_SUCCESS = 0,

    E_GIMX_STATUS_GENERIC_ERROR = -1,

    E_GIMX_STATUS_ADAPTER_NOT_DETECTED = -2, // wiring issue, incorrect firmware, target not powered
    E_GIMX_STATUS_NO_ACTIVATION = -3, // user did not activate the controller
    E_GIMX_STATUS_INACTIVITY_TIMEOUT = -4, // no user input during defined time

    E_GIMX_STATUS_AUTH_MISSING_X360 = 1, // auth source missing
    E_GIMX_STATUS_AUTH_MISSING_PS4 = 2, // auth source missing
    E_GIMX_STATUS_AUTH_MISSING_XONE = 3, // auth source missing
} e_gimx_status;

#define MAX_CONTROLLERS 7
#define MAX_PROFILES 8
#define MAX_DEVICES 256
#define MAX_CONTROLS 256

/*
 * Controllers are listening from TCP_PORT to TCP_PORT+MAX_CONTROLLERS-1
 */
#define TCP_PORT 21313

typedef struct
{
  char* homedir;
  int force_updates;
  char* keygen;
  int grab;
  int refresh_period;
  double frequency_scale;
  int status;
  int curses;
  int curses_status; // 1 = started
  struct {
      int haptic;
      int controller;
      int macros;
      int sixaxis;
  } debug;
  char* config_file;
  int postpone_count;
  int subpositions;
  int window_events;
  int network_input;
  int btstack;
  char * logfilename;
  FILE * logfile;
  int skip_leds;
  int ff_conv;
  unsigned int inactivity_timeout; // minutes, 0 means not defined
} s_gimx_params;

extern s_gimx_params gimx_params;

enum {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GREY,
    DARKGREY,
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

#if defined(_WIN32)
static inline int saveDefaultColorStdout() {
    static char initialized = 0;
    static WORD attributes;
    if (!initialized) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        attributes = csbi.wAttributes;
        initialized = 1;
    }
    return (int)attributes;
}
static inline int saveDefaultColorStderr() {
    static char initialized = 0;
    static WORD attributes;
    if (!initialized) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &csbi);
        attributes = csbi.wAttributes;
        initialized = 1;
    }
    return (int)attributes;
}
static inline void setColorStdout(int c) {
    saveDefaultColorStdout();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SetConsoleTextAttribute(hConsole, (csbi.wAttributes & 0xFFF0) | (WORD)c); // Foreground colors take up the least significant byte
}
static inline void setColorStderr(int c) {
    saveDefaultColorStderr();
    HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SetConsoleTextAttribute(hConsole, (csbi.wAttributes & 0xFFF0) | (WORD)c); // Foreground colors take up the least significant byte
}
static inline void resetColorStdout(void) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)saveDefaultColorStdout());
}
static inline void resetColorStderr(void) {
    SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), (WORD)saveDefaultColorStderr());
}

#else
static const char * ANSI_ATTRIBUTE_RESET    = "\033[0m";
static const char * ANSI_BLACK              = "\033[22;30m";
static const char * ANSI_RED                = "\033[22;31m";
static const char * ANSI_GREEN              = "\033[22;32m";
static const char * ANSI_BROWN              = "\033[22;33m";
static const char * ANSI_BLUE               = "\033[22;34m";
static const char * ANSI_MAGENTA            = "\033[22;35m";
static const char * ANSI_CYAN               = "\033[22;36m";
static const char * ANSI_GREY               = "\033[22;37m";
static const char * ANSI_DARKGREY           = "\033[01;30m";
static const char * ANSI_LIGHTRED           = "\033[01;31m";
static const char * ANSI_LIGHTGREEN         = "\033[01;32m";
static const char * ANSI_YELLOW             = "\033[01;33m";
static const char * ANSI_LIGHTBLUE          = "\033[01;34m";
static const char * ANSI_LIGHTMAGENTA       = "\033[01;35m";
static const char * ANSI_LIGHTCYAN          = "\033[01;36m";
static const char * ANSI_WHITE              = "\033[01;37m";

static inline const char * getANSIColor(const int c) {
    switch (c) {
        case BLACK       : return ANSI_BLACK;
        case BLUE        : return ANSI_BLUE; // non-ANSI
        case GREEN       : return ANSI_GREEN;
        case CYAN        : return ANSI_CYAN; // non-ANSI
        case RED         : return ANSI_RED; // non-ANSI
        case MAGENTA     : return ANSI_MAGENTA;
        case BROWN       : return ANSI_BROWN;
        case GREY        : return ANSI_GREY;
        case DARKGREY    : return ANSI_DARKGREY;
        case LIGHTBLUE   : return ANSI_LIGHTBLUE; // non-ANSI
        case LIGHTGREEN  : return ANSI_LIGHTGREEN;
        case LIGHTCYAN   : return ANSI_LIGHTCYAN; // non-ANSI;
        case LIGHTRED    : return ANSI_LIGHTRED; // non-ANSI;
        case LIGHTMAGENTA: return ANSI_LIGHTMAGENTA;
        case YELLOW      : return ANSI_YELLOW; // non-ANSI
        case WHITE       : return ANSI_WHITE;
        default: return "";
    }
}
static inline void setColorStdout(int c) {
    printf("%s", getANSIColor(c));
}
static inline void setColorStderr(int c) {
    fprintf(stderr, "%s", getANSIColor(c));
}
static inline void resetColorStdout(void) {
    printf("%s", ANSI_ATTRIBUTE_RESET);
}
static inline void resetColorStderr(void) {
    fprintf(stderr, "%s", ANSI_ATTRIBUTE_RESET);
}
#endif

#define gstatus(...) \
    if(gimx_params.status) { \
        printf(__VA_ARGS__); \
    }

#define ginfo(...) \
    if(!gimx_params.curses_status) { \
        printf(__VA_ARGS__); \
    }

#define gwarn(...) \
    if(!gimx_params.curses_status) { \
        setColorStdout(YELLOW); \
        printf("Warning: "); \
        resetColorStdout(); \
        printf(__VA_ARGS__); \
    }

#define gerror(...) \
    setColorStderr(LIGHTRED); \
    fprintf(stderr, "Error: "); \
    resetColorStderr(); \
    fprintf(stderr, __VA_ARGS__);

int process_event(GE_Event*);
int ignore_event(GE_Event*);

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif

#endif /* GIMX_H_ */
