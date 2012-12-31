/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "conversion.h"
#include <string.h>
#include "events.h"

typedef struct
{
  char arg[MAX_NAME_LENGTH];
  SDLKey key;
} s_arg_key;

char* keynames[249] =
{
    "RESERVED",
    "ESCAPE",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "MINUS",
    "EQUALS",
    "BACKSPACE",
    "TAB",
    "q",
    "w",
    "e",
    "r",
    "t",
    "y",
    "u",
    "i",
    "o",
    "p",
    "LEFTBRACKET",
    "RIGHTBRACKET",
    "RETURN",
    "LCTRL",
    "a",
    "s",
    "d",
    "f",
    "g",
    "h",
    "j",
    "k",
    "l",
    "SEMICOLON",
    "QUOTE",
    "BACKQUOTE",
    "LSHIFT",
    "BACKSLASH",
    "z",
    "x",
    "c",
    "v",
    "b",
    "n",
    "m",
    "COMMA",
    "PERIOD",
    "SLASH",
    "RSHIFT",
    "KP_MULTIPLY",
    "LALT",
    "SPACE",
    "CAPSLOCK",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "NUMLOCK",
    "SCROLLOCK",
    "KP7",
    "KP8",
    "KP9",
    "KP_MINUS",
    "KP4",
    "KP5",
    "KP6",
    "KP_PLUS",
    "KP1",
    "KP2",
    "KP3",
    "KP0",
    "KP_PERIOD",
    "UNDEFINED",
    "ZENKAKUHANKAKU",
    "102ND",
    "F11",
    "F12",
    "RO",
    "KATAKANA",
    "HIRAGANA",
    "HENKAN",
    "KATAKANAHIRAGANA",
    "MUHENKAN",
    "KPJPCOMMA",
    "KP_ENTER",
    "RCTRL",
    "KP_DIVIDE",
    "PRINT",
    "RALT",
    "LINEFEED",
    "HOME",
    "UP",
    "PAGEUP",
    "LEFT",
    "RIGHT",
    "END",
    "DOWN",
    "PAGEDOWN",
    "INSERT",
    "DELETE",
    "MACRO",
    "MUTE",
    "VOLUMEDOWN",
    "VOLUMEUP",
    "POWER",
    "KP_EQUAL",
    "KP_PLUSMINUS",
    "PAUSE",
    "SCALE",
    "KP_COMMA",
    "HANGEUL",
    "HANJA",
    "YEN",
    "LSUPER",
    "RSUPER",
    "MENU",
    "STOP",
    "AGAIN",
    "PROPS",
    "UNDO",
    "FRONT",
    "COPY",
    "OPEN",
    "PASTE",
    "FIND",
    "CUT",
    "HELP",
    "MENU",
    "CALC",
    "SETUP",
    "SLEEP",
    "WAKEUP",
    "FILE",
    "SENDFILE",
    "DELETEFILE",
    "XFER",
    "PROG1",
    "PROG2",
    "WWW",
    "MSDOS",
    "SCREENLOCK",
    "DIRECTION",
    "CYCLEWINDOWS",
    "MAIL",
    "BOOKMARKS",
    "COMPUTER",
    "BACK",
    "FORWARD",
    "CLOSECD",
    "EJECTCD",
    "EJECTCLOSECD",
    "NEXTSONG",
    "PLAYPAUSE",
    "PREVIOUSSONG",
    "STOPCD",
    "RECORD",
    "REWIND",
    "PHONE",
    "ISO",
    "CONFIG",
    "HOMEPAGE",
    "REFRESH",
    "EXIT",
    "MOVE",
    "EDIT",
    "SCROLLUP",
    "SCROLLDOWN",
    "KPLEFTPAREN",
    "KPRIGHTPAREN",
    "NEW",
    "REDO",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "UNDEFINED",
    "PLAYCD",
    "PAUSECD",
    "PROG3",
    "PROG4",
    "DASHBOARD",
    "SUSPEND",
    "CLOSE",
    "PLAY",
    "FASTFORWARD",
    "BASSBOOST",
    "PRINT",
    "HP",
    "CAMERA",
    "SOUND",
    "QUESTION",
    "EMAIL",
    "CHAT",
    "SEARCH",
    "CONNECT",
    "FINANCE",
    "SPORT",
    "SHOP",
    "ALTERASE",
    "CANCEL",
    "BRIGHTNESSDOWN",
    "BRIGHTNESSUP",
    "MEDIA",
    "SWITCHVIDEOMODE",
    "KBDILLUMTOGGLE",
    "KBDILLUMDOWN",
    "KBDILLUMUP",
    "SEND",
    "REPLY",
    "FORWARDMAIL",
    "SAVE",
    "DOCUMENTS",
    "BATTERY",
    "BLUETOOTH",
    "WLAN",
    "UWB",
    "UNKNOWN",
    "VIDEO_NEXT",
    "VIDEO_PREV",
    "BRIGHTNESS_CYCLE",
    "BRIGHTNESS_ZERO",
    "DISPLAY_OFF",
    "WIMAX",
    "RFKILL",
    "MICMUTE",
};

/*
 * This function gives a key code from a char string.
 */
SDLKey get_key_from_buffer(const char* buffer)
{
  int i;

  for (i = 0; i < MAX_KEYNAMES; i++)
  {
    if (!strncmp(keynames[i], buffer, MAX_NAME_LENGTH))
    {
      return i;
    }
  }

  return 0;
}

/*
 * This function gives a char string from a key code.
 */
const char* get_chars_from_key(SDLKey key)
{
  if(key > 0 && key < MAX_KEYNAMES)
  {
    return keynames[key];
  }

  return keynames[0];
}

/*
 * This table gives a correspondence between a char string and a button code.
 */
char* butnames[MOUSE_BUTTONS_MAX] =
{
    "BUTTON_LEFT",
    "BUTTON_RIGHT",
    "BUTTON_MIDDLE",
    "BUTTON_X3",
    "BUTTON_X4",
    "BUTTON_X5",
    "BUTTON_X6",
    "BUTTON_X7",
    "BUTTON_WHEELUP",
    "BUTTON_WHEELDOWN",
    "BUTTON_X1",
    "BUTTON_X2",
};

const char* get_chars_from_button(int but)
{
  if(but >= 0 && but <sizeof(butnames)/sizeof(char*))
  {
    return butnames[but];
  }
  return NULL;
}

unsigned int get_mouse_event_id_from_buffer(const char* event_id)
{
  unsigned int r_event_id = 0;
  int i;

  if (!strncmp(event_id, MOUSE_AXIS_X, sizeof(MOUSE_AXIS_X)))
  {
    r_event_id = AXIS_X;
  }
  else if (!strncmp(event_id, MOUSE_AXIS_Y, sizeof(MOUSE_AXIS_Y)))
  {
    r_event_id = AXIS_Y;
  }
  else
  {
    for(i=0; i<sizeof(butnames)/sizeof(char*); ++i)
    {
      if(!strcmp(event_id, butnames[i]))
      {
        r_event_id = i;
        break;
      }
    }
  }

  return r_event_id;
}
