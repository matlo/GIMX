/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "xinput.h"
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>

static Display* dpy = NULL;
static Window win;
static int xi_opcode;

#define DEVTYPE_KEYBOARD 0x01
#define DEVTYPE_MOUSE    0x02

static struct
{
  unsigned char type;
  int fd;
  int id;
  char* name;
} devices[GE_MAX_DEVICES];

static int nb_devices;

static int device_index[GE_MAX_DEVICES];

static int (*event_callback)(GE_Event*) = NULL;

static struct
{
  int x;
  int y;
} mouse_coordinates;

void xinput_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

inline uint8_t get_button(int detail)
{
  switch(detail)
  {
    case 1:
      return GE_BTN_LEFT;
    case 2:
      return GE_BTN_MIDDLE;
    case 3:
      return GE_BTN_RIGHT;
    case 4:
      return GE_BTN_WHEELUP;
    case 5:
      return GE_BTN_WHEELDOWN;
    case 6:
      return GE_BTN_WHEELRIGHT;
    case 7:
      return GE_BTN_WHEELLEFT;
    case 8:
      return GE_BTN_BACK;
    case 9:
      return GE_BTN_FORWARD;
    default:
      return 0xff;
  }
}

static void xinput_process_event(XIRawEvent* revent)
{
  GE_Event evt = {};
  int i;

  //ignore events from master device
  if (revent->deviceid != revent->sourceid || revent->sourceid >= GE_MAX_DEVICES)
  {
    return;
  }

  int index = device_index[revent->sourceid];

  switch (revent->evtype)
  {
    case XI_RawMotion:
      if(index >= 0)
      {
        evt.type = GE_MOUSEMOTION;
        evt.motion.which = devices[index].id;
        i = 0;
        evt.motion.xrel = XIMaskIsSet(revent->valuators.mask, 0) ? revent->raw_values[i++] : 0;
        evt.motion.yrel = XIMaskIsSet(revent->valuators.mask, 1) ? revent->raw_values[i++] : 0;
      }
      break;
    case XI_RawButtonPress:
      if(index >= 0)
      {
        evt.type = GE_MOUSEBUTTONDOWN;
        evt.button.which = devices[index].id;
        evt.button.button = get_button(revent->detail);
      }
      break;
    case XI_RawButtonRelease:
      if(index >= 0)
      {
        evt.type = GE_MOUSEBUTTONUP;
        evt.button.which = devices[index].id;
        evt.button.button = get_button(revent->detail);
      }
      break;
    case XI_RawKeyPress:
    {
      if(index >= 0)
      {
        evt.type = GE_KEYDOWN;
        evt.button.which = devices[index].id;
        evt.button.button = revent->detail - 8;
      }
      break;
    }
    case XI_RawKeyRelease:
    {
      if(index >= 0)
      {
        evt.type = GE_KEYUP;
        evt.button.which = devices[index].id;
        evt.button.button = revent->detail - 8;
      }
      break;
    }
    default:
      break;
  }

  /*
   * Process evt.
   */
  if(evt.type != GE_NOEVENT)
  {
    event_callback(&evt);
  }
}

static int xinput_process_events(int index)
{
  XEvent ev;
  XGenericEventCookie *cookie = &ev.xcookie;

  while (XPending(dpy))
  {
    XFlush(dpy);
    XPending(dpy);

    XNextEvent(dpy, &ev);
    if (XGetEventData(dpy, cookie))
    {
      XIRawEvent* revent = cookie->data;

      if(cookie->type == GenericEvent && cookie->extension == xi_opcode)
      {
        xinput_process_event(revent);
      }

      XFreeEventData(dpy, cookie);
    }
  }

  return 0;
}

static int xinput_close(int id)
{
  xinput_quit();

  return 1;
}

static Window create_win(Display *dpy)
{
  XIEventMask mask;

  Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0, 0, 0);

  mask.deviceid = XIAllDevices;
  mask.mask_len = XIMaskLen(XI_RawMotion);
  mask.mask = calloc(mask.mask_len, sizeof(char));

  XISetMask(mask.mask, XI_RawButtonPress);
  XISetMask(mask.mask, XI_RawButtonRelease);
  XISetMask(mask.mask, XI_RawKeyPress);
  XISetMask(mask.mask, XI_RawKeyRelease);
  XISetMask(mask.mask, XI_RawMotion);

  XISelectEvents(dpy, DefaultRootWindow(dpy), &mask, 1);

  free(mask.mask);
  XMapWindow(dpy, win);
  XSync(dpy, True);

  return win;
}

int xinput_init()
{
  int ret = 0;
  int event, error;
  int m_num = 0;
  int k_num = 0;

  memset(devices, 0x00, sizeof(devices));
  nb_devices = 0;

  unsigned int i;
  for(i=0; i<sizeof(device_index)/sizeof(*device_index); ++i)
  {
    device_index[i] = -1;
  }

  dpy = XOpenDisplay(NULL);

  if (!dpy)
  {
    fprintf(stderr, "Failed to open display.\n");
    return -1;
  }

  if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error))
  {
    fprintf(stderr, "X Input extension not available.\n");
    return -1;
  }

  win = create_win(dpy);

  int nxdevices;
  XIDeviceInfo *xdevices, *xdevice;

  xdevices = XIQueryDevice(dpy, XIAllDevices, &nxdevices);

  for (i = 0; nxdevices > 0 && i < (unsigned int) nxdevices; i++)
  {
      xdevice = &xdevices[i];

      if(xdevice->deviceid >= GE_MAX_DEVICES)
      {
        continue;
      }

      device_index[xdevice->deviceid] = nb_devices;

      switch(xdevice->use) {
         case XISlaveKeyboard:
           devices[nb_devices].type = DEVTYPE_KEYBOARD;
           devices[nb_devices].id = k_num;
           ++k_num;
           break;
         case XISlavePointer:
           devices[nb_devices].type = DEVTYPE_MOUSE;
           devices[nb_devices].id = m_num;
           ++m_num;
           break;
      }

      if(devices[nb_devices].type)
      {
        devices[nb_devices].name = strdup(xdevice->name);
        ++nb_devices;
      }
  }

  XIFreeDeviceInfo(xdevices);

  ev_register_source(ConnectionNumber(dpy), i, &xinput_process_events, NULL, &xinput_close);

  XEvent xevent;

  /* get info about current pointer position */
  XQueryPointer(dpy, DefaultRootWindow(dpy),
  &xevent.xbutton.root, &xevent.xbutton.window,
  &xevent.xbutton.x_root, &xevent.xbutton.y_root,
  &xevent.xbutton.x, &xevent.xbutton.y,
  &xevent.xbutton.state);

  mouse_coordinates.x = xevent.xbutton.x;
  mouse_coordinates.y = xevent.xbutton.y;

  return ret;
}

void xinput_quit()
{
  if(dpy)
  {
    ev_remove_source(ConnectionNumber(dpy));

    XDestroyWindow(dpy, win);

    XWarpPointer(dpy, None, DefaultRootWindow(dpy), 0, 0, 0, 0, mouse_coordinates.x, mouse_coordinates.y);

    XCloseDisplay(dpy);
    dpy = NULL;

    int i;
    for(i=0; i<nb_devices; ++i)
    {
      free(devices[i].name);
      devices[i].name = NULL;
    }
  }
}

/*
 * Grab the pointer into the window.
 * This function retries until success or 500ms have elapsed.
 */
void xinput_grab(int mode)
{
  int i = 0;
  while(XGrabPointer(dpy, win, True, 0, GrabModeAsync, GrabModeAsync, win, None, CurrentTime) != GrabSuccess && i < 50)
  {
    usleep(10000);
    ++i;
  }
}

static char* get_name(unsigned char devtype, int index)
{
  int i;
  for(i=0; i<nb_devices; ++i)
  {
    if(devices[i].type == devtype && devices[i].id == index)
    {
      return devices[i].name;
    }
  }
  return NULL;
}

const char* xinput_get_mouse_name(int index)
{
  return get_name(DEVTYPE_MOUSE, index);
}

const char* xinput_get_keyboard_name(int index)
{
  return get_name(DEVTYPE_KEYBOARD, index);
}
