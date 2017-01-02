/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ginput.h>

/* WinUser.h won't include rawinput stuff without this... */
#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>

static unsigned char noflag_keys[0xFF] =
{
  [0x57] = KEY_F11,
  [0x58] = KEY_F12,
  [0x73] = KEY_RO,
  [0x78] = KEY_KATAKANA,
  [0x77] = KEY_HIRAGANA,
  [0x79] = KEY_HENKAN,
  [0x70] = KEY_KATAKANAHIRAGANA,
  [0x7B] = KEY_MUHENKAN,
  [0x5C] = KEY_KPJPCOMMA,
  [0x59] = KEY_KPEQUAL,
  [0x7E] = KEY_KPCOMMA,
  [0xF2] = KEY_HANGEUL,
  [0xF1] = KEY_HANJA,
  [0x7D] = KEY_YEN,
  [0x64] = KEY_F13,
  [0x65] = KEY_F14,
  [0x66] = KEY_F15,
  [0x67] = KEY_F16,
  [0x68] = KEY_F17,
  [0x69] = KEY_F18,
  [0x6A] = KEY_F19,
  [0x6B] = KEY_F20,
  [0x6C] = KEY_F21,
  [0x6D] = KEY_F22,
  [0x6E] = KEY_F23,
  [0x76] = KEY_F24,
  [0x56] = KEY_102ND,
  [0x54] = KEY_PRINT, // SYSRQ
};

static unsigned char e0flag_keys[0xFF] =
{
  [0x1C] = KEY_KPENTER,
  [0x1D] = KEY_RIGHTCTRL,
  [0x35] = KEY_KPSLASH,
  [0x38] = KEY_RIGHTALT,
  [0x47] = KEY_HOME,
  [0x48] = KEY_UP,
  [0x49] = KEY_PAGEUP,
  [0x4B] = KEY_LEFT,
  [0x4D] = KEY_RIGHT,
  [0x4F] = KEY_END,
  [0x50] = KEY_DOWN,
  [0x51] = KEY_PAGEDOWN,
  [0x52] = KEY_INSERT,
  [0x53] = KEY_DELETE,
  [0x20] = KEY_MUTE,
  [0x2E] = KEY_VOLUMEDOWN,
  [0x30] = KEY_VOLUMEUP,
  [0x5E] = KEY_POWER,
  [0x5B] = KEY_LEFTMETA,
  [0x5C] = KEY_RIGHTMETA,
  [0x21] = KEY_CALC,
  [0x5F] = KEY_SLEEP,
  [0x63] = KEY_WAKEUP,
  [0x6C] = KEY_MAIL,
  [0x66] = KEY_BOOKMARKS,
  [0x6B] = KEY_COMPUTER,
  [0x6A] = KEY_BACK,
  [0x69] = KEY_FORWARD,
  [0x19] = KEY_NEXTSONG,
  [0x22] = KEY_PLAYPAUSE,
  [0x10] = KEY_PREVIOUSSONG,
  [0x24] = KEY_STOP,
  [0x32] = KEY_HOMEPAGE,
  [0x67] = KEY_REFRESH,
  [0x37] = KEY_PRINT, // PRINTSCREEN
  [0x65] = KEY_SEARCH,
  [0x6D] = KEY_MEDIA,
  [0x5D] = KEY_COMPOSE,
};

unsigned char get_keycode(unsigned short flags, unsigned short scancode)
{
  if(scancode > 0xFF)
  {
    return 0;
  }
  if(flags & RI_KEY_E0)
  {
	  return e0flag_keys[scancode];
  }
  else if(flags & RI_KEY_E1)
  {
    if(scancode == 0x1D)
    {
      return KEY_PAUSE;
    }
  }
  else if(scancode > KEY_KPDOT)
  {
    return noflag_keys[scancode];
  }
  else
  {
    return scancode;
  }
  return 0;
}
