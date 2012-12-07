/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "conversion.h"
#include "sixaxis.h"
#include <string.h>
#include "events.h"

static const char *axis_name[SA_MAX] = {
    [sa_lstick_x] = "lstick x",
    [sa_lstick_y] = "lstick y",
    [sa_rstick_x] = "rstick x",
    [sa_rstick_y] = "rstick y",
    [sa_acc_x] = "acc x",
    [sa_acc_y] = "acc y",
    [sa_acc_z] = "acc z",
    [sa_gyro] = "gyro",
    [sa_select] = "select",
    [sa_start] = "start",
    [sa_ps] = "PS",
    [sa_up] = "up",
    [sa_right] = "right",
    [sa_down] = "down",
    [sa_left] = "left",
    [sa_triangle] = "triangle",
    [sa_circle] = "circle",
    [sa_cross] = "cross",
    [sa_square] = "square",
    [sa_l1] = "l1",
    [sa_r1] = "r1",
    [sa_l2] = "l2",
    [sa_r2] = "r2",
    [sa_l3] = "l3",
    [sa_r3] = "r3"
};

const char* get_axis_name(int index)
{
  return axis_name[index];
}

int get_button_index_from_name(const char* name)
{
  int i;
  for(i=0; i<SA_MAX; ++i)
  {
    if(!strncmp(axis_name[i], name, MAX_NAME_LENGTH))
    {
      return i;
    }
  }
  return -1;
}

typedef struct {
    char name[MAX_NAME_LENGTH];
    s_axis_index aindex;
} s_axis_name_index;

s_axis_name_index axis_name_index[] =
{
    {.name="rstick x",     {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick y",     {.value=-1,  .index=sa_rstick_y}},
    {.name="lstick x",     {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick y",     {.value=-1,  .index=sa_lstick_y}},
    {.name="rstick left",  {.value=-1,  .index=sa_rstick_x}},
    {.name="rstick right", {.value= 1,  .index=sa_rstick_x}},
    {.name="rstick up",    {.value=-1,  .index=sa_rstick_y}},
    {.name="rstick down",  {.value= 1,  .index=sa_rstick_y}},
    {.name="lstick left",  {.value=-1,  .index=sa_lstick_x}},
    {.name="lstick right", {.value= 1,  .index=sa_lstick_x}},
    {.name="lstick up",    {.value=-1,  .index=sa_lstick_y}},
    {.name="lstick down",  {.value= 1,  .index=sa_lstick_y}},
    {.name="acc x",        {.value=-1,  .index=sa_acc_x}},
    {.name="acc y",        {.value=-1,  .index=sa_acc_y}},
    {.name="acc z",        {.value=-1,  .index=sa_acc_z}},
    {.name="gyro",         {.value=-1,  .index=sa_gyro}},
    {.name="acc x -",      {.value=-1,  .index=sa_acc_x}},
    {.name="acc y -",      {.value=-1,  .index=sa_acc_y}},
    {.name="acc z -",      {.value=-1,  .index=sa_acc_z}},
    {.name="gyro -",       {.value=-1,  .index=sa_gyro}},
    {.name="acc x +",      {.value= 1,  .index=sa_acc_x}},
    {.name="acc y +",      {.value= 1,  .index=sa_acc_y}},
    {.name="acc z +",      {.value= 1,  .index=sa_acc_z}},
    {.name="gyro +",       {.value= 1,  .index=sa_gyro}},
    {.name="up",           {.value= 0,  .index=sa_up}},
    {.name="down",         {.value= 0,  .index=sa_down}},
    {.name="right",        {.value= 0,  .index=sa_right}},
    {.name="left",         {.value= 0,  .index=sa_left}},
    {.name="r1",           {.value= 0,  .index=sa_r1}},
    {.name="r2",           {.value= 0,  .index=sa_r2}},
    {.name="l1",           {.value= 0,  .index=sa_l1}},
    {.name="l2",           {.value= 0,  .index=sa_l2}},
    {.name="circle",       {.value= 0,  .index=sa_circle}},
    {.name="square",       {.value= 0,  .index=sa_square}},
    {.name="cross",        {.value= 0,  .index=sa_cross}},
    {.name="triangle",     {.value= 0,  .index=sa_triangle}},
};

s_axis_index get_axis_index_from_name(const char* name)
{
  int i;
  s_axis_index none = {-1, -1};
  for(i=0; i<sizeof(axis_name_index)/sizeof(s_axis_name_index); ++i)
  {
    if(!strncmp(axis_name_index[i].name, name, MAX_NAME_LENGTH))
    {
      return axis_name_index[i].aindex;
    }
  }
  return none;
}

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
