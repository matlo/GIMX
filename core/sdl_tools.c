/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "sdl_tools.h"
#include "config.h"
#include "emuclient.h"
#include <math.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "Sixaxis Control"
#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

char* joystickName[MAX_DEVICES] = {};
SDL_Joystick* joysticks[MAX_DEVICES] = {};
int joystickVirtualIndex[MAX_DEVICES] = {};
int joystickNbButton[MAX_DEVICES] = {};
int joystickSixaxis[MAX_DEVICES] = {};
char* mouseName[MAX_DEVICES] = {};
int mouseVirtualIndex[MAX_DEVICES] = {};
char* keyboardName[MAX_DEVICES] = {};
int keyboardVirtualIndex[MAX_DEVICES] = {};

int joystickNbHat[MAX_DEVICES] = {};
unsigned char* joystickHat[MAX_DEVICES] = {};

static SDL_Surface *screen = NULL;
static int grab = 0;

int merge_all_devices = 0;

/*
 * Initializes the SDL library.
 */
int sdl_initialize()
{
  int i = 0;
  int j;
  const char* name;

  /* Init SDL */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    return 0;
  }

  SDL_WM_SetCaption(TITLE, TITLE);

  /* Init video */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
      SDL_HWSURFACE | SDL_ANYFORMAT | SDL_NOFRAME);
  if (screen == NULL)
  {
    fprintf(stderr, "Unable to create video surface: %s\n", SDL_GetError());
    return 0;
  }

  SDL_ShowCursor(SDL_DISABLE);

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
        fprintf(stderr, "Unable to allocate %d bytes for joystick hats.\n", joystickNbHat[i]*sizeof(unsigned char));
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

  i = 0;
  while ((name = SDL_GetMouseName(i)))
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
  while ((name = SDL_GetKeyboardName(i)))
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
void sdl_release_unused()
{
  int i;
  int none = 1;
  for(i=0; i<MAX_DEVICES && joystickName[i]; ++i)
  {
    if(!cfg_is_joystick_used(i))
    {
      gprintf(_("close unused joystick: %s\n"), joystickName[i]);
      free(joystickName[i]);
      joystickName[i] = NULL;
      SDL_JoystickClose(joysticks[i]);
    }
    else
    {
      none = 0;
    }
  }
  if(none)
  {
    gprintf(_("close joystick subsystem\n"));
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  }
}

/*
 * Grab/Release the mouse.
 */
void sdl_grab_toggle()
{
  if (grab)
  {
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    grab = 0;
  }
  else
  {
    SDL_WM_GrabInput(SDL_GRAB_ON);
    grab = 1;
  }
}

/*
 * Grab the mouse.
 */
void sdl_grab()
{
  SDL_WM_GrabInput(SDL_GRAB_ON);
  grab = 1;
}

void sdl_free_mk()
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
void sdl_quit()
{
  int i;

  for (i = 0; i < MAX_DEVICES; ++i)
  {
    if(joystickName[i])
    {
      free(joystickName[i]);
      free(joystickHat[i]);
      SDL_JoystickClose(joysticks[i]);
    }
  }
  for (i = 0; i < MAX_DEVICES && mouseName[i]; ++i)
  {
    free(mouseName[i]);
  }
  for (i = 0; i < MAX_DEVICES && keyboardName[i]; ++i)
  {
    free(keyboardName[i]);
  }
#ifndef WIN32
  SDL_FreeSurface(screen);
  SDL_Quit();
#endif
}

inline char* sdl_get_mouse_name(int id)
{
  if(id >= 0)
  {
    return mouseName[id];
  }
  return NULL;
}

inline char* sdl_get_keyboard_name(int id)
{
  if(id >= 0)
  {
    return keyboardName[id];
  }
  return NULL;
}

inline char* sdl_get_joystick_name(int id)
{
  if(id >= 0)
  {
    return joystickName[id];
  }
  return NULL;
}

inline int sdl_get_joystick_virtual_id(int id)
{
  if(id >= 0)
  {
    return joystickVirtualIndex[id];
  }
  return 0;
}

inline int sdl_get_mouse_virtual_id(int id)
{
  if(id >= 0)
  {
    return mouseVirtualIndex[id];
  }
  return 0;
}

inline int sdl_get_keyboard_virtual_id(int id)
{
  if(id >= 0)
  {
    return keyboardVirtualIndex[id];
  }
  return 0;
}

inline int sdl_get_joystick_buttons(int id)
{
  if(id >= 0)
  {
    return joystickNbButton[id];
  }
  return 0;
}

inline int sdl_get_joystick_hat_button(SDL_Event* event, unsigned char hat_dir)
{
  return sdl_get_joystick_buttons(event->jhat.which) + 4*event->jhat.hat + log2(hat_dir);
}

inline unsigned char sdl_get_joystick_hat(SDL_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    return joystickHat[event->jhat.which][event->jhat.hat];
  }
  return 0;
}

inline void sdl_set_joystick_hat(SDL_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    joystickHat[event->jhat.which][event->jhat.hat] = event->jhat.value;
  }
}

inline int sdl_is_sixaxis(int id)
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
inline int sdl_get_device_id(SDL_Event* e)
{
  /*
   * 'which' should always be at that place
   * There is no need to check the value, since it's stored as an uint8_t, and MAX_DEVICES is 256.
   */
  unsigned int device_id = ((SDL_KeyboardEvent*)e)->which;

  switch(e->type)
  {
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    case SDL_JOYAXISMOTION:
    break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
      if(merge_all_devices)
      {
        device_id = 0;
      }
    break;
  }

  return device_id;
}

inline void sdl_pump_events()
{
  SDL_PumpEvents();
}

inline int sdl_peep_events(SDL_Event *events, int numevents, SDL_eventaction action, Uint32 mask)
{
  return SDL_PeepEvents(events, numevents, action, mask);
}

/*
 * This function translates joystick hat events into joystick button events.
 * The joystick button events are inserted just before the joystick hat events.
 */
inline int sdl_preprocess_events(SDL_Event *events, int numevents)
{
  SDL_Event* event;
  unsigned char hat_dir;

  if(numevents == EVENT_BUFFER_SIZE)
  {
    return EVENT_BUFFER_SIZE;
  }

  for (event = events; event < events + numevents; ++event)
  {
    switch (event->type)
    {
      case SDL_JOYHATMOTION:
        /*
         * Check what hat directions changed.
         * The new hat state is compared to the previous one.
         */
        for(hat_dir=1; hat_dir < 16 && event < events + numevents; hat_dir*=2)
        {
          if(event->jhat.value & hat_dir)
          {
            if(!(sdl_get_joystick_hat(event) & hat_dir))
            {
              /*
               * The hat direction is pressed.
               */
              memmove(event+1, event, (events + numevents - event)*sizeof(SDL_Event));
              event->type = SDL_JOYBUTTONDOWN;
              event->jbutton.which = (event+1)->jhat.which;
              event->jbutton.button = sdl_get_joystick_hat_button((event+1), hat_dir);
              event++;
              numevents++;
            }
          }
          else
          {
            if(sdl_get_joystick_hat(event) & hat_dir)
            {
              /*
               * The hat direction is released.
               */
              memmove(event+1, event, (events + numevents - event)*sizeof(SDL_Event));
              event->type = SDL_JOYBUTTONUP;
              event->jbutton.which = (event+1)->jhat.which;
              event->jbutton.button = sdl_get_joystick_hat_button((event+1), hat_dir);
              event++;
              numevents++;
            }
          }
        }
        /*
         * Save the new hat state.
         */
        sdl_set_joystick_hat(event);
        /*
         * Remove the joystick hat event.
         */
        memmove(event, event+1, (events + numevents - event - 1)*sizeof(SDL_Event));
        event--;
        numevents--;
        break;
      default:
        break;
    }
  }
  return numevents;
}
