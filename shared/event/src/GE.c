/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <conversion.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>
#ifndef WIN32
#include <timer.h>
#endif

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

static char* sixaxis_names[] =
{
  "Sony PLAYSTATION(R)3 Controller",
  "Sony Navigation Controller"
};

static char* joystickName[GE_MAX_DEVICES] = {};
static int joystickVirtualIndex[GE_MAX_DEVICES] = {};
static int joystickUsed[GE_MAX_DEVICES] = {};
static int joystickSixaxis[GE_MAX_DEVICES] = {};
static char* mouseName[GE_MAX_DEVICES] = {};
static int mouseVirtualIndex[GE_MAX_DEVICES] = {};
static char* keyboardName[GE_MAX_DEVICES] = {};
static int keyboardVirtualIndex[GE_MAX_DEVICES] = {};

static int grab = GE_GRAB_OFF;

static GE_MK_Mode mk_mode = GE_MK_MODE_MULTIPLE_INPUTS;

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
 * \bried Initializes the GE library.
 *
 * \return 1 if successful
 *         0 in case of error
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
 * \brief Release unused stuff. It currently only releases unused joysticks.
 */
void GE_release_unused()
{
  int i;
  for (i = 0; i < GE_MAX_DEVICES && joystickName[i]; ++i)
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
 * \brief Grab/Release the mouse cursor (Windows) or grab/release all keyboard and mouse event devices (Linux).
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
 * \brief Grab the mouse.
 */
void GE_grab()
{
  ev_grab_input(GE_GRAB_ON);
  grab = 1;
}

/*
 * \brief Free the mouse and keyboard names.
 */
void GE_FreeMKames()
{
  int i;
  for (i = 0; i < GE_MAX_DEVICES && mouseName[i]; ++i)
  {
    free(mouseName[i]);
    mouseName[i] = NULL;
  }
  for (i = 0; i < GE_MAX_DEVICES && keyboardName[i]; ++i)
  {
    free(keyboardName[i]);
    keyboardName[i] = NULL;
  }
}

/*
 * \brief Quit the GE library (free allocated data, release devices...).
 */
void GE_quit()
{
  int i;

  for (i = 0; i < GE_MAX_DEVICES; ++i)
  {
    if (joystickName[i])
    {
      free(joystickName[i]);
      joystickName[i] = NULL;
      ev_joystick_close(i);
    }
  }
  GE_FreeMKames();
  ev_quit();
}

/*
 * \brief Get the mouse name for a given index.
 * 
 * \param id  the mouse index (in the [0..GE_MAX_DEVICES[ range).
 * 
 * \return the mouse name if present, NULL otherwise.
 */
char* GE_MouseName(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return mouseName[id];
  }
  return NULL;
}

/*
 * \brief Get the keyboard name for a given index.
 * 
 * \param id  the keyboard index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the keyboard name if present, NULL otherwise.
 */
char* GE_KeyboardName(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return keyboardName[id];
  }
  return NULL;
}

/*
 * \brief Get the joystick name for a given index.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the joystick name if present, NULL otherwise.
 */
char* GE_JoystickName(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return joystickName[id];
  }
  return NULL;
}

/*
 * \brief Get the joystick virtual id for a given index.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the joystick name if present, NULL otherwise.
 */
int GE_JoystickVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return joystickVirtualIndex[id];
  }
  return 0;
}

/*
 * \brief Set a joystick to the "used" state, so that a call to GE_release_unused will keep it open.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 */
void GE_SetJoystickUsed(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    joystickUsed[id] = 1;
  }
}

/*
 * \brief Get the mouse virtual id for a given index.
 * 
 * \param id  the mouse index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the mouse name if present, NULL otherwise.
 */
int GE_MouseVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return mouseVirtualIndex[id];
  }
  return 0;
}

/*
 * \brief Get the keyboard virtual id for a given index.
 * 
 * \param id  the keyboard index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the keyboard name if present, NULL otherwise.
 */
int GE_KeyboardVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return keyboardVirtualIndex[id];
  }
  return 0;
}

/*
 * \brief Tell if a joystick is a sixaxis / dualshock / navigation controller given its index.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return 1 if it is such a joystick, 0 otherwise.
 */
int GE_IsSixaxis(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return joystickSixaxis[id];
  }
  return 0;
}

/*
 * \brief Get the mk mode.
 *
 * \return value GE_MK_MODE_MULTIPLE_INPUTS multiple mice and  multiple keyboards (default value),
 *               GE_MK_MODE_SINGLE_INPUT    single mouse and single keyboard
 */
inline GE_MK_Mode GE_GetMKMode()
{
  return mk_mode;
}

/*
 * \brief Set the mk mode.
 *
 * \param value GE_MK_MODE_MULTIPLE_INPUTS multiple mice and  multiple keyboards (default value),
 *              GE_MK_MODE_SINGLE_INPUT    single mouse and single keyboard
 */
inline void GE_SetMKMode(GE_MK_Mode value)
{
  mk_mode = value;
}

/*
 * \brief Returns the device id corresponding to a given event.
 * 
 * \param e  the event
 *
 * \return the device id (0 if the event is from a mouse or a keyboard and the mk mode is GE_MK_MODE_SINGLE_INPUT).
 */
int GE_GetDeviceId(GE_Event* e)
{
  /*
   * 'which' should always be at that place
   * There is no need to check the value, since it's stored as an uint8_t, and GE_MAX_DEVICES is 256.
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
      if (GE_GetMKMode() == GE_MK_MODE_SINGLE_INPUT)
      {
        device_id = 0;
      }
      break;
  }

  return device_id;
}

/*
 * \brief Push an event into the event queue.
 * 
 * \param e  the event
 *
 * \return the device id.
 */
int GE_PushEvent(GE_Event *event)
{
  return ev_push_event(event);
}

#ifndef WIN32
/*
 * \brief Set a callback function for processing events (Linux only).
 *
 * \param fp  the callback function
 */
void GE_SetCallback(int(*fp)(GE_Event*))
{
  ev_set_callback(fp);
}

/*
 * \brief Start a timer to make GE_PumpEvents return periodically (Linux only).
 * 
 * \param period  the period of the timer.
 */
void GE_TimerStart(struct timespec* period)
{
  int tfd = timer_start(period);

  if(tfd >= 0)
  {
    ev_register_source(tfd, 0, &timer_read, &timer_close);
  }
}

/*
 * \brief Stop the timer (Linux only).
 */
void GE_TimerClose()
{
  timer_close(0);
}

/*
 * Add an event source.
 */
void GE_AddSource(int fd, int id, int (*fd_read)(int), int (*fd_cleanup)(int))
{
  ev_register_source(fd, id, fd_read, fd_cleanup);
}

/*
 * Remove an event source.
 */
void GE_RemoveSource(int fd)
{
  ev_remove_source(fd);
}
#endif

/*
 * \brief Get events from devices.
 *        In Linux:
 *        - it is mandatory to call GE_SetCallback once before calling this function.
 *        - if GE_TimerStart wasn't previously called, this function will block undefinitely.
 *        In Windows:
 *        - this function queues all pending events and returns.
 */
void GE_PumpEvents()
{
  ev_pump_events();
}

/*
 * \brief Get all events from the event queue.
 * 
 * \param events  the buffer to store the events
 * \param numevents  the max number of events to retrieve
 * 
 * \return the number of retrieved events.
 */
int GE_PeepEvents(GE_Event *events, int numevents)
{
  return ev_peep_events(events, numevents);
}

/*
 * \brief Get the button name for a given button id.
 *
 * \param button  the button id
 *
 * \return the button name
 */
const char* GE_MouseButtonName(int button)
{
  return get_chars_from_button(button);
}

/*
 * \brief Get the button id for a given button name.
 *
 * \param name  the button name
 *
 * \return the button id
 */
int GE_MouseButtonId(const char* name)
{
  return get_mouse_event_id_from_buffer(name);
}

/*
 * \brief Get the key name for a given key id.
 *
 * \param key  the key id
 *
 * \return the key name
 */
const char* GE_KeyName(uint16_t key)
{
  return get_chars_from_key(key);
}

/*
 * \brief Get the key id for a given key name.
 *
 * \param name  the key name
 *
 * \return the key id
 */
uint16_t GE_KeyId(const char* name)
{
  return get_key_from_buffer(name);
}
