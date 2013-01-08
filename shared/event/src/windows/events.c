/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <SDL/SDL.h>
#include <GE.h>
#include <events.h>
#include <math.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "Sixaxis Control"

static SDL_Surface *screen = NULL;

static SDL_Joystick* joysticks[MAX_DEVICES] = {};
static int j_num;
static int j_max;

int joystickNbButton[MAX_DEVICES] = {};
int joystickNbHat[MAX_DEVICES] = {};
unsigned char* joystickHat[MAX_DEVICES] = {};

int ev_init()
{
  int i;
  
  /* Init SDL */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
  {
   fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
   return 0;
  }

  /* Init video */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
     SDL_HWSURFACE | SDL_ANYFORMAT | SDL_NOFRAME);
  if (screen == NULL)
  {
   fprintf(stderr, "Unable to create video surface: %s\n", SDL_GetError());
   return 0;
  }
  
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
  
  SDL_WarpMouse(100, 100);
}

const char* ev_joystick_name(int id)
{
  return SDL_JoystickName(id);
}

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

  SDL_ShowCursor(SDL_DISABLE);
}

void ev_set_callback(int (*fp)(GE_Event*))
{
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

  int nb = SDL_PeepEvents(events, size, SDL_GETEVENT, SDL_ALLEVENTS);

  for(i=0; i<nb; ++i)
  {
    convert_s2g(events+i, ev+i);
  }
  
  return preprocess_events(ev, nb);
}
