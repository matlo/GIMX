/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GE_H_
#define GE_H_

#include <stdint.h>

#define GE_MAX_DEVICES 256

#define GE_MOUSE_BUTTONS_MAX 12

#ifdef WIN32
#include "GE_Windows.h"
typedef void* HANDLE;
#else
#include "GE_Linux.h"
#include <sys/time.h>
#endif

#define GE_MKB_SOURCE_NONE 0
#define GE_MKB_SOURCE_PHYSICAL 1
#define GE_MKB_SOURCE_WINDOW_SYSTEM 2

#define GE_GRAB_OFF 0
#define GE_GRAB_ON 1

#ifndef KEY_MICMUTE
#define KEY_MICMUTE 248
#endif

#define GE_KEY_ESC  KEY_ESC
#define GE_KEY_1  KEY_1
#define GE_KEY_2  KEY_2
#define GE_KEY_3  KEY_3
#define GE_KEY_4  KEY_4
#define GE_KEY_5  KEY_5
#define GE_KEY_6  KEY_6
#define GE_KEY_7  KEY_7
#define GE_KEY_8  KEY_8
#define GE_KEY_9  KEY_9
#define GE_KEY_0  KEY_0
#define GE_KEY_MINUS  KEY_MINUS
#define GE_KEY_EQUAL  KEY_EQUAL
#define GE_KEY_BACKSPACE  KEY_BACKSPACE
#define GE_KEY_TAB  KEY_TAB
#define GE_KEY_Q  KEY_Q
#define GE_KEY_W  KEY_W
#define GE_KEY_E  KEY_E
#define GE_KEY_R  KEY_R
#define GE_KEY_T  KEY_T
#define GE_KEY_Y  KEY_Y
#define GE_KEY_U  KEY_U
#define GE_KEY_I  KEY_I
#define GE_KEY_O  KEY_O
#define GE_KEY_P  KEY_P
#define GE_KEY_LEFTBRACE  KEY_LEFTBRACE
#define GE_KEY_RIGHTBRACE   KEY_RIGHTBRACE
#define GE_KEY_ENTER  KEY_ENTER
#define GE_KEY_LEFTCTRL   KEY_LEFTCTRL
#define GE_KEY_A  KEY_A
#define GE_KEY_S  KEY_S
#define GE_KEY_D  KEY_D
#define GE_KEY_F  KEY_F
#define GE_KEY_G  KEY_G
#define GE_KEY_H  KEY_H
#define GE_KEY_J  KEY_J
#define GE_KEY_K  KEY_K
#define GE_KEY_L  KEY_L
#define GE_KEY_SEMICOLON  KEY_SEMICOLON
#define GE_KEY_APOSTROPHE   KEY_APOSTROPHE
#define GE_KEY_GRAVE  KEY_GRAVE
#define GE_KEY_LEFTSHIFT  KEY_LEFTSHIFT
#define GE_KEY_BACKSLASH  KEY_BACKSLASH
#define GE_KEY_Z  KEY_Z
#define GE_KEY_X  KEY_X
#define GE_KEY_C  KEY_C
#define GE_KEY_V  KEY_V
#define GE_KEY_B  KEY_B
#define GE_KEY_N  KEY_N
#define GE_KEY_M  KEY_M
#define GE_KEY_COMMA  KEY_COMMA
#define GE_KEY_DOT  KEY_DOT
#define GE_KEY_SLASH  KEY_SLASH
#define GE_KEY_RIGHTSHIFT   KEY_RIGHTSHIFT
#define GE_KEY_KPASTERISK   KEY_KPASTERISK
#define GE_KEY_LEFTALT  KEY_LEFTALT
#define GE_KEY_SPACE  KEY_SPACE
#define GE_KEY_CAPSLOCK   KEY_CAPSLOCK
#define GE_KEY_F1   KEY_F1
#define GE_KEY_F2   KEY_F2
#define GE_KEY_F3   KEY_F3
#define GE_KEY_F4   KEY_F4
#define GE_KEY_F5   KEY_F5
#define GE_KEY_F6   KEY_F6
#define GE_KEY_F7   KEY_F7
#define GE_KEY_F8   KEY_F8
#define GE_KEY_F9   KEY_F9
#define GE_KEY_F10  KEY_F10
#define GE_KEY_NUMLOCK  KEY_NUMLOCK
#define GE_KEY_SCROLLLOCK   KEY_SCROLLLOCK
#define GE_KEY_KP7  KEY_KP7
#define GE_KEY_KP8  KEY_KP8
#define GE_KEY_KP9  KEY_KP9
#define GE_KEY_KPMINUS  KEY_KPMINUS
#define GE_KEY_KP4  KEY_KP4
#define GE_KEY_KP5  KEY_KP5
#define GE_KEY_KP6  KEY_KP6
#define GE_KEY_KPPLUS   KEY_KPPLUS
#define GE_KEY_KP1  KEY_KP1
#define GE_KEY_KP2  KEY_KP2
#define GE_KEY_KP3  KEY_KP3
#define GE_KEY_KP0  KEY_KP0
#define GE_KEY_KPDOT  KEY_KPDOT

#define GE_KEY_ZENKAKUHANKAKU   KEY_ZENKAKUHANKAKU
#define GE_KEY_102ND  KEY_102ND
#define GE_KEY_F11  KEY_F11
#define GE_KEY_F12  KEY_F12
#define GE_KEY_RO   KEY_RO
#define GE_KEY_KATAKANA   KEY_KATAKANA
#define GE_KEY_HIRAGANA   KEY_HIRAGANA
#define GE_KEY_HENKAN   KEY_HENKAN
#define GE_KEY_KATAKANAHIRAGANA   KEY_KATAKANAHIRAGANA
#define GE_KEY_MUHENKAN   KEY_MUHENKAN
#define GE_KEY_KPJPCOMMA  KEY_KPJPCOMMA
#define GE_KEY_KPENTER  KEY_KPENTER
#define GE_KEY_RIGHTCTRL  KEY_RIGHTCTRL
#define GE_KEY_KPSLASH  KEY_KPSLASH
#define GE_KEY_SYSRQ  KEY_SYSRQ
#define GE_KEY_RIGHTALT   KEY_RIGHTALT
#define GE_KEY_LINEFEED   KEY_LINEFEED
#define GE_KEY_HOME   KEY_HOME
#define GE_KEY_UP   KEY_UP
#define GE_KEY_PAGEUP   KEY_PAGEUP
#define GE_KEY_LEFT   KEY_LEFT
#define GE_KEY_RIGHT  KEY_RIGHT
#define GE_KEY_END  KEY_END
#define GE_KEY_DOWN   KEY_DOWN
#define GE_KEY_PAGEDOWN   KEY_PAGEDOWN
#define GE_KEY_INSERT   KEY_INSERT
#define GE_KEY_DELETE   KEY_DELETE
#define GE_KEY_MACRO  KEY_MACRO
#define GE_KEY_MUTE   KEY_MUTE
#define GE_KEY_VOLUMEDOWN   KEY_VOLUMEDOWN
#define GE_KEY_VOLUMEUP   KEY_VOLUMEUP
#define GE_KEY_POWER  KEY_POWER
#define GE_KEY_KPEQUAL  KEY_KPEQUAL
#define GE_KEY_KPPLUSMINUS  KEY_KPPLUSMINUS
#define GE_KEY_PAUSE  KEY_PAUSE
#define GE_KEY_SCALE  KEY_SCALE

#define GE_KEY_KPCOMMA  KEY_KPCOMMA
#define GE_KEY_HANGEUL  KEY_HANGEUL
#define GE_KEY_HANGUEL  KEY_HANGEUL
#define GE_KEY_HANJA  KEY_HANJA
#define GE_KEY_YEN  KEY_YEN
#define GE_KEY_LEFTMETA   KEY_LEFTMETA
#define GE_KEY_RIGHTMETA  KEY_RIGHTMETA
#define GE_KEY_COMPOSE  KEY_COMPOSE

#define GE_KEY_STOP   KEY_STOP
#define GE_KEY_AGAIN  KEY_AGAIN
#define GE_KEY_PROPS  KEY_PROPS
#define GE_KEY_UNDO   KEY_UNDO
#define GE_KEY_FRONT  KEY_FRONT
#define GE_KEY_COPY   KEY_COPY
#define GE_KEY_OPEN   KEY_OPEN
#define GE_KEY_PASTE  KEY_PASTE
#define GE_KEY_FIND   KEY_FIND
#define GE_KEY_CUT  KEY_CUT
#define GE_KEY_HELP   KEY_HELP
#define GE_KEY_MENU   KEY_MENU
#define GE_KEY_CALC   KEY_CALC
#define GE_KEY_SETUP  KEY_SETUP
#define GE_KEY_SLEEP  KEY_SLEEP
#define GE_KEY_WAKEUP   KEY_WAKEUP
#define GE_KEY_FILE   KEY_FILE
#define GE_KEY_SENDFILE   KEY_SENDFILE
#define GE_KEY_DELETEFILE   KEY_DELETEFILE
#define GE_KEY_XFER   KEY_XFER
#define GE_KEY_PROG1  KEY_PROG1
#define GE_KEY_PROG2  KEY_PROG2
#define GE_KEY_WWW  KEY_WWW
#define GE_KEY_MSDOS  KEY_MSDOS
#define GE_KEY_COFFEE   KEY_COFFEE
#define GE_KEY_SCREENLOCK   KEY_COFFEE
#define GE_KEY_DIRECTION  KEY_DIRECTION
#define GE_KEY_CYCLEWINDOWS   KEY_CYCLEWINDOWS
#define GE_KEY_MAIL   KEY_MAIL
#define GE_KEY_BOOKMARKS  KEY_BOOKMARKS
#define GE_KEY_COMPUTER   KEY_COMPUTER
#define GE_KEY_BACK   KEY_BACK
#define GE_KEY_FORWARD  KEY_FORWARD
#define GE_KEY_CLOSECD  KEY_CLOSECD
#define GE_KEY_EJECTCD  KEY_EJECTCD
#define GE_KEY_EJECTCLOSECD   KEY_EJECTCLOSECD
#define GE_KEY_NEXTSONG   KEY_NEXTSONG
#define GE_KEY_PLAYPAUSE  KEY_PLAYPAUSE
#define GE_KEY_PREVIOUSSONG   KEY_PREVIOUSSONG
#define GE_KEY_STOPCD   KEY_STOPCD
#define GE_KEY_RECORD   KEY_RECORD
#define GE_KEY_REWIND   KEY_REWIND
#define GE_KEY_PHONE  KEY_PHONE
#define GE_KEY_ISO  KEY_ISO
#define GE_KEY_CONFIG   KEY_CONFIG
#define GE_KEY_HOMEPAGE   KEY_HOMEPAGE
#define GE_KEY_REFRESH  KEY_REFRESH
#define GE_KEY_EXIT   KEY_EXIT
#define GE_KEY_MOVE   KEY_MOVE
#define GE_KEY_EDIT   KEY_EDIT
#define GE_KEY_SCROLLUP   KEY_SCROLLUP
#define GE_KEY_SCROLLDOWN   KEY_SCROLLDOWN
#define GE_KEY_KPLEFTPAREN  KEY_KPLEFTPAREN
#define GE_KEY_KPRIGHTPAREN   KEY_KPRIGHTPAREN
#define GE_KEY_NEW  KEY_NEW
#define GE_KEY_REDO   KEY_REDO

#define GE_KEY_F13  KEY_F13
#define GE_KEY_F14  KEY_F14
#define GE_KEY_F15  KEY_F15
#define GE_KEY_F16  KEY_F16
#define GE_KEY_F17  KEY_F17
#define GE_KEY_F18  KEY_F18
#define GE_KEY_F19  KEY_F19
#define GE_KEY_F20  KEY_F20
#define GE_KEY_F21  KEY_F21
#define GE_KEY_F22  KEY_F22
#define GE_KEY_F23  KEY_F23
#define GE_KEY_F24  KEY_F24

#define GE_KEY_PLAYCD   KEY_PLAYCD
#define GE_KEY_PAUSECD  KEY_PAUSECD
#define GE_KEY_PROG3  KEY_PROG3
#define GE_KEY_PROG4  KEY_PROG4
#define GE_KEY_DASHBOARD  KEY_DASHBOARD
#define GE_KEY_SUSPEND  KEY_SUSPEND
#define GE_KEY_CLOSE  KEY_CLOSE
#define GE_KEY_PLAY   KEY_PLAY
#define GE_KEY_FASTFORWARD  KEY_FASTFORWARD
#define GE_KEY_BASSBOOST  KEY_BASSBOOST
#define GE_KEY_PRINT  KEY_PRINT
#define GE_KEY_HP   KEY_HP
#define GE_KEY_CAMERA   KEY_CAMERA
#define GE_KEY_SOUND  KEY_SOUND
#define GE_KEY_QUESTION   KEY_QUESTION
#define GE_KEY_EMAIL  KEY_EMAIL
#define GE_KEY_CHAT   KEY_CHAT
#define GE_KEY_SEARCH   KEY_SEARCH
#define GE_KEY_CONNECT  KEY_CONNECT
#define GE_KEY_FINANCE  KEY_FINANCE
#define GE_KEY_SPORT  KEY_SPORT
#define GE_KEY_SHOP   KEY_SHOP
#define GE_KEY_ALTERASE   KEY_ALTERASE
#define GE_KEY_CANCEL   KEY_CANCEL
#define GE_KEY_BRIGHTNESSDOWN   KEY_BRIGHTNESSDOWN
#define GE_KEY_BRIGHTNESSUP   KEY_BRIGHTNESSUP
#define GE_KEY_MEDIA  KEY_MEDIA

#define GE_KEY_SWITCHVIDEOMODE  KEY_SWITCHVIDEOMODE
#define GE_KEY_KBDILLUMTOGGLE   KEY_KBDILLUMTOGGLE
#define GE_KEY_KBDILLUMDOWN   KEY_KBDILLUMDOWN
#define GE_KEY_KBDILLUMUP   KEY_KBDILLUMUP

#define GE_KEY_SEND   KEY_SEND
#define GE_KEY_REPLY  KEY_REPLY
#define GE_KEY_FORWARDMAIL  KEY_FORWARDMAIL
#define GE_KEY_SAVE   KEY_SAVE
#define GE_KEY_DOCUMENTS  KEY_DOCUMENTS

#define GE_KEY_BATTERY  KEY_BATTERY

#define GE_KEY_BLUETOOTH  KEY_BLUETOOTH
#define GE_KEY_WLAN   KEY_WLAN
#define GE_KEY_UWB  KEY_UWB

#define GE_KEY_UNKNOWN  KEY_UNKNOWN

#define GE_KEY_VIDEO_NEXT   KEY_VIDEO_NEXT
#define GE_KEY_VIDEO_PREV   KEY_VIDEO_PREV
#define GE_KEY_BRIGHTNESS_CYCLE   KEY_BRIGHTNESS_CYCLE
#define GE_KEY_BRIGHTNESS_ZERO  KEY_BRIGHTNESS_ZERO
#define GE_KEY_DISPLAY_OFF  KEY_DISPLAY_OFF

#define GE_KEY_WIMAX  KEY_WIMAX
#define GE_KEY_RFKILL   KEY_RFKILL

#define GE_KEY_MICMUTE  KEY_MICMUTE

typedef enum {
       GE_NOEVENT = 0,     /**< Unused (do not remove) */
       GE_KEYDOWN,     /**< Keys pressed */
       GE_KEYUP,     /**< Keys released */
       GE_MOUSEMOTION,     /**< Mouse moved */
       GE_MOUSEBUTTONDOWN,   /**< Mouse button pressed */
       GE_MOUSEBUTTONUP,   /**< Mouse button released */
       GE_JOYAXISMOTION,   /**< Joystick axis motion */
       GE_JOYHATMOTION,    /**< Joystick hat position change */
       GE_JOYBUTTONDOWN,   /**< Joystick button pressed */
       GE_JOYBUTTONUP,     /**< Joystick button released */
       GE_JOYRUMBLE,     /**< Joystick rumble */
       GE_QUIT,     /**< Joystick button released */
} GE_EventType;

typedef struct GE_KeyboardEvent {
  uint8_t type; /**< GE_KEYDOWN or GE_KEYUP */
  uint8_t which;  /**< The keyboard device index */
  uint16_t keysym;
} GE_KeyboardEvent;

typedef struct GE_MouseMotionEvent {
  uint8_t type; /**< GE_MOUSEMOTION */
  uint8_t which;  /**< The mouse device index */
  int16_t xrel;  /**< The relative motion in the X direction */
  int16_t yrel;  /**< The relative motion in the Y direction */
} GE_MouseMotionEvent;

typedef struct GE_MouseButtonEvent {
  uint8_t type; /**< GE_MOUSEBUTTONDOWN or GE_MOUSEBUTTONUP */
  uint8_t which;  /**< The mouse device index */
  uint8_t button; /**< The mouse button index */
} GE_MouseButtonEvent;

typedef struct GE_JoyAxisEvent {
  uint8_t type; /**< GE_JOYAXISMOTION */
  uint8_t which;  /**< The joystick device index */
  uint8_t axis; /**< The joystick axis index */
  int16_t value; /**< The axis value (range: -32768 to 32767) */
} GE_JoyAxisEvent;

typedef struct GE_JoyHatEvent {
  uint8_t type; /**< GE_JOYHATMOTION */
  uint8_t which;  /**< The joystick device index */
  uint8_t hat;  /**< The joystick hat index */
  uint8_t value;  /**< The hat position value:
       *   GE_HAT_LEFTUP   GE_HAT_UP       GE_HAT_RIGHTUP
       *   GE_HAT_LEFT     GE_HAT_CENTERED GE_HAT_RIGHT
       *   GE_HAT_LEFTDOWN GE_HAT_DOWN     GE_HAT_RIGHTDOWN
       *  Note that zero means the POV is centered.
       */
} GE_JoyHatEvent;

typedef struct GE_JoyButtonEvent {
  uint8_t type; /**< GE_JOYBUTTONDOWN or GE_JOYBUTTONUP */
  uint8_t which;  /**< The joystick device index */
  uint8_t button; /**< The joystick button index */
  uint8_t state;  /**< GE_PRESSED or GE_RELEASED */
} GE_JoyButtonEvent;

typedef struct GE_JoyRumbleEvent {
  uint8_t type; /**< GE_JOYRUMBLE */
  uint8_t which;  /**< The joystick device index */
  uint16_t weak; /**< Weak motor */
  uint16_t strong;  /**< Strong motor */
} GE_JoyRumbleEvent;

typedef union GE_Event {
  struct
  {
    uint8_t type;
    uint8_t which;
  };
  GE_KeyboardEvent key;
  GE_MouseMotionEvent motion;
  GE_MouseButtonEvent button;
  GE_JoyAxisEvent jaxis;
  GE_JoyHatEvent jhat;
  GE_JoyButtonEvent jbutton;
  GE_JoyRumbleEvent jrumble;
} GE_Event;

typedef enum
{
  GE_MK_MODE_MULTIPLE_INPUTS,
  GE_MK_MODE_SINGLE_INPUT
} GE_MK_Mode;

typedef enum
{
  GE_JS_OTHER,
  GE_JS_SIXAXIS,
  GE_JS_DS4,
  GE_JS_360PAD,
  GE_JS_XONEPAD,
} GE_JS_Type;

#define EVENT_BUFFER_SIZE 256

#define AXIS_X 0
#define AXIS_Y 1

#define MOUSE_AXIS_X "x"
#define MOUSE_AXIS_Y "y"

#ifdef __cplusplus
extern "C" {
#endif

int GE_initialize(unsigned char mkb_src);
void GE_grab_toggle();
void GE_grab();
void GE_release_unused();
void GE_quit();
void GE_FreeMKames();

char* GE_MouseName(int);
char* GE_KeyboardName(int);
int GE_MouseVirtualId(int);
int GE_KeyboardVirtualId(int);
int GE_GetDeviceId(GE_Event*);
char* GE_JoystickName(int);
int GE_JoystickVirtualId(int);
void GE_SetJoystickUsed(int);
GE_JS_Type GE_GetJSType(int id);
int GE_RegisterJoystick(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short));

const char* GE_MouseButtonName(int);
int GE_MouseButtonId(const char*);
const char* GE_KeyName(uint16_t);
uint16_t GE_KeyId(const char*);

GE_MK_Mode GE_GetMKMode();
void GE_SetMKMode(GE_MK_Mode);

int GE_JoystickHasRumble(int id);
int GE_JoystickSetRumble(int id, unsigned short weak, unsigned short strong);

void GE_TimerStart(int usec);
void GE_TimerClose();
void GE_PumpEvents();
int GE_PeepEvents(GE_Event*, int);
int GE_PushEvent(GE_Event*);
void GE_SetCallback(int(*)(GE_Event*));

void GE_AddSource(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
#ifdef WIN32
void GE_AddSourceHandle(HANDLE handle, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int));
void GE_RemoveSourceHandle(HANDLE handle);
int GE_JoystickGetUsbIds(int id, unsigned short * vendor, unsigned short * product);
#else
int GE_JoystickGetUHidId(int id);
#endif
void GE_RemoveSource(int fd);

#ifdef __cplusplus
}
#endif

#endif /* GE_H_ */
