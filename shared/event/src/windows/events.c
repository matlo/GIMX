/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <SDL/SDL.h>
#include <GE.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "Sixaxis Control"

static SDL_Surface *screen = NULL;

int ev_init()
{
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

  return 1;
}

void ev_set_caption (const char *title, const char *icon)
{
  SDL_WM_SetCaption(title, icon);
}

void ev_quit(void)
{
  SDL_Quit();
}

const char* ev_joystick_name(int id)
{
  return NULL;
}

const char* ev_mouse_name(int id)
{
  return SDL_GetMouseName(id);
}

const char* ev_keyboard_name(int id)
{
  return SDL_GetKeyboardName(id);
}

void ev_grab_input(int mode)
{
  SDL_WM_GrabInput(mode);
}

void ev_pump_events()
{
  SDL_PumpEvents();
}

static inline void convert_g2s(SDL_Event* se, GE_Event* ge)
{
  switch (ge->type)
  {
  case GE_KEYDOWN:
    se->type = SDL_KEYDOWN;
    se->key.which = ge->key.which;
    se->key.keysym.sym = ge->key.keysym;
    break;
  case GE_KEYUP:
    se->type = SDL_KEYUP;
    se->key.which = ge->key.which;
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
    se->type = SDL_JOYBUTTONDOWN;
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
    ge->key.which = se->key.which;
    ge->key.keysym = se->key.keysym.sym;
    break;
  case SDL_KEYUP:
    ge->type = GE_KEYUP;
    ge->key.which = se->key.which;
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
    ge->type = GE_JOYBUTTONDOWN;
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

static SDL_Event events[EVENT_BUFFER_SIZE];

int ev_peep_events(GE_Event* ev, int size)
{
  int i;

  if(size > EVENT_BUFFER_SIZE)
  {
    size = EVENT_BUFFER_SIZE;
  }

  int nb = SDL_PeepEvents(events, size, SDL_GETEVENT, SDL_ALLEVENTS);

  for(i=0; i<nb; ++i)
  {
    convert_s2g(events+i, ev+i);
  }

  return nb;
}
