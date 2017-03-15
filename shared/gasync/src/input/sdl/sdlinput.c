/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "sdlinput.h"
#include "keycodes.h"

#include <SDL.h>

#include <ginput.h>
#include <common/gerror.h>

#define PRINT_ERROR_SDL(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %s\n", __FILE__, __LINE__, __func__, msg, SDL_GetError());

#define SCREEN_WIDTH  1
#define SCREEN_HEIGHT 1

static struct
{
  unsigned int sdltype;
  GE_HapticType type;
} effect_types[] =
{
  { SDL_HAPTIC_LEFTRIGHT, GE_HAPTIC_RUMBLE },
  { SDL_HAPTIC_CONSTANT,  GE_HAPTIC_CONSTANT },
  { SDL_HAPTIC_SPRING,    GE_HAPTIC_SPRING },
  { SDL_HAPTIC_DAMPER,    GE_HAPTIC_DAMPER }
};

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
    SDL_Haptic* haptic;
    unsigned int effects;
    int ids[sizeof(effect_types) / sizeof(*effect_types)];
    int (*haptic_cb)(const GE_Event * event);
  } force_feedback;
  struct
  {
    int joystickHatButtonBaseIndex; // the base index of the generated hat buttons equals the number of physical buttons
    int joystickNbHat; // the number of hats
    unsigned char* joystickHat; // the current hat values
  } hat_info; // allows to convert hat axes to buttons
  struct
  {
      unsigned short vendor;
      unsigned short product;
  } usb_ids;
} joysticks[GE_MAX_DEVICES] = {};

static struct
{
  short x;
  short y;
} mouse[GE_MAX_DEVICES] = {};

static int (*event_callback)(GE_Event*) = NULL;

int get_effect_id(int id, GE_HapticType type)
{
  int i = -1;
  switch(type)
  {
  case GE_HAPTIC_RUMBLE:
	  i = 0;
	  break;
  case GE_HAPTIC_CONSTANT:
	  i = 1;
	  break;
  case GE_HAPTIC_SPRING:
	  i = 2;
	  break;
  case GE_HAPTIC_DAMPER:
	  i = 3;
	  break;
  case GE_HAPTIC_NONE:
	  break;
  }
  if (i < 0)
  {
	return -1;
  }
  return joysticks[id].force_feedback.ids[i];
}

static void open_haptic(int id, SDL_Joystick* joystick) {

  joysticks[id].force_feedback.effects = GE_HAPTIC_NONE;
  
  SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joystick);
  if(haptic)
  {
    unsigned int features = SDL_HapticQuery(haptic);
    unsigned int i;
    for (i = 0; i < sizeof(effect_types) / sizeof(*effect_types); ++i)
    {
      if(features & effect_types[i].sdltype)
      {
        SDL_HapticEffect effect = {
          .type = effect_types[i].sdltype,
        };
        switch (effect_types[i].sdltype) {
        case SDL_HAPTIC_LEFTRIGHT:
            effect.leftright.length = SDL_HAPTIC_INFINITY;
            break;
        case SDL_HAPTIC_CONSTANT:
            effect.constant.length = SDL_HAPTIC_INFINITY;
            effect.constant.direction.type = SDL_HAPTIC_FIRST_AXIS;
            effect.constant.direction.dir[0] = 0;
            break;
        case SDL_HAPTIC_SPRING:
        case SDL_HAPTIC_DAMPER:
            effect.condition.length = SDL_HAPTIC_INFINITY;
            effect.condition.direction.type = SDL_HAPTIC_FIRST_AXIS;
            effect.condition.direction.dir[0] = 0;
            break;
        }
        int effect_id = SDL_HapticNewEffect(haptic, &effect);
        if(effect_id >= 0)
        {
          joysticks[id].force_feedback.haptic = haptic;
          joysticks[id].force_feedback.effects |= effect_types[i].type;
          joysticks[id].force_feedback.ids[i] = effect_id;
        }
        else
        {
          PRINT_ERROR_SDL("SDL_HapticNewEffect")
        }
      }
    }
    if (joysticks[id].force_feedback.effects == GE_HAPTIC_NONE)
    {
      SDL_HapticClose(haptic);
    }
  }
}

int sdlinput_init(unsigned char mkb_src, int(*callback)(GE_Event*))
{
  int i;

  if (callback == NULL) {
    fprintf(stderr, "callback cannot be NULL\n");
    return -1;
  }
  
  event_callback = callback;

  /* Init SDL */
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0)
  {
   PRINT_ERROR_SDL("SDL_Init")
   return -1;
  }

  mkb_source = mkb_src;

  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
      PRINT_ERROR_SDL("SDL_InitSubSystem")
      return -1;
    }

    /* Init video */
    window = SDL_CreateWindow(SDLINPUT_WINDOW_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (window == NULL)
    {
      PRINT_ERROR_SDL("SDL_CreateWindow")
      return -1;
    }
  }

  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

  for (i = 0; i < SDL_NumJoysticks() && joysticks_nb < GE_MAX_DEVICES; ++i)
  {
    if (SDL_IsGameController(i))
    {
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller)
      {
        SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
        int instanceId = SDL_JoystickInstanceID(joystick);
        if(instanceId >= 0)
        {
          SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
          joysticks[joysticks_nb].usb_ids.vendor = guid.data[1] << 8 | guid.data[0];
          joysticks[joysticks_nb].usb_ids.product = guid.data[3] << 8 | guid.data[2];
          joysticks[joysticks_nb].controller = controller;
          open_haptic(joysticks_nb, joystick);
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
        PRINT_ERROR_SDL("SDL_GameControllerOpen")
        return -1;
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
          SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
          joysticks[joysticks_nb].usb_ids.vendor = guid.data[1] << 8 | guid.data[0];
          joysticks[joysticks_nb].usb_ids.product = guid.data[3] << 8 | guid.data[2];
          joysticks[joysticks_nb].joystick = joystick;
          open_haptic(joysticks_nb, joystick);
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

  return 0;
}

void sdlinput_quit(void)
{
  int i;
  for(i=0; i<joysticks_nb; ++i)
  {
    sdlinput_joystick_close(i);
  }
  joysticks_nb = 0;

  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    SDL_DestroyWindow(window);
  }

  SDL_Quit();
}

const char* sdlinput_joystick_name(int id)
{
  if(id < 0 || id >= joysticks_nb)
  {
    return NULL;
  }
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

int sdlinput_joystick_register(const char* name, unsigned int effects, int (*haptic_cb)(const GE_Event * event))
{
  int index = -1;
  if(joysticks_nb < GE_MAX_DEVICES)
  {
    index = joysticks_nb;
    joysticks[index].name = strdup(name);
    joysticks[index].force_feedback.effects = effects;
    joysticks[index].force_feedback.haptic_cb = haptic_cb;
    ++joysticks_nb;
    ++joysticks_registered;
  }
  return index;
}

/*
 * Close a joystick, and close the joystick subsystem if none is used anymore.
 */
void sdlinput_joystick_close(int id)
{
  if(id < 0 || id >= joysticks_nb)
  {
    return;
  }
  int closed = 0;
  if(joysticks[id].name)
  {
    free(joysticks[id].name);
    joysticks[id].name = NULL;
    closed = 1;
    --joysticks_registered;
  }
  else
  {
    if(joysticks[id].force_feedback.haptic)
    {
      SDL_HapticClose(joysticks[id].force_feedback.haptic);
      joysticks[id].force_feedback.haptic = NULL;
      joysticks[id].force_feedback.effects = GE_HAPTIC_NONE;
    }
    if(joysticks[id].joystick)
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
  }
  if(closed)
  {
    --joysticks_opened;

    // Closing the joystick subsystem also closes SDL's event queue.
    // Don't close it if the mouse and keyboard event source is the window system.
    if(joysticks_opened == joysticks_registered && mkb_source != GE_MKB_SOURCE_WINDOW_SYSTEM)
    {
      SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
    }
  }
}

static unsigned int m_num = 0;

const char* sdlinput_mouse_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

const char* sdlinput_keyboard_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    if(id == 0)
    {
      return "Window Events";
    }
  }
  return NULL;
}

static int sdl_peep_events(GE_Event* events, int size);

void sdlinput_sync_process()
{
  unsigned int i;
  int num_evt;
  static GE_Event events[EVENT_BUFFER_SIZE];
  GE_Event* event;

  /*
   * Pump events only if a joystick is opened
   * or if the mouse and keyboard source is the window system.
   */
  if(joysticks_opened != joysticks_registered || mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
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

static inline int convert_s2g(SDL_Event* se, GE_Event* ge, int size)
{
  int nb = 1;
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
    if (size >= 2)
    {
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
      *(ge + 1) = *ge;
      (ge + 1)->type = GE_MOUSEBUTTONUP;
      nb = 2;
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
  return nb;
}

static int joystick_hat_button(GE_Event* event, unsigned char hat_dir)
{
  return joysticks[event->jhat.which].hat_info.joystickHatButtonBaseIndex + 4*event->jhat.hat + log2(hat_dir);
}

static unsigned char get_joystick_hat(GE_Event* event)
{
  if(event->jhat.hat < joysticks[event->jhat.which].hat_info.joystickNbHat)
  {
    return joysticks[event->jhat.which].hat_info.joystickHat[event->jhat.hat];
  }
  return 0;
}

static void set_joystick_hat(GE_Event* event)
{
  if(event->jhat.hat < joysticks[event->jhat.which].hat_info.joystickNbHat)
  {
    joysticks[event->jhat.which].hat_info.joystickHat[event->jhat.hat] = event->jhat.value;
  }
}

/*
 * This function translates joystick hat events into joystick button events.
 * The joystick button events are inserted just before the joystick hat events.
 */
static int hats_to_buttons(GE_Event *events, int numevents)
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
        for (hat_dir = 1; hat_dir < 16 && event < events + numevents; hat_dir *= 2)
        {
          if (event->jhat.value & hat_dir) {
            if (!(get_joystick_hat(event) & hat_dir)) {
              /*
               * The hat direction is pressed.
               */
              memmove(event + 1, event, (events + numevents - event) * sizeof(GE_Event));
              event->type = GE_JOYBUTTONDOWN;
              event->jbutton.which = (event + 1)->jhat.which;
              event->jbutton.button = joystick_hat_button((event + 1), hat_dir);
              event++;
              numevents++;
              if (numevents == EVENT_BUFFER_SIZE) {
                return EVENT_BUFFER_SIZE;
              }
            }
          } else {
            if (get_joystick_hat(event) & hat_dir) {
              /*
               * The hat direction is released.
               */
              memmove(event + 1, event, (events + numevents - event) * sizeof(GE_Event));
              event->type = GE_JOYBUTTONUP;
              event->jbutton.which = (event + 1)->jhat.which;
              event->jbutton.button = joystick_hat_button((event + 1), hat_dir);
              event++;
              numevents++;
              if (numevents == EVENT_BUFFER_SIZE) {
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

static int sdl_peep_events(GE_Event* events, int size)
{
  static SDL_Event sdl_events[EVENT_BUFFER_SIZE];

  int i, j;

  if(size > EVENT_BUFFER_SIZE)
  {
    size = EVENT_BUFFER_SIZE;
  }

  unsigned int minType = SDL_JOYAXISMOTION;
  unsigned int maxType = SDL_CONTROLLERDEVICEREMAPPED;

  if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    minType = SDL_KEYDOWN;
  }

  if (joysticks_opened == joysticks_registered)
  {
    maxType = SDL_MOUSEWHEEL;
  }

  if (minType > maxType)
  {
    return 0;
  }

  int nb = SDL_PeepEvents(sdl_events, size, SDL_GETEVENT, minType, maxType);

  if (nb < 0) {
    PRINT_ERROR_SDL("SDL_PeepEvents")
    return -1;
  }

  j = 0;
  for (i = 0; i < nb && j < size; ++i)
  {
    j += convert_s2g(sdl_events + i, events + j, size - j);
  }

  return hats_to_buttons(events, j);
}

int sdlinput_joystick_get_haptic(int joystick)
{
  if(joystick < 0 || joystick >= joysticks_nb)
  {
    return -1;
  }
  return joysticks[joystick].force_feedback.effects;
}

int sdlinput_joystick_set_haptic(const GE_Event * event)
{
  if(event->which >= joysticks_nb)
  {
    PRINT_ERROR_OTHER("Invalid joystick id.")
    return -1;
  }
  if (joysticks[event->which].controller == NULL && joysticks[event->which].joystick == NULL) {
    if (joysticks[event->which].force_feedback.haptic_cb != NULL) {
      return joysticks[event->which].force_feedback.haptic_cb(event);
    } else {
      PRINT_ERROR_OTHER("External joystick has no haptic callback.")
      return -1;
    }
  }
  int effect_id = -1;
  SDL_HapticEffect effect = {};
  unsigned int effects = joysticks[event->which].force_feedback.effects;
  switch (event->type)
  {
  case GE_JOYRUMBLE:
    if (effects & GE_HAPTIC_RUMBLE)
    {
      effect_id = get_effect_id(event->which, GE_HAPTIC_RUMBLE);
      effect.leftright.type = SDL_HAPTIC_LEFTRIGHT;
      effect.leftright.length = 0;
      effect.leftright.large_magnitude = event->jrumble.strong;
      effect.leftright.small_magnitude = event->jrumble.weak;
    }
    break;
  case GE_JOYCONSTANTFORCE:
    if (effects & GE_HAPTIC_CONSTANT)
    {
      effect_id = get_effect_id(event->which, GE_HAPTIC_CONSTANT);
      effect.constant.type = SDL_HAPTIC_CONSTANT;
      effect.constant.direction.type = SDL_HAPTIC_FIRST_AXIS;
      effect.constant.direction.dir[0] = 0;
      effect.constant.length = SDL_HAPTIC_INFINITY;
      effect.constant.level = event->jconstant.level;
    }
    break;
  case GE_JOYSPRINGFORCE:
    if (effects & GE_HAPTIC_SPRING)
    {
      effect_id = get_effect_id(event->which, GE_HAPTIC_SPRING);
      effect.condition.type = SDL_HAPTIC_SPRING;
      effect.condition.direction.type = SDL_HAPTIC_FIRST_AXIS;
      effect.condition.direction.dir[0] = 0;
      effect.condition.length = SDL_HAPTIC_INFINITY;
      effect.condition.right_sat[0] = event->jcondition.saturation.right;
      effect.condition.left_sat[0] = event->jcondition.saturation.left;
      effect.condition.right_coeff[0] = event->jcondition.coefficient.right;
      effect.condition.left_coeff[0] = event->jcondition.coefficient.left;
      effect.condition.center[0] = event->jcondition.center;
      effect.condition.deadband[0] = event->jcondition.deadband;
    }
    break;
  case GE_JOYDAMPERFORCE:
    if (effects & GE_HAPTIC_DAMPER)
    {
      effect_id = get_effect_id(event->which, GE_HAPTIC_DAMPER);
      effect.condition.type = SDL_HAPTIC_DAMPER;
      effect.condition.direction.type = SDL_HAPTIC_FIRST_AXIS;
      effect.condition.direction.dir[0] = 0;
      effect.condition.length = SDL_HAPTIC_INFINITY;
      effect.condition.right_sat[0] = event->jcondition.saturation.right;
      effect.condition.left_sat[0] = event->jcondition.saturation.left;
      effect.condition.right_coeff[0] = event->jcondition.coefficient.right;
      effect.condition.left_coeff[0] = event->jcondition.coefficient.left;
    }
    break;
  default:
    break;
  }
  if (effect_id != -1)
  {
    if(SDL_HapticUpdateEffect(joysticks[event->which].force_feedback.haptic, effect_id, &effect))
    {
      PRINT_ERROR_SDL("SDL_HapticUpdateEffect")
      return -1;
    }
    if(SDL_HapticRunEffect(joysticks[event->which].force_feedback.haptic, effect_id, 1))
    {
      PRINT_ERROR_SDL("SDL_HapticRunEffect")
      return -1;
    }
  }
  return 0;
}

int sdlinput_joystick_get_usb_ids(int joystick, unsigned short * vendor, unsigned short * product)
{
  if(joystick < 0 || joystick >= joysticks_nb)
  {
    return -1;
  }
  *vendor = joysticks[joystick].usb_ids.vendor;
  *product = joysticks[joystick].usb_ids.product;
  return 0;
}

void sdlinput_grab(int mode)
{
  SDL_SetRelativeMouseMode((mode == GE_GRAB_ON) ? SDL_TRUE : SDL_FALSE);
}
