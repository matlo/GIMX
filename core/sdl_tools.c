/*
 * sdl_tools.c
 *
 *  Created on: 28 mai 2011
 *      Author: Matlo
 *
 *  License: GPLv3
 */
#include "sdl_tools.h"
#include "config.h"
#include "emuclient.h"

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
      gprintf("close unused joystick: %s\n", joystickName[i]);
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
    gprintf("close joystick subsystem\n");
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
