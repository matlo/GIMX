/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <queue.h>
#include <conversion.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>
#include <timer.h>
#include <stdio.h>
#ifndef WIN32
#include <poll.h>
#else
#include <windows.h>
#endif

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"
#define XONE_PAD_NAME "Microsoft X-Box One pad"

static struct
{
  const char* name;
  GE_JS_Type type;
} js_types[] =
{
#ifndef WIN32
  { "Sony PLAYSTATION(R)3 Controller", GE_JS_SIXAXIS },
  { "Sony Navigation Controller", GE_JS_SIXAXIS },
  { "Sony Computer Entertainment Wireless Controller", GE_JS_DS4 },
  { "Microsoft X-Box 360 pad", GE_JS_360PAD },
  { "Microsoft X-Box One pad", GE_JS_XONEPAD },
#else
  { "PS4 Controller", GE_JS_DS4 },
  { "X360 Controller", GE_JS_360PAD },
  { "XBOX 360 For Windows (Controller)", GE_JS_360PAD },
#endif
};

static struct
{
  char* name;
  int virtualIndex;
  unsigned char isUsed;
  GE_JS_Type type;
} joysticks[GE_MAX_DEVICES] = {};

static struct
{
  char* name;
  int virtualIndex;
} mice[GE_MAX_DEVICES] = {};

static struct
{
  char* name;
  int virtualIndex;
} keyboards[GE_MAX_DEVICES] = {};

static int grab = GE_GRAB_OFF;

static GE_MK_Mode mk_mode = GE_MK_MODE_MULTIPLE_INPUTS;

static int initialized = 0;

/*
 * Convert an ISO-8859-1 string to an UTF-8 string.
 * The returned string is hold in a statically allocated buffer that is modified at each call.
 */
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
 * \param mkb_src GE_MKB_SOURCE_PHYSICAL: use evdev under Linux and raw inputs under Windows.
 *                GE_MKB_SOURCE_WINDOW_SYSTEM: use X inputs under Linux and the SDL library under Windows.
 *
 * \return 1 if successful
 *         0 in case of error
 */
int GE_initialize(unsigned char mkb_src)
{
  int i = 0;
  int j;
  const char* name;

  if (!ev_init(mkb_src))
  {
    return 0;
  }

  i = 0;
  while (i < GE_MAX_DEVICES && (name = ev_joystick_name(i)))
  {
    joysticks[i].type = GE_JS_OTHER; //default value

    if (!strncmp(name, BT_SIXAXIS_NAME, sizeof(BT_SIXAXIS_NAME) - 1))
    {
      // Rename QtSixA devices.
      name = "Sony PLAYSTATION(R)3 Controller";
      joysticks[i].type = GE_JS_SIXAXIS;
    }
#ifdef WIN32
    if (!strncmp(name, XONE_PAD_NAME, sizeof(XONE_PAD_NAME) - 1))
    {
      // In Windows, rename joysticks that are named XONE_PAD_NAME.
      // Such controllers are registered using GE_RegisterJoystick().
      // It's currently not possible to distinguish Xbox One controllers
      // from Xbox 360 controllers, as Xinput does not provide controller names.
      name = "X360 Controller";
      joysticks[i].type = GE_JS_XONEPAD;
    }
#endif

    joysticks[i].name = strdup(_8BIT_to_UTF8(name));

    // Go backward and look for a joystick with the same name.
    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(joysticks[i].name, joysticks[j].name))
      {
        // Found => compute the virtual index.
        joysticks[i].virtualIndex = joysticks[j].virtualIndex + 1;
        break;
      }
    }
    if (j < 0)
    {
      // Not found => the virtual index is 0.
      joysticks[i].virtualIndex = 0;
    }
    if(joysticks[i].type == GE_JS_OTHER)
    {
      // Determine the joystick type.
      for (j = 0; (unsigned int) j < sizeof(js_types) / sizeof(*js_types); ++j)
      {
        if (!strcmp(joysticks[i].name, js_types[j].name))
        {
          joysticks[i].type = js_types[j].type;
          break;
        }
      }
    }
    i++;
  }
  i = 0;
  while (i < GE_MAX_DEVICES && (name = ev_mouse_name(i)))
  {
    mice[i].name = strdup(_8BIT_to_UTF8(name));

    // Go backward and look for a mouse with the same name.
    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(mice[i].name, mice[j].name))
      {
        // Found => compute the virtual index.
        mice[i].virtualIndex = mice[j].virtualIndex + 1;
        break;
      }
    }
    if (j < 0)
    {
      // Not found => the virtual index is 0.
      mice[i].virtualIndex = 0;
    }
    i++;
  }
  i = 0;
  while (i < GE_MAX_DEVICES && (name = ev_keyboard_name(i)))
  {
    keyboards[i].name = strdup(_8BIT_to_UTF8(name));

    // Go backward and look for a keyboard with the same name.
    for (j = i - 1; j >= 0; --j)
    {
      if (!strcmp(keyboards[i].name, keyboards[j].name))
      {
        // Found => compute the virtual index.
        keyboards[i].virtualIndex = keyboards[j].virtualIndex + 1;
        break;
      }
    }
    if (j < 0)
    {
      // Not found => the virtual index is 0.
      keyboards[i].virtualIndex = 0;
    }
    i++;
  }

  initialized = 1;

  return 1;
}

/*
 * \brief Release unused stuff. It currently only releases unused joysticks.
 */
void GE_release_unused()
{
  int i;
  for (i = 0; i < GE_MAX_DEVICES && joysticks[i].name; ++i)
  {
    if (!joysticks[i].isUsed)
    {
      free(joysticks[i].name);
      joysticks[i].name = NULL;
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
  for (i = 0; i < GE_MAX_DEVICES && mice[i].name; ++i)
  {
    free(mice[i].name);
    mice[i].name = NULL;
  }
  for (i = 0; i < GE_MAX_DEVICES && keyboards[i].name; ++i)
  {
    free(keyboards[i].name);
    keyboards[i].name = NULL;
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
    if (joysticks[i].name)
    {
      free(joysticks[i].name);
      joysticks[i].name = NULL;
      ev_joystick_close(i);
    }
  }
  GE_FreeMKames();
  ev_quit();

  initialized = 0;
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
    return mice[id].name;
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
    return keyboards[id].name;
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
    return joysticks[id].name;
  }
  return NULL;
}

/*
 * \brief Get the joystick virtual id for a given index.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the joystick virtual id if present, NULL otherwise.
 */
int GE_JoystickVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return joysticks[id].virtualIndex;
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
    joysticks[id].isUsed = 1;
  }
}

/*
 * \brief Register a joystick to be emulated in software.
 * 
 * \remark This function has to be called before calling GE_initialize.
 *
 * \param name  the name of the joystick to register.
 * 
 * \return the id of the joystick, that can be used to forge a GE_Event to pass as argument to GE_PushEvent(),
 *         or -1 if the library was already initialized
 */
int GE_RegisterJoystick(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short))
{
  if(initialized)
  {
    fprintf(stderr, "GE_RegisterJoystick has to be called before GE_initialize.\n");
    return -1;
  }

  return ev_joystick_register(name, rumble_cb);
}

/*
 * \brief Get the mouse virtual id for a given index.
 * 
 * \param id  the mouse index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the mouse virtual id if present, NULL otherwise.
 */
int GE_MouseVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return mice[id].virtualIndex;
  }
  return 0;
}

/*
 * \brief Get the keyboard virtual id for a given index.
 * 
 * \param id  the keyboard index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return the keyboard virtual id if present, NULL otherwise.
 */
int GE_KeyboardVirtualId(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return keyboards[id].virtualIndex;
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
GE_JS_Type GE_GetJSType(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return joysticks[id].type;
  }
  return GE_JS_OTHER;
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
 * \param event  the event
 *
 * \return 0 in case of success, -1 in case of error.
 */
int GE_PushEvent(GE_Event *event)
{
  return queue_push_event(event);
}

/*
 * \brief Set a callback function for processing events.
 *
 * \param fp  the callback function
 */
void GE_SetCallback(int(*fp)(GE_Event*))
{
  ev_set_callback(fp);
}

/*
 * Add an event source.
 */
void GE_AddSource(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  ev_register_source(fd, id, fp_read, fp_write, fp_cleanup);
}

#ifdef WIN32
/*
 * Add an event source.
 */
void GE_AddSourceHandle(HANDLE handle, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  ev_register_source_handle(handle, id, fp_read, fp_write, fp_cleanup);
}
/*
 * Remove an event source.
 */
void GE_RemoveSourceHandle(HANDLE handle)
{
  ev_remove_source_handle(handle);
}
#endif

/*
 * Remove an event source.
 */
void GE_RemoveSource(int fd)
{
  ev_remove_source(fd);
}

/*
 * \brief Start a timer to make GE_PumpEvents return periodically.
 * 
 * \param period  the period of the timer (microseconds).
 */
void GE_TimerStart(int usec)
{
#ifndef WIN32
  int tfd = timer_start(usec);

  if(tfd >= 0)
  {
    ev_register_source(tfd, 0, &timer_read, NULL, &timer_close);
  }
#else
  timer_start(usec);
#endif
}

/*
 * \brief Stop the timer.
 */
void GE_TimerClose()
{
#ifndef WIN32
  int tfd = timer_get();

  if(tfd >= 0)
  {
    ev_remove_source(tfd);
  }
#endif
  timer_close(0);
}

/*
 * \brief Tell if a joystick has rumble capabilities.
 * 
 * \param id  the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * 
 * \return 1 if the joystick has rumble capabilities, false otherwise
 */
int GE_JoystickHasRumble(int id)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return ev_joystick_has_ff_rumble(id);
  }
  return 0;
}

/*
 * \brief Set the joystick rumble.
 *
 * \param id    the joystick index (in the [0..GE_MAX_DEVICES[ range)
 * \param weak  the weak motor magnitude
 * \param strong  the weak strong magnitude
 *
 * \return -1 in case of error, 0 otherwise
 */
int GE_JoystickSetRumble(int id, unsigned short weak, unsigned short strong)
{
  if (id >= 0 && id < GE_MAX_DEVICES)
  {
    return ev_joystick_set_ff_rumble(id, weak, strong);
  }
  return 0;
}

#ifndef WIN32
int GE_JoystickGetUHidId(int id)
{
  return ev_joystick_get_uhid_id(id);
}
#else
int GE_JoystickGetUsbIds(int id, unsigned short * vendor, unsigned short * product)
{
  return ev_joystick_get_usb_ids(id, vendor, product);
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
  return queue_peep_events(events, numevents);
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
