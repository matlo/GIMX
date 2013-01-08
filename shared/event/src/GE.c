/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <string.h>
#include <stdlib.h>

#include <iconv.h>

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

static char* sixaxis_names[] =
{
  "Sony PLAYSTATION(R)3 Controller",
  "Sony Navigation Controller"
};

char* joystickName[MAX_DEVICES] = {};
int joystickVirtualIndex[MAX_DEVICES] = {};
int joystickUsed[MAX_DEVICES] = {};
int joystickSixaxis[MAX_DEVICES] = {};
char* mouseName[MAX_DEVICES] = {};
int mouseVirtualIndex[MAX_DEVICES] = {};
char* keyboardName[MAX_DEVICES] = {};
int keyboardVirtualIndex[MAX_DEVICES] = {};

static int grab = 0;

int merge_all_devices = 0;

static const char* _8BIT_to_UTF8(const char* _8bit)
{
  iconv_t cd;
  char* input = (char*)_8bit;
  size_t in = strlen(input) + 1;
  static char output[256];
  char* poutput = output;
  size_t out = sizeof(output);
  cd = iconv_open ("UTF-8", "ISO-8859-1");
  iconv(cd, &input, &in, &poutput, &out);
  iconv_close(cd);
  return output;
}

/*
 * Initializes the library.
 */
int GE_initialize()
{
  int i = 0;
  int j;
  const char* name;

  if (!ev_init())
  {
    return 0;
  }

  i = 0;
  while ((name = ev_joystick_name(i)))
  {
    if (!strncmp(name, BT_SIXAXIS_NAME, sizeof(BT_SIXAXIS_NAME) - 1))
    {
      /*
       * Rename QtSixA devices.
       */
      name = "Sony PLAYSTATION(R)3 Controller";
    }

    joystickName[i] = strdup(_8BIT_to_UTF8(name));

    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(joystickName[i], joystickName[j]))
      {
        joystickVirtualIndex[i] = joystickVirtualIndex[j] + 1;
        break;
      }
    }
    if (j < 0)
    {
      joystickVirtualIndex[i] = 0;
    }
    for (j = 0; j < sizeof(sixaxis_names) / sizeof(sixaxis_names[0]); ++j)
    {
      if (!strcmp(joystickName[i], sixaxis_names[j]))
      {
        joystickSixaxis[i] = 1;
      }
    }
    i++;
  }
  i = 0;
  while ((name = ev_mouse_name(i)))
  {
    mouseName[i] = strdup(_8BIT_to_UTF8(name));

    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(mouseName[i], mouseName[j]))
      {
        mouseVirtualIndex[i] = mouseVirtualIndex[j] + 1;
        break;
      }
    }
    if (j < 0)
    {
      mouseVirtualIndex[i] = 0;
    }
    i++;
  }
  i = 0;
  while ((name = ev_keyboard_name(i)))
  {
    keyboardName[i] = strdup(_8BIT_to_UTF8(name));

    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(keyboardName[i], keyboardName[j]))
      {
        keyboardVirtualIndex[i] = keyboardVirtualIndex[j] + 1;
        break;
      }
    }
    if (j < 0)
    {
      keyboardVirtualIndex[i] = 0;
    }
    i++;
  }

  return 1;
}

/*
 * Release unused stuff.
 * It currently releases unused joysticks, and closes the joystick subsystem if none is used.
 */
void GE_release_unused()
{
  int i;
  for (i = 0; i < MAX_DEVICES && joystickName[i]; ++i)
  {
    if (!joystickUsed[i])
    {
      free(joystickName[i]);
      joystickName[i] = NULL;
      ev_joystick_close(i);
    }
  }
}

/*
 * Grab/Release the mouse.
 */
void GE_grab_toggle()
{
  if (grab)
  {
    ev_grab_input(GE_GRAB_OFF);
    grab = 0;
  }
  else
  {
    ev_grab_input(GE_GRAB_ON);
    grab = 1;
  }
}

/*
 * Grab the mouse.
 */
void GE_grab()
{
  ev_grab_input(GE_GRAB_ON);
  grab = 1;
}

void GE_free_mouse_keyboard_names()
{
  int i;
  for (i = 0; i < MAX_DEVICES && mouseName[i]; ++i)
  {
    free(mouseName[i]);
    mouseName[i] = NULL;
  }
  for (i = 0; i < MAX_DEVICES && keyboardName[i]; ++i)
  {
    free(keyboardName[i]);
    keyboardName[i] = NULL;
  }
}

/*
 * Free allocated data.
 */
void GE_quit()
{
  int i;

  for (i = 0; i < MAX_DEVICES; ++i)
  {
    if (joystickName[i])
    {
      free(joystickName[i]);
      joystickName[i] = NULL;
      ev_joystick_close(i);
    }
  }
  GE_free_mouse_keyboard_names();
  ev_quit();
}

char* GE_MouseName(int id)
{
  if (id >= 0)
  {
    return mouseName[id];
  }
  return NULL;
}

char* GE_KeyboardName(int id)
{
  if (id >= 0)
  {
    return keyboardName[id];
  }
  return NULL;
}

char* GE_JoystickName(int id)
{
  if (id >= 0)
  {
    return joystickName[id];
  }
  return NULL;
}

int GE_JoystickVirtualId(int id)
{
  if (id >= 0)
  {
    return joystickVirtualIndex[id];
  }
  return 0;
}

void GE_SetJoystickUsed(int id)
{
  if (id >= 0)
  {
    joystickUsed[id] = 1;
  }
}

int GE_MouseVirtualId(int id)
{
  if (id >= 0)
  {
    return mouseVirtualIndex[id];
  }
  return 0;
}

int GE_KeyboardVirtualId(int id)
{
  if (id >= 0)
  {
    return keyboardVirtualIndex[id];
  }
  return 0;
}

int GE_IsSixaxis(int id)
{
  if (id >= 0)
  {
    return joystickSixaxis[id];
  }
  return 0;
}

/*
 * Returns the device id of a given event.
 */
int GE_GetDeviceId(GE_Event* e)
{
  /*
   * 'which' should always be at that place
   * There is no need to check the value, since it's stored as an uint8_t, and MAX_DEVICES is 256.
   */
  unsigned int device_id = ((GE_KeyboardEvent*) e)->which;

  switch (e->type)
  {
    case GE_JOYHATMOTION:
    case GE_JOYBUTTONDOWN:
    case GE_JOYBUTTONUP:
    case GE_JOYAXISMOTION:
      break;
    case GE_KEYDOWN:
    case GE_KEYUP:
    case GE_MOUSEBUTTONDOWN:
    case GE_MOUSEBUTTONUP:
    case GE_MOUSEMOTION:
      if (merge_all_devices)
      {
        device_id = 0;
      }
      break;
  }

  return device_id;
}

int GE_PushEvent(GE_Event *event)
{
  return ev_push_event(event);
}

void GE_SetCallback(int(*fp)(GE_Event*))
{
  ev_set_callback(fp);
}

void GE_PumpEvents()
{
  ev_pump_events();
}

int GE_PeepEvents(GE_Event *events, int numevents)
{
  return ev_peep_events(events, numevents);
}
