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
#include <queue.h>

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1
#define TITLE "gimx"

static unsigned char mkb_source;

static SDL_Window* window = NULL;

static int instanceIdToIndex[GE_MAX_DEVICES] = {};

static int joysticks_nb = 0;
// Keep tracking of the number of registered joysticks (externally handled) and the
// number of opened joysticks, so as to be able to close the joystick subsystem
// and to avoid pumping the SDL library events when no joystick is used.
static int joysticks_registered = 0;
static int joysticks_opened;

static struct
{
  char* name; // registered joysticks (externally handled)
  SDL_Joystick* joystick;
  SDL_GameController* controller;
  struct
  {
    int joystickHatButtonBaseIndex; // the base index of the generated hat buttons equals the number of physical buttons
    int joystickNbHat; // the number of hats
    unsigned char* joystickHat; // the current hat values
  } hat_info; // allows to convert hat axes to buttons
} joysticks[GE_MAX_DEVICES] = {};

int ev_init(unsigned char mkb_src)
{
  int i;
  
  /* Init SDL */
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
  {
   fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
   return 0;
  }

  for (i = 0; i < SDL_NumJoysticks(); ++i)
  {
    if (SDL_IsGameController(i))
    {
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller)
      {
        int instanceId = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
        if(instanceId >= 0)
        {
          joysticks[joysticks_nb].controller = controller;
          instanceIdToIndex[instanceId] = joysticks_nb;
          ++joysticks_nb;
        }
        else
        {
          SDL_GameControllerClose(controller);
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
      SDL_Joystick* joystick = SDL_JoystickOpen(i);
      if(joystick)
      {
        int instanceId = SDL_JoystickInstanceID(joystick);
        if(instanceId >= 0)
        {
          joysticks[joysticks_nb].joystick = joystick;
          instanceIdToIndex[instanceId] = joysticks_nb;
          joysticks[joysticks_nb].hat_info.joystickHatButtonBaseIndex = SDL_JoystickNumButtons(joystick);
          joysticks[joysticks_nb].hat_info.joystickNbHat = SDL_JoystickNumHats(joystick);
          if(joysticks[joysticks_nb].hat_info.joystickNbHat > 0)
          {
            joysticks[joysticks_nb].hat_info.joystickHat = calloc(joysticks[joysticks_nb].hat_info.joystickNbHat, sizeof(unsigned char));
            if(!joysticks[joysticks_nb].hat_info.joystickHat)
            {
              fprintf(stderr, "Unable to allocate memory for joystick hats.\n");
            }
          }
          ++joysticks_nb;
        }
        else
        {
          SDL_JoystickClose(joystick);
        }
      }
    }
  }

  joysticks_opened = joysticks_nb;

  mkb_source = mkb_src;

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    if(ManyMouse_Init() < 0)
    {
      return 0;
    }
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
      fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
      return 0;
    }

    /* Init video */
    window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (window == NULL)
    {
      fprintf(stderr, "Unable to create video surface: %s\n", SDL_GetError());
      return 0;
    }
  }

  queue_init();

  return 1;
}

void ev_quit(void)
{
  int i;
  for(i=0; i<joysticks_nb; ++i)
  {
    ev_joystick_close(i);
  }
  joysticks_nb = 0;

  ev_grab_input(GE_GRAB_OFF);

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    ManyMouse_Quit();
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    SDL_DestroyWindow(window);
  }

  SDL_Quit();
}

const char* ev_joystick_name(int id)
{
  if(joysticks[id].name)
  {
    return joysticks[id].name;
  }
  if(joysticks[id].controller)
  {
    return SDL_GameControllerName(joysticks[id].controller);
  }
  return SDL_JoystickName(joysticks[id].joystick);
}

int ev_joystick_register(const char* name)
{
  int index = -1;
  if(joysticks_nb < GE_MAX_DEVICES)
  {
    index = joysticks_nb;
    joysticks[index].name = strdup(name);
    ++joysticks_nb;
    ++joysticks_registered;
  }
  return index;
}

/*
 * Close a joystick, and close the joystick subsystem if none is used anymore.
 */
void ev_joystick_close(int id)
{
  int closed = 0;
  if(joysticks[id].name)
  {
    free(joysticks[id].name);
    joysticks[id].name = NULL;
    closed = 1;
    --joysticks_registered;
  }
  else if(joysticks[id].joystick)
  {
    SDL_JoystickClose(joysticks[id].joystick);
    joysticks[id].joystick = NULL;
    free(joysticks[id].hat_info.joystickHat);
    joysticks[id].hat_info.joystickHat = NULL;
    closed = 1;
  }
  else if(joysticks[id].controller)
  {
    SDL_GameControllerClose(joysticks[id].controller);
    joysticks[id].controller = NULL;
    closed = 1;
  }
  if(closed)
  {
    --joysticks_opened;

    // Closing the joystick subsystem also closes SDL's event queue,
    // but we don't care as we won't use it anymore.
    if(joysticks_opened == joysticks_registered)
    {
      SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
  }
}

static int m_num = 0;

const char* ev_mouse_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
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
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

const char* ev_keyboard_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return ManyMouse_KeyboardName(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

void ev_grab_input(int mode)
{
  int i;

  if(mode == GE_GRAB_ON)
  {
    HWND hwnd = NULL;

    if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
    {
      hwnd = FindWindow("ManyMouseRawInputCatcher", "ManyMouseRawInputMsgWindow");
    }
    else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
    {
      SDL_SetRelativeMouseMode(SDL_TRUE);

      hwnd = FindWindow(NULL, "gimx");
    }

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

    if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
    {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }
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

/*
 * Register a socket as an event source.
 * Note that the socket becomes non-blocking.
 */
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

void ev_register_source_handle(HANDLE handle, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  if(!fp_cleanup)
  {
    fprintf(stderr, "%s: the cleanup function is mandatory.", __FUNCTION__);
    return;
  }
  if(max_source < MAX_SOURCES)
  {
    sources[max_source].fd = -1;
    sources[max_source].id = id;
    sources[max_source].handle = handle;
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

static int sdl_peep_events(GE_Event* events, int size);

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
  
  Uint8 button = 0;

  HANDLE hTimer = timer_get();

  int result;
  int done = 0;
  int count;

  do
  {
    count = max_source;
    
    if(hTimer)
    {
      ++count;
    }
    
    HANDLE handles[max_source+1];
    fill_handles(handles);
    handles[max_source] = hTimer;
    
    unsigned int dwWakeMask = 0;

    if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
    {
      dwWakeMask = QS_RAWINPUT;
    }

    result = MsgWaitForMultipleObjects(count, handles, FALSE, INFINITE, dwWakeMask);

    if(result == WAIT_FAILED)
    {
      fprintf(stderr, "MsgWaitForMultipleObjects failed with error %d\n", result);
    }
    else if(result == WAIT_OBJECT_0 + count)
    {
      if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
      {
        static ManyMouseEvent mm_events[EVENT_BUFFER_SIZE];
        ManyMouseEvent* mm_event;

        int num_mm_evt = ManyMouse_PollEvent(mm_events, sizeof(mm_events)/sizeof(*mm_events));
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
              queue_push_event(&ge);
            }
            else
            {
              GE_Event ge = { };
              ge.button.type = GE_MOUSEBUTTONDOWN;
              ge.button.which = mm_event->device;
              ge.button.button = (mm_event->value < 0) ? GE_BTN_WHEELLEFT : GE_BTN_WHEELRIGHT;
              event_callback(&ge);
              ge.button.type = GE_MOUSEBUTTONUP;
              queue_push_event(&ge);
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
    }
    else if(handles[result] == hTimer)
    {
      done = 1;
    }
    else
    {
      WSANETWORKEVENTS NetworkEvents;
      
      for(i=0; i<max_source; ++i)
      {
        if(sources[i].handle == handles[result])
        {
          if(sources[i].fd >= 0)
          {
            /*
             * Network source
             */
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
                  fprintf(stderr, "iErrorCode[FD_READ_BIT] is set\n");
                  sources[i].fp_cleanup(sources[i].id);
                }
                else
                {
                  if(sources[i].fp_read(sources[i].id))
                  {
                    done = 1;
                  }
                }
              }
            }
          }
          else
          {
            /*
             * Serial source
             */
            if(sources[i].fp_read(sources[i].id))
            {
              done = 1;
            }
          }
          break;
        }
      }
    }

  } while(!done);

  if(joysticks_opened != joysticks_registered)
  {
    SDL_PumpEvents();
  }

  num_evt = sdl_peep_events(events, sizeof(events) / sizeof(events[0]));

  if (num_evt > 0)
  {
    for (event = events; event < events + num_evt; ++event)
    {
      event_callback(event);
    }
  }

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
}

extern unsigned char get_keycode_from_scancode(unsigned short scancode);

static inline int convert_s2g(SDL_Event* se, GE_Event* ge)
{
  int index;
  switch (se->type)
  {
  case SDL_KEYDOWN:
    ge->type = GE_KEYDOWN;
    ge->key.which = 0;
    ge->key.keysym = get_keycode_from_scancode(se->key.keysym.scancode);
    break;
  case SDL_KEYUP:
    ge->type = GE_KEYUP;
    ge->key.which = 0;
    ge->key.keysym = get_keycode_from_scancode(se->key.keysym.scancode);
    break;
  case SDL_MOUSEBUTTONDOWN:
    ge->type = GE_MOUSEBUTTONDOWN;
    ge->button.which = 0;
    ge->button.button = se->button.button;
    break;
  case SDL_MOUSEBUTTONUP:
    ge->type = GE_MOUSEBUTTONUP;
    ge->button.which = 0;
    ge->button.button = se->button.button;
    break;
  case SDL_MOUSEWHEEL:
    ge->type = GE_MOUSEBUTTONDOWN;
    ge->button.which = 0;
    if(se->wheel.x > 0)
    {
      ge->button.button = GE_BTN_WHEELRIGHT;
    }
    else if(se->wheel.x < 0)
    {
      ge->button.button = GE_BTN_WHEELLEFT;
    }
    else if(se->wheel.y > 0)
    {
      ge->button.button = GE_BTN_WHEELUP;
    }
    else if(se->wheel.y < 0)
    {
      ge->button.button = GE_BTN_WHEELDOWN;
    }
    break;
  case SDL_JOYBUTTONDOWN:
    index = instanceIdToIndex[se->jbutton.which];
    if(!joysticks[index].joystick)
    {
      return 0;
    }
    ge->type = GE_JOYBUTTONDOWN;
    ge->jbutton.which = index;
    ge->jbutton.button = se->jbutton.button;
    break;
  case SDL_JOYBUTTONUP:
    index = instanceIdToIndex[se->jbutton.which];
    if(!joysticks[index].joystick)
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
    ge->motion.which = 0;
    ge->motion.xrel = se->motion.xrel;
    ge->motion.yrel = se->motion.yrel;
    break;
  case SDL_JOYAXISMOTION:
    index = instanceIdToIndex[se->jaxis.which];
    if(!joysticks[index].joystick)
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
    if(!joysticks[index].joystick)
    {
      return 0;
    }
    ge->type = GE_JOYHATMOTION;
    ge->jhat.which = index;
    ge->jhat.hat = se->jhat.hat;
    ge->jhat.value = se->jhat.value;
    break;
  default:
    return 0;
  }
  return 1;
}

static int joystick_hat_button(GE_Event* event, unsigned char hat_dir)
{
  return joysticks[event->jhat.which].hat_info.joystickHatButtonBaseIndex + 4*event->jhat.hat + log2(hat_dir);
}

static unsigned char get_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joysticks[event->jhat.which].hat_info.joystickNbHat)
  {
    return joysticks[event->jhat.which].hat_info.joystickHat[event->jhat.hat];
  }
  return 0;
}

static void set_joystick_hat(GE_Event* event)
{
  if(event->jhat.which >= 0 && event->jhat.hat < joysticks[event->jhat.which].hat_info.joystickNbHat)
  {
    joysticks[event->jhat.which].hat_info.joystickHat[event->jhat.hat] = event->jhat.value;
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

static SDL_Event sdl_events[EVENT_BUFFER_SIZE];

static int sdl_peep_events(GE_Event* events, int size)
{
  int i, j;

  if(size > EVENT_BUFFER_SIZE)
  {
    size = EVENT_BUFFER_SIZE;
  }

  unsigned int minType = SDL_JOYAXISMOTION;

  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    minType = SDL_KEYDOWN;
  }

  int nb = SDL_PeepEvents(sdl_events, size, SDL_GETEVENT, minType, SDL_CONTROLLERDEVICEREMAPPED);

  j = 0;
  for(i=0; i<nb; ++i)
  {
    if(convert_s2g(sdl_events+i, events+j))
    {
      j++;
    }
  }

  return preprocess_events(events, j);
}
