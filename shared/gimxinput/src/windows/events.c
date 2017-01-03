/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "../events.h"

#include <windows.h>

#include <ginput.h>
#include <gimxpoll/include/gpoll.h>
#include "rawinput.h"
#include "../sdl/sdlinput.h"

static unsigned char mkb_source;

int ev_init(const GPOLL_INTERFACE * poll_interface __attribute__((unused)), unsigned char mkb_src, int(*callback)(GE_Event*))
{
  if (callback == NULL) {
    fprintf(stderr, "callback cannot be NULL\n");
    return -1;
  }

  if(sdlinput_init(mkb_src, callback) < 0)
  {
    return -1;
  }

  mkb_source = mkb_src;

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    if(rawinput_init(callback) < 0)
    {
      return -1;
    }
  }

  gpoll_set_rawinput_callback(rawinput_poll);

  return 0;
}

void ev_quit(void)
{
  ev_grab_input(GE_GRAB_OFF);

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    rawinput_quit();
  }

  sdlinput_quit();
}

const char* ev_joystick_name(int id)
{
  return sdlinput_joystick_name(id);
}

int ev_joystick_register(const char* name, unsigned int effects, int (*haptic_cb)(const GE_Event * event))
{
  return sdlinput_joystick_register(name, effects, haptic_cb);
}

/*
 * Close a joystick, and close the joystick subsystem if none is used anymore.
 */
void ev_joystick_close(int id)
{
  return sdlinput_joystick_close(id);
}

const char* ev_mouse_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return rawinput_mouse_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

const char* ev_keyboard_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return rawinput_keyboard_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

void ev_grab_input(int mode)
{
  int i;

  if(mode == GE_GRAB_ON)
  {
    HWND hwnd = NULL;

    if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
    {
      hwnd = FindWindow(RAWINPUT_CLASS_NAME, RAWINPUT_WINDOW_NAME);
    }
    else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
    {
      SDL_SetRelativeMouseMode(SDL_TRUE);

      hwnd = FindWindow(NULL, SDLINPUT_WINDOW_NAME);
    }

    if(hwnd)
    {
      // Reading from stdin before initializing ginput prevents the capture window from reaching the foreground...
      // This is a hack to work-around this issue.
      ShowWindow(hwnd, SW_MINIMIZE);
      ShowWindow(hwnd, SW_RESTORE);

      //clip the mouse cursor into the window
      RECT _clip;

      if(GetWindowRect(hwnd, &_clip))
      {
        ClipCursor(&_clip);
      }

      i = 10;
      while(i > 0 && ShowCursor(FALSE) >= 0) { i--; }
    }
  }
  else
  {
    ClipCursor(NULL);

    i = 10;
    while(i > 0 && ShowCursor(TRUE) < 0) { i--; }

    if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
    {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }
  }
}

void ev_sync_process()
{
  return sdlinput_sync_process();
}

int ev_joystick_get_haptic(int joystick)
{
  return sdlinput_joystick_get_haptic(joystick);
}

int ev_joystick_set_haptic(const GE_Event * event)
{
  return sdlinput_joystick_set_haptic(event);
}

int ev_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product)
{
  return sdlinput_joystick_get_usb_ids(joystick, vendor, product);
}
