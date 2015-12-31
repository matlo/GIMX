/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include "events.h"
#include <sys/signalfd.h>
#include <math.h>
#include <ginput.h>
#include <string.h>
#include "mkb.h"
#include "js.h"
#include "xinput.h"
#include <queue.h>

static unsigned char mkb_source;

int ev_init(unsigned char mkb_src, int(*callback)(GE_Event*))
{
  int ret;

  mkb_source = mkb_src;

  if (callback == NULL) {
    fprintf(stderr, "callback cannot be NULL\n");
    return 0;
  }

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    ret = mkb_init();

    if(ret < 0)
    {
      fprintf(stderr, "mkb_init failed.\n");
      return 0;
    }

    mkb_set_callback(callback);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    ret = xinput_init();

    if(ret < 0)
    {
      fprintf(stderr, "xinput_init failed.\n");
      return 0;
    }

    xinput_set_callback(callback);
  }

  ret = js_init();

  if(ret < 0)
  {
    fprintf(stderr, "jsdev_init failed.\n");
    return 0;
  }

  js_set_callback(callback);

  queue_init();

  return 1;
}

int ev_joystick_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short))
{
  return js_register(name, rumble_cb);
}

void ev_joystick_close(int id)
{
  js_close(id);
}

void ev_grab_input(int mode)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    mkb_grab(mode);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    xinput_grab(mode);
  }
}

void ev_quit(void)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    mkb_quit();
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    xinput_quit();
  }
  js_quit();
}

const char* ev_joystick_name(int index)
{
  return js_get_name(index);
}

const char* ev_mouse_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return mkb_get_m_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    return xinput_get_mouse_name(id);
  }
  return NULL;
}

const char* ev_keyboard_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return mkb_get_k_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    return xinput_get_keyboard_name(id);
  }
  return NULL;
}

int ev_joystick_has_ff_rumble(int joystick)
{
  return js_has_ff_rumble(joystick);
}

int ev_joystick_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong)
{
  return js_set_ff_rumble(joystick, weak, strong);
}

int ev_joystick_get_uhid_id(int joystick)
{
  return js_get_uhid_id(joystick);
}

void ev_sync_process()
{
  // All inputs are asynchronous on Linux!
}
