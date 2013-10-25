/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <SDL.h>
#include "winmm/manymouse.h"
#include <GE.h>
#include <events.h>
#include <math.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "Sixaxis Control"

static SDL_Surface *screen = NULL;

static SDL_Joystick* joysticks[GE_MAX_DEVICES] = {};
static int j_num;
static int j_max;

static int joystickNbButton[GE_MAX_DEVICES] = {};
static int joystickNbHat[GE_MAX_DEVICES] = {};
static unsigned char* joystickHat[GE_MAX_DEVICES] = {};

int ev_init()
{
  int i;
  
  /* Init SDL */
#ifndef SDL2
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
#else
  if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
#endif
  {
   fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
   return 0;
  }

#ifndef SDL2
  /* Init video */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
     SDL_HWSURFACE | SDL_ANYFORMAT | SDL_NOFRAME);
  if (screen == NULL)
  {
   fprintf(stderr, "Unable to create video surface: %s\n", SDL_GetError());
   return 0;
  }
#endif
  
  j_max = 0;
  i = 0;
  while((joysticks[i] = SDL_JoystickOpen(i)))
  {
    j_max++;
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
    i++;
  }
  
  j_num = j_max;

  if(ManyMouse_Init() < 0)
  {
    return 0;
  }

  return 1;
}

void ev_quit(void)
{
  int i;
  for(i=0; i<j_max; ++i)
  {
    ev_joystick_close(i);
  }
  SDL_Quit();
#ifndef SDL2
  SDL_WarpMouse(100, 100);
#else
  SDL_WarpMouseInWindow(NULL, 100, 100);//TODO MLA: test this
#endif
  ManyMouse_Quit();
}

const char* ev_joystick_name(int id)
{
#ifndef SDL2
  return SDL_JoystickName(id);
#else
  return SDL_JoystickName(joysticks[id]);
#endif
}

/*
 * Close a joystick, and close the joystick subsystem if none is used anymore.
 */
void ev_joystick_close(int id)
{
  if(joysticks[id])
  {
    SDL_JoystickClose(joysticks[id]);
    joysticks[id] = NULL;
    free(joystickHat[id]);
    joystickHat[id] = NULL;
    j_num--;
    if(j_num == 0)
    {
      SDL_QuitSubSystem(SDL_INIT_JOYSTICK);    
    }
  }
}

static int m_num = 0;

const char* ev_mouse_name(int id)
{
  const char* name = ManyMouse_MouseName(id);
  if(name)
  {
    if(id+1 > m_num)
    {
      m_num = id+1;
    }
  }
  return name;
}

const char* ev_keyboard_name(int id)
{
  return ManyMouse_KeyboardName(id);
}

void ev_grab_input(int mode)
{
#ifndef SDL2
  SDL_WM_GrabInput(mode);

  SDL_ShowCursor(SDL_DISABLE);
#else
  SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
}

void ev_set_callback(int (*fp)(GE_Event*))
{
}

static short m_x[GE_MAX_DEVICES] = {};
static short m_y[GE_MAX_DEVICES] = {};

void ev_pump_events()
{
  SDL_PumpEvents();

  ManyMouseEvent event;
  Uint8 button = 0;

  while (ManyMouse_PollEvent(&event))
  {
    if (event.type == MANYMOUSE_EVENT_RELMOTION)
    {
      if(event.item == 0)
      {
          m_x[event.device]+=event.value;
      }
      else
      {
          m_y[event.device]+=event.value;
      }
    }
    else if (event.type == MANYMOUSE_EVENT_BUTTON)
    {
        switch(event.item)
        {
          case 0:
            button = SDL_BUTTON_LEFT;
            break;
          case 1:
            button = SDL_BUTTON_RIGHT;
            break;
          case 2:
            button = SDL_BUTTON_MIDDLE;
            break;
          case 3:
            button = SDL_BUTTON_X1;
            break;
          case 4:
            button = SDL_BUTTON_X2;
            break;
        }
        SDL_Event se = {};
        se.button.type = event.value ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
        se.button.which = event.device;
        se.button.button = button;
        SDL_PushEvent(&se);
    }
    else if (event.type == MANYMOUSE_EVENT_SCROLL)
    {
      if(event.item == 0)
      {
        SDL_Event se = {};
#ifndef SDL2
        se.button.type = SDL_MOUSEBUTTONDOWN;
        se.button.which = event.device;
        se.button.button = (event.value > 0) ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        SDL_PushEvent(&se);
        se.button.type = SDL_MOUSEBUTTONUP;
        SDL_PushEvent(&se);
#else
        se.wheel.type = SDL_MOUSEWHEEL;
        se.wheel.which = event.device;
        se.wheel.x = event.value;
        SDL_PushEvent(&se);
#endif
      }
      else
      {
        SDL_Event se = {};
#ifndef SDL2
        se.button.type = SDL_MOUSEBUTTONDOWN;
        se.button.which = event.device;
        se.button.button = (event.value < 0) ? SDL_BUTTON_X3 : SDL_BUTTON_X4;
        SDL_PushEvent(&se);
        se.button.type = SDL_MOUSEBUTTONUP;
        SDL_PushEvent(&se);
#else
        se.wheel.type = SDL_MOUSEWHEEL;
        se.wheel.which = event.device;
        se.wheel.y = event.value;
        SDL_PushEvent(&se);
#endif
      }
    }
    else if(event.type == MANYMOUSE_EVENT_KEY)
    {
      SDL_Event se = {};
      se.key.type = event.value ? SDL_KEYDOWN : SDL_KEYUP;
#ifndef SDL2
      se.key.which = event.device;
#else
      se.key.padding2 = event.device;
#endif
      se.key.keysym.sym = event.scancode;
      SDL_PushEvent(&se);
    }
  }

  int i;
  for(i=0; i<m_num; ++i)
  {
    if(m_x[i] || m_y[i])
    {
      SDL_Event se = {};
      se.motion.type = SDL_MOUSEMOTION;
      se.motion.which = i;
      se.motion.xrel = m_x[i];
      se.motion.yrel = m_y[i];
      SDL_PushEvent(&se);
      m_x[i] = 0;
      m_y[i] = 0;
    }
  }
}

static inline void convert_g2s(SDL_Event* se, GE_Event* ge)
{
  switch (ge->type)
  {
  case GE_KEYDOWN:
    se->type = SDL_KEYDOWN;
#ifndef SDL2
    se->key.which = ge->key.which;
#else
    se->key.padding2 = ge->key.which;
#endif
    se->key.keysym.sym = ge->key.keysym;
    break;
  case GE_KEYUP:
    se->type = SDL_KEYUP;
#ifndef SDL2
    se->key.which = ge->key.which;
#else
    se->key.padding2 = ge->key.which;
#endif
    se->key.keysym.sym = ge->key.keysym;
    break;
  case GE_MOUSEBUTTONDOWN:
    se->type = SDL_MOUSEBUTTONDOWN;
    se->button.which = ge->button.which;
    se->button.button = ge->button.button;
    break;
  case GE_MOUSEBUTTONUP:
    se->type = SDL_MOUSEBUTTONUP;
    se->button.which = ge->button.which;
    se->button.button = ge->button.button;
    break;
  case GE_JOYBUTTONDOWN:
    se->type = SDL_JOYBUTTONDOWN;
    se->jbutton.which = ge->jbutton.which;
    se->jbutton.button = ge->jbutton.button;
    break;
  case GE_JOYBUTTONUP:
    se->type = SDL_JOYBUTTONUP;
    se->jbutton.which = ge->jbutton.which;
    se->jbutton.button = ge->jbutton.button;
    break;
  case GE_MOUSEMOTION:
    se->type = SDL_MOUSEMOTION;
    se->motion.which = ge->motion.which;
    se->motion.xrel = ge->motion.xrel;
    se->motion.yrel = ge->motion.yrel;
    break;
  case GE_JOYAXISMOTION:
    se->type = SDL_JOYAXISMOTION;
    se->jaxis.which = ge->jaxis.which;
    se->jaxis.axis = ge->jaxis.axis;
    se->jaxis.value = ge->jaxis.value;
    break;
  case GE_JOYHATMOTION:
    se->type = SDL_JOYHATMOTION;
    se->jhat.which = ge->jhat.which;
    se->jhat.hat = ge->jhat.hat;
    se->jhat.value = ge->jhat.value;
    break;
  }
}

int ev_push_event(GE_Event* ge)
{
  SDL_Event se;

  convert_g2s(&se, ge);

  return SDL_PushEvent(&se);
}

static inline void convert_s2g(SDL_Event* se, GE_Event* ge)
{
  switch (se->type)
  {
  case SDL_KEYDOWN:
    ge->type = GE_KEYDOWN;
#ifndef SDL2
    ge->key.which = se->key.which;
#else
    ge->key.which = se->key.padding2;
#endif
    ge->key.keysym = se->key.keysym.sym;
    break;
  case SDL_KEYUP:
    ge->type = GE_KEYUP;
#ifndef SDL2
    ge->key.which = se->key.which;
#else
    ge->key.which = se->key.padding2;
#endif
    ge->key.keysym = se->key.keysym.sym;
    break;
  case SDL_MOUSEBUTTONDOWN:
    ge->type = GE_MOUSEBUTTONDOWN;
    ge->button.which = se->button.which;
    ge->button.button = se->button.button;
    break;
  case SDL_MOUSEBUTTONUP:
    ge->type = GE_MOUSEBUTTONUP;
    ge->button.which = se->button.which;
    ge->button.button = se->button.button;
    break;
  case SDL_JOYBUTTONDOWN:
    ge->type = GE_JOYBUTTONDOWN;
    ge->jbutton.which = se->jbutton.which;
    ge->jbutton.button = se->jbutton.button;
    break;
  case SDL_JOYBUTTONUP:
    ge->type = GE_JOYBUTTONUP;
    ge->jbutton.which = se->jbutton.which;
    ge->jbutton.button = se->jbutton.button;
    break;
  case SDL_MOUSEMOTION:
    ge->type = GE_MOUSEMOTION;
    ge->motion.which = se->motion.which;
    ge->motion.xrel = se->motion.xrel;
    ge->motion.yrel = se->motion.yrel;
    break;
  case SDL_JOYAXISMOTION:
    ge->type = GE_JOYAXISMOTION;
    ge->jaxis.which = se->jaxis.which;
    ge->jaxis.axis = se->jaxis.axis;
    ge->jaxis.value = se->jaxis.value;
    break;
  case SDL_JOYHATMOTION:
    ge->type = GE_JOYHATMOTION;
    ge->jhat.which = se->jhat.which;
    ge->jhat.hat = se->jhat.hat;
    ge->jhat.value = se->jhat.value;
    break;
  }
}

static int joystick_hat_button(GE_Event* event, unsigned char hat_dir)
{
  return joystickNbButton[event->jhat.which] + 4*event->jhat.hat + log2(hat_dir);
}

static unsigned char get_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    return joystickHat[event->jhat.which][event->jhat.hat];
  }
  return 0;
}

static void set_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joystickNbHat[event->jhat.which])
  {
    joystickHat[event->jhat.which][event->jhat.hat] = event->jhat.value;
  }
}

/*
 * This function translates joystick hat events into joystick button events.
 * The joystick button events are inserted just before the joystick hat events.
 */
static int preprocess_events(GE_Event *events, int numevents)
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
              event->jbutton.button = joystick_hat_button((event+1), hat_dir);
              event++;
              numevents++;
              if(numevents == EVENT_BUFFER_SIZE)
              {
                return EVENT_BUFFER_SIZE;
              }
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
              event->jbutton.button = joystick_hat_button((event+1), hat_dir);
              event++;
              numevents++;
              if(numevents == EVENT_BUFFER_SIZE)
              {
                return EVENT_BUFFER_SIZE;
              }
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

static SDL_Event events[EVENT_BUFFER_SIZE];

int ev_peep_events(GE_Event* ev, int size)
{
  int i;

  if(size > EVENT_BUFFER_SIZE)
  {
    size = EVENT_BUFFER_SIZE;
  }

#ifndef SDL2
  int nb = SDL_PeepEvents(events, size, SDL_GETEVENT, SDL_ALLEVENTS);
#else
  int nb = SDL_PeepEvents(events, size, SDL_GETEVENT, SDL_KEYDOWN, SDL_JOYDEVICEREMOVED);
#endif

  for(i=0; i<nb; ++i)
  {
    convert_s2g(events+i, ev+i);
  }
  
  return preprocess_events(ev, nb);
}
