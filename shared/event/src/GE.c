/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */
#include <GE.h>
#include <math.h>
#include <events.h>
#include <string.h>
#include <stdlib.h>

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

#ifdef WIN32
#include <SDL/SDL.h>
SDL_Joystick* joysticks[MAX_DEVICES] = {};
#endif

char* joystickName[MAX_DEVICES] = {};
int joystickVirtualIndex[MAX_DEVICES] = {};
int joystickNbButton[MAX_DEVICES] = {};
int joystickSixaxis[MAX_DEVICES] = {};
char* mouseName[MAX_DEVICES] = {};
int mouseVirtualIndex[MAX_DEVICES] = {};
char* keyboardName[MAX_DEVICES] = {};
int keyboardVirtualIndex[MAX_DEVICES] = {};

int joystickNbHat[MAX_DEVICES] = {};
unsigned char* joystickHat[MAX_DEVICES] = {};

static int grab = 0;

int merge_all_devices = 0;

/*
 * Initializes the SDL library.
 */
int GE_initialize()
{
  int i = 0;
  int j;
  const char* name;

  if(!ev_init())
  {
    return 0;
  }

#ifndef WIN32
  i = 0;
  while ((name = ev_joystick_name(i)))
  {
   joystickName[i] = strdup(name);

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
   if (!strcmp(joystickName[i], "Sony PLAYSTATION(R)3 Controller")
       || !strcmp(joystickName[i], "Sony Navigation Controller"))
   {
     joystickSixaxis[i] = 1;
   }
   i++;
  }
#else
  while ((joysticks[i] = SDL_JoystickOpen(i)))
  {
    joystickName[i] = strdup(SDL_JoystickName(i));

    if (!strncmp(joystickName[i], BT_SIXAXIS_NAME, sizeof(BT_SIXAXIS_NAME) - 1))
    {
      joystickName[i][sizeof(BT_SIXAXIS_NAME) - 1] = '\0';
    }

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
    joystickNbButton[i] = SDL_JoystickNumButtons(joysticks[i]);
    joystickNbHat[i] = SDL_JoystickNumHats(joysticks[i]);
    if(joystickNbHat[i] > 0)
    {
      joystickHat[i] = calloc(joystickNbHat[i], sizeof(unsigned char));
      if(!joystickHat[i])
      {
        fprintf(stderr, "Unable to allocate %u bytes for joystick hats.\n", joystickNbHat[i]*sizeof(unsigned char));
        return 0;
      }
    }
    if (!strcmp(joystickName[i], "Sony PLAYSTATION(R)3 Controller")
        || !strcmp(joystickName[i], "Sony Navigation Controller"))
    {
      joystickSixaxis[i] = 1;
    }
    i++;
  }
#endif
  i = 0;
  while ((name = ev_mouse_name(i)))
  {
    mouseName[i] = strdup(name);

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
    keyboardName[i] = strdup(name);

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
  int none = 1;
  for(i=0; i<MAX_DEVICES && joystickName[i]; ++i)
  {
    if(!cfg_is_joystick_used(i))
    {
      free(joystickName[i]);
      joystickName[i] = NULL;
#ifdef WIN32
      SDL_JoystickClose(joysticks[i]);
#endif
    }
    else
    {
      none = 0;
    }
  }
  if(none)
  {
#ifdef WIN32
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
#endif
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
    if(joystickName[i])
    {
      free(joystickName[i]);
      free(joystickHat[i]);
#ifdef WIN32
      SDL_JoystickClose(joysticks[i]);
#endif
    }
  }
  GE_free_mouse_keyboard_names();
  ev_quit();
}

inline char* GE_MouseName(int id)
{
  if(id >= 0)
  {
    return mouseName[id];
  }
  return NULL;
}

inline char* GE_KeyboardName(int id)
{
  if(id >= 0)
  {
    return keyboardName[id];
  }
  return NULL;
}

inline char* GE_JoystickName(int id)
{
  if(id >= 0)
  {
    return joystickName[id];
  }
  return NULL;
}

inline int GE_JoystickVirtualId(int id)
{
  if(id >= 0)
  {
    return joystickVirtualIndex[id];
  }
  return 0;
}

inline int GE_MouseVirtualId(int id)
{
  if(id >= 0)
  {
    return mouseVirtualIndex[id];
  }
  return 0;
}

inline int GE_KeyboardVirtualId(int id)
{
  if(id >= 0)
  {
    return keyboardVirtualIndex[id];
  }
  return 0;
}

inline int GE_JoystickHatButton(GE_Event* event, unsigned char hat_dir)
{
  return joystickNbButton[event->jhat.which] + 4*event->jhat.hat + log2(hat_dir);
}

static inline unsigned char get_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    return joystickHat[event->jhat.which][event->jhat.hat];
  }
  return 0;
}

static inline void set_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    joystickHat[event->jhat.which][event->jhat.hat] = event->jhat.value;
  }
}

inline int GE_is_sixaxis(int id)
{
  if(id >= 0)
  {
    return joystickSixaxis[id];
  }
  return 0;
}

/*
 * Returns the device id of a given event.
 */
inline int GE_get_device_id(GE_Event* e)
{
  /*
   * 'which' should always be at that place
   * There is no need to check the value, since it's stored as an uint8_t, and MAX_DEVICES is 256.
   */
  unsigned int device_id = ((GE_KeyboardEvent*)e)->which;

  switch(e->type)
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
      if(merge_all_devices)
      {
        device_id = 0;
      }
    break;
  }

  return device_id;
}

inline int GE_PushEvent(GE_Event *event)
{
  return ev_push_event(event);
}

inline void GE_pump_events()
{
  ev_pump_events();
}

inline int GE_peep_events(GE_Event *events, int numevents)
{
  return ev_peep_events(events, numevents);
}

/*
 * This function translates joystick hat events into joystick button events.
 * The joystick button events are inserted just before the joystick hat events.
 */
inline int GE_preprocess_events(GE_Event *events, int numevents)
{
  GE_Event* event;
  unsigned char hat_dir;

  if(numevents == EVENT_BUFFER_SIZE)
  {
    return EVENT_BUFFER_SIZE;
  }

  for (event = events; event < events + numevents; ++event)
  {
    switch (event->type)
    {
      case GE_JOYHATMOTION:
        /*
         * Check what hat directions changed.
         * The new hat state is compared to the previous one.
         */
        for(hat_dir=1; hat_dir < 16 && event < events + numevents; hat_dir*=2)
        {
          if(event->jhat.value & hat_dir)
          {
            if(!(get_joystick_hat(event) & hat_dir))
            {
              /*
               * The hat direction is pressed.
               */
              memmove(event+1, event, (events + numevents - event)*sizeof(GE_Event));
              event->type = GE_JOYBUTTONDOWN;
              event->jbutton.which = (event+1)->jhat.which;
              event->jbutton.button = GE_JoystickHatButton((event+1), hat_dir);
              event++;
              numevents++;
            }
          }
          else
          {
            if(get_joystick_hat(event) & hat_dir)
            {
              /*
               * The hat direction is released.
               */
              memmove(event+1, event, (events + numevents - event)*sizeof(GE_Event));
              event->type = GE_JOYBUTTONUP;
              event->jbutton.which = (event+1)->jhat.which;
              event->jbutton.button = GE_JoystickHatButton((event+1), hat_dir);
              event++;
              numevents++;
            }
          }
        }
        /*
         * Save the new hat state.
         */
        set_joystick_hat(event);
        /*
         * Remove the joystick hat event.
         */
        memmove(event, event+1, (events + numevents - event - 1)*sizeof(GE_Event));
        event--;
        numevents--;
        break;
      default:
        break;
    }
  }
  return numevents;
}
