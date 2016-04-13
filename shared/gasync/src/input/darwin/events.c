/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "../events.h"

#include <ginput.h>
#include <gpoll.h>
#include "../sdl/sdlinput.h"

int ev_init(unsigned char mkb_src, int(*callback)(GE_Event*))
{
  if (callback == NULL) {
    fprintf(stderr, "callback cannot be NULL\n");
    return -1;
  }

  if (mkb_src == GE_MKB_SOURCE_PHYSICAL)
  {
    fprintf(stderr, "Physical events are not available on this platform.\n");
    return -1;
  }

  if(sdlinput_init(mkb_src, callback) < 0)
  {
    return -1;
  }

  return 0;
}

void ev_quit(void)
{
  sdlinput_quit();
}

const char* ev_joystick_name(int id)
{
  return sdlinput_joystick_name(id);
}

int ev_joystick_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short))
{
  return sdlinput_joystick_register(name, rumble_cb);
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
  if(id == 0)
  {
    return "Window Events";
  }
  return NULL;
}

const char* ev_keyboard_name(int id)
{
  if(id == 0)
  {
    return "Window Events";
  }
  return NULL;
}

void ev_grab_input(int mode)
{
  int i;

  if(mode == GE_GRAB_ON)
  {
    SDL_SetRelativeMouseMode(SDL_TRUE);

    //TODO MLA
  }
  else
  {
    //TODO MLA

    SDL_SetRelativeMouseMode(SDL_FALSE);
  }
}

void ev_sync_process()
{
  return sdlinput_sync_process();
}

int ev_joystick_has_ff_rumble(int joystick)
{
  return sdlinput_joystick_has_ff_rumble(joystick);
}

int ev_joystick_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong)
{
  return sdlinput_joystick_set_ff_rumble(joystick, weak, strong);
}

int ev_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product)
{
  return sdlinput_joystick_get_usb_ids(joystick, vendor, product);
}
