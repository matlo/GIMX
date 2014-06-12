/*
 Copyright (c) 2012 Mathieu Laurendeau
 License: GPLv3
 */

#include <SDL.h>
#include "winmm/manymouse.h"
#include <GE.h>
#include <events.h>
#include <math.h>
#include <winsock2.h>
#include <windows.h>
#include <timer.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "Sixaxis Control"

static SDL_Joystick* joysticks[GE_MAX_DEVICES] = {};
static SDL_GameController* controllers[GE_MAX_DEVICES] = {};
static int instanceIdToIndex[GE_MAX_DEVICES] = {};
static int j_num;
static int j_max;

static int joystickNbButton[GE_MAX_DEVICES] = {};
static int joystickNbHat[GE_MAX_DEVICES] = {};
static unsigned char* joystickHat[GE_MAX_DEVICES] = {};

int ev_init()
{
  int i;
  
  /* Init SDL */
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
  {
   fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
   return 0;
  }

  j_max = 0;
  for (i = 0; i < SDL_NumJoysticks(); ++i)
  {
    if (SDL_IsGameController(i))
    {
      if ((controllers[i] = SDL_GameControllerOpen(i)))
      {
        int instanceId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[i]));
        if(instanceId >= 0)
        {
          instanceIdToIndex[instanceId] = i;
          j_max++;
        }
        else
        {
          SDL_GameControllerClose(controllers[i]);
          controllers[i] = NULL;
        }
      }
      else
      {
        fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
        return 0;
      }
    }
    else
    {
      if((joysticks[i] = SDL_JoystickOpen(i)))
      {
        int instanceId = SDL_JoystickInstanceID(joysticks[i]);
        if(instanceId >= 0)
        {
          instanceIdToIndex[instanceId] = i;
          j_max++;
          joystickNbButton[i] = SDL_JoystickNumButtons(joysticks[i]);
          joystickNbHat[i] = SDL_JoystickNumHats(joysticks[i]);
          if(joystickNbHat[i] > 0)
          {
            joystickHat[i] = calloc(joystickNbHat[i], sizeof(unsigned char));
            if(!joystickHat[i])
            {
              fprintf(stderr, "Unable to allocate memory for joystick hats.\n");
              return 0;
            }
          }
        }
        else
        {
          SDL_JoystickClose(joysticks[i]);
          joysticks[i] = NULL;
        }
      }
    }
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
  ev_grab_input(GE_GRAB_OFF);
  ManyMouse_Quit();
}

const char* ev_joystick_name(int id)
{
  if(controllers[id])
  {
    return SDL_GameControllerName(controllers[id]);
  }
  return SDL_JoystickName(joysticks[id]);
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

    //Don't quit the joystick subsystem or the event queue will be disabled.
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
  int i;

  if(mode == GE_GRAB_ON)
  {
    HWND hwnd = FindWindow("ManyMouseRawInputCatcher", "ManyMouseRawInputMsgWindow");

    if(hwnd)
    {
      //clip the mouse cursor into the window
      RECT _clip;

      if(GetWindowRect(hwnd, &_clip))
      {
        ClipCursor(&_clip);
      }

      i = 10;
      while(i > 0 && ShowCursor(FALSE) >= 0) { i--; }
    }
  }
  else
  {
    ClipCursor(NULL);

    i = 10;
    while(i > 0 && ShowCursor(TRUE) < 0) { i--; }
  }
}

static int (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

#define MAX_SOURCES (MAXIMUM_WAIT_OBJECTS-1)

static struct
{
  int fd;
  int id;
  HANDLE handle;
  int (*fp_read)(int);
  int (*fp_cleanup)(int);
} sources[MAX_SOURCES] = {};

static unsigned int max_source = 0;

void ev_register_source(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  if(!fp_cleanup)
  {
    fprintf(stderr, "%s: the cleanup function is mandatory.", __FUNCTION__);
    return;
  }
  if(max_source < MAX_SOURCES)
  {
    HANDLE evt = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(WSAEventSelect(fd, evt, FD_READ | FD_CLOSE) == SOCKET_ERROR)
    {
      fprintf(stderr, "WSAEventSelect failed.");
      return;
    }
    
    sources[max_source].fd = fd;
    sources[max_source].id = id;
    sources[max_source].handle = evt;
    sources[max_source].fp_read = fp_read;
    sources[max_source].fp_cleanup = fp_cleanup;
    ++max_source;
  }
}

void ev_remove_source(int fd)
{
  int i;
  for(i=0; i<max_source; ++i)
  {
    if(sources[i].fd == fd)
    {
      WSACloseEvent(sources[i].handle);
      memmove(sources+i, sources+i+1, (max_source-i)*sizeof(*sources));
      --max_source;
      break;
    }
  }
}

static unsigned int fill_handles(HANDLE handles[])
{
  int i;
  for(i=0; i<max_source; ++i)
  {
    if(sources[i].fp_read)
    {
      handles[i] = sources[i].handle;
    }
  }

  return max_source;
}

void ev_pump_events()
{
  int num_evt;
  static GE_Event events[EVENT_BUFFER_SIZE];
  GE_Event* event;
  int i;

  static struct
  {
    short x;
    short y;
  } mouse[GE_MAX_DEVICES] = {};

  int num_mm_evt;
  static ManyMouseEvent mm_events[EVENT_BUFFER_SIZE];
  ManyMouseEvent* mm_event;
  
  Uint8 button = 0;

  HANDLE hTimer = timer_get();

  int result;

  do
  {
    HANDLE handles[max_source+1];
    handles[0] = hTimer;
    
    fill_handles(handles+1);
    
    result = MsgWaitForMultipleObjects(max_source+1, handles, FALSE, INFINITE, QS_RAWINPUT);

    if(result == WAIT_OBJECT_0 + max_source + 1)
    {
      num_mm_evt = ManyMouse_PollEvent(mm_events, sizeof(mm_events)/sizeof(*mm_events));
      for(mm_event=mm_events; mm_event < mm_events + num_mm_evt; ++mm_event)
      {
        if (mm_event->type == MANYMOUSE_EVENT_RELMOTION)
        {
          if (mm_event->item == 0)
          {
            mouse[mm_event->device].x += mm_event->value;
          }
          else
          {
            mouse[mm_event->device].y += mm_event->value;
          }
        }
        else if (mm_event->type == MANYMOUSE_EVENT_BUTTON)
        {
          switch (mm_event->item)
          {
          case 0:
            button = GE_BTN_LEFT;
            break;
          case 1:
            button = GE_BTN_RIGHT;
            break;
          case 2:
            button = GE_BTN_MIDDLE;
            break;
          case 3:
            button = GE_BTN_BACK;
            break;
          case 4:
            button = GE_BTN_FORWARD;
            break;
          }
          GE_Event ge = { };
          ge.button.type = mm_event->value ? GE_MOUSEBUTTONDOWN : GE_MOUSEBUTTONUP;
          ge.button.which = mm_event->device;
          ge.button.button = button;
          event_callback(&ge);
        }
        else if (mm_event->type == MANYMOUSE_EVENT_SCROLL)
        {
          if (mm_event->item == 0)
          {
            GE_Event ge = { };
            ge.button.type = GE_MOUSEBUTTONDOWN;
            ge.button.which = mm_event->device;
            ge.button.button = (mm_event->value > 0) ? GE_BTN_WHEELUP : GE_BTN_WHEELDOWN;
            event_callback(&ge);
            ge.button.type = GE_MOUSEBUTTONUP;
            event_callback(&ge);
          }
          else
          {
            GE_Event ge = { };
            ge.button.type = GE_MOUSEBUTTONDOWN;
            ge.button.which = mm_event->device;
            ge.button.button = (mm_event->value < 0) ? GE_BTN_WHEELLEFT : GE_BTN_WHEELRIGHT;
            event_callback(&ge);
            ge.button.type = GE_MOUSEBUTTONUP;
            event_callback(&ge);
          }
        }
        else if (mm_event->type == MANYMOUSE_EVENT_KEY)
        {
          GE_Event ge = { };
          ge.key.type = mm_event->value ? GE_KEYDOWN : GE_KEYUP;
          ge.key.which = mm_event->device;
          ge.key.keysym = mm_event->scancode;
          event_callback(&ge);
        }
      }
    }
    else if(result > WAIT_OBJECT_0)
    {
      WSANETWORKEVENTS NetworkEvents;
      
      for(i=0; i<max_source; ++i)
      {
        if(sources[i].handle == handles[result])
        {
          if(WSAEnumNetworkEvents(sources[i].fd, handles[result], &NetworkEvents))
          {
            fprintf(stderr, "WSAEnumNetworkEvents: %d\n", WSAGetLastError());
            sources[i].fp_cleanup(sources[i].id);
          }
          else
          {
            if(NetworkEvents.lNetworkEvents & FD_READ)
            {
              if(NetworkEvents.iErrorCode[FD_READ_BIT])
              {
                fprintf(stderr, "iErrorCode is set\n");
                sources[i].fp_cleanup(sources[i].id);
              }
              else
              {
                sources[i].fp_read(sources[i].id);
              }
            }
          }
          break;
        }
      }
    }

  } while(result != WAIT_OBJECT_0);

  for(i=0; i<m_num; ++i)
  {
    if(mouse[i].x || mouse[i].y)
    {
      GE_Event ge = {};
      ge.motion.type = GE_MOUSEMOTION;
      ge.motion.which = i;
      ge.motion.xrel = mouse[i].x;
      ge.motion.yrel = mouse[i].y;
      event_callback(&ge);
      mouse[i].x = 0;
      mouse[i].y = 0;
    }
  }

  SDL_PumpEvents();

  num_evt = GE_PeepEvents(events, sizeof(events) / sizeof(events[0]));

  if (num_evt > 0)
  {
    for (event = events; event < events + num_evt; ++event)
    {
      event_callback(event);
    }
  }
}

static inline void convert_g2s(SDL_Event* se, GE_Event* ge)
{
  switch (ge->type)
  {
  case GE_KEYDOWN:
    se->type = SDL_KEYDOWN;
    se->key.padding2 = ge->key.which;
    se->key.keysym.sym = ge->key.keysym;
    break;
  case GE_KEYUP:
    se->type = SDL_KEYUP;
    se->key.padding2 = ge->key.which;
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

static inline int convert_s2g(SDL_Event* se, GE_Event* ge)
{
  int index;
  switch (se->type)
  {
  case SDL_KEYDOWN:
    ge->type = GE_KEYDOWN;
    ge->key.which = se->key.padding2;
    ge->key.keysym = se->key.keysym.sym;
    break;
  case SDL_KEYUP:
    ge->type = GE_KEYUP;
    ge->key.which = se->key.padding2;
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
    index = instanceIdToIndex[se->jbutton.which];
    if(!joysticks[index])
    {
      return 0;
    }
    ge->type = GE_JOYBUTTONDOWN;
    ge->jbutton.which = index;
    ge->jbutton.button = se->jbutton.button;
    break;
  case SDL_JOYBUTTONUP:
    index = instanceIdToIndex[se->jbutton.which];
    if(!joysticks[index])
    {
      return 0;
    }
    ge->type = GE_JOYBUTTONUP;
    ge->jbutton.which = index;
    ge->jbutton.button = se->jbutton.button;
    break;
  case SDL_CONTROLLERBUTTONDOWN:
    ge->type = GE_JOYBUTTONDOWN;
    ge->jbutton.which = instanceIdToIndex[se->cbutton.which];
    ge->jbutton.button = se->cbutton.button;
    break;
  case SDL_CONTROLLERBUTTONUP:
    ge->type = GE_JOYBUTTONUP;
    ge->jbutton.which = instanceIdToIndex[se->cbutton.which];
    ge->jbutton.button = se->cbutton.button;
    break;
  case SDL_MOUSEMOTION:
    ge->type = GE_MOUSEMOTION;
    ge->motion.which = se->motion.which;
    ge->motion.xrel = se->motion.xrel;
    ge->motion.yrel = se->motion.yrel;
    break;
  case SDL_JOYAXISMOTION:
    index = instanceIdToIndex[se->jaxis.which];
    if(!joysticks[index])
    {
      return 0;
    }
    ge->type = GE_JOYAXISMOTION;
    ge->jaxis.which = index;
    ge->jaxis.axis = se->jaxis.axis;
    ge->jaxis.value = se->jaxis.value;
    break;
  case SDL_CONTROLLERAXISMOTION:
    ge->type = GE_JOYAXISMOTION;
    ge->jaxis.which = instanceIdToIndex[se->caxis.which];
    ge->jaxis.axis = se->caxis.axis;
    ge->jaxis.value = se->caxis.value;
    break;
  case SDL_JOYHATMOTION:
    index = instanceIdToIndex[se->jhat.which];
    if(!joysticks[index])
    {
      return 0;
    }
    ge->type = GE_JOYHATMOTION;
    ge->jhat.which = index;
    ge->jhat.hat = se->jhat.hat;
    ge->jhat.value = se->jhat.value;
    break;
  }
  return 1;
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
  int i, j;

  if(size > EVENT_BUFFER_SIZE)
  {
    size = EVENT_BUFFER_SIZE;
  }

  int nb = SDL_PeepEvents(events, size, SDL_GETEVENT, SDL_KEYDOWN, SDL_CONTROLLERDEVICEREMAPPED);

  j = 0;
  for(i=0; i<nb; ++i)
  {
    if(convert_s2g(events+i, ev+j))
    {
      j++;
    }
  }
  
  return preprocess_events(ev, j);
}
