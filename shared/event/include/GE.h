/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GE_H_
#define GE_H_

#include <stdint.h>

#define MAX_KEYNAMES 249
#define MOUSE_BUTTONS_MAX 12

#ifdef WIN32
#include "GE_Windows.h"
#else
#include "GE_Linux.h"
#include <sys/time.h>
#endif

#define GE_GRAB_OFF 0
#define GE_GRAB_ON 1

typedef enum {
       GE_NOEVENT = 0,     /**< Unused (do not remove) */
       GE_KEYDOWN,     /**< Keys pressed */
       GE_KEYUP,     /**< Keys released */
       GE_MOUSEMOTION,     /**< Mouse moved */
       GE_MOUSEBUTTONDOWN,   /**< Mouse button pressed */
       GE_MOUSEBUTTONUP,   /**< Mouse button released */
       GE_JOYAXISMOTION,   /**< Joystick axis motion */
       GE_JOYHATMOTION,    /**< Joystick hat position change */
       GE_JOYBUTTONDOWN,   /**< Joystick button pressed */
       GE_JOYBUTTONUP,     /**< Joystick button released */
       GE_QUIT,     /**< Joystick button released */
} GE_EventType;

typedef struct GE_KeyboardEvent {
  uint8_t type; /**< GE_KEYDOWN or GE_KEYUP */
  uint8_t which;  /**< The keyboard device index */
  uint16_t keysym;
} GE_KeyboardEvent;

typedef struct GE_MouseMotionEvent {
  uint8_t type; /**< GE_MOUSEMOTION */
  uint8_t which;  /**< The mouse device index */
  int16_t xrel;  /**< The relative motion in the X direction */
  int16_t yrel;  /**< The relative motion in the Y direction */
} GE_MouseMotionEvent;

typedef struct GE_MouseButtonEvent {
  uint8_t type; /**< GE_MOUSEBUTTONDOWN or GE_MOUSEBUTTONUP */
  uint8_t which;  /**< The mouse device index */
  uint8_t button; /**< The mouse button index */
} GE_MouseButtonEvent;

typedef struct GE_JoyAxisEvent {
  uint8_t type; /**< GE_JOYAXISMOTION */
  uint8_t which;  /**< The joystick device index */
  uint8_t axis; /**< The joystick axis index */
  int16_t value; /**< The axis value (range: -32768 to 32767) */
} GE_JoyAxisEvent;

typedef struct GE_JoyHatEvent {
  uint8_t type; /**< GE_JOYHATMOTION */
  uint8_t which;  /**< The joystick device index */
  uint8_t hat;  /**< The joystick hat index */
  uint8_t value;  /**< The hat position value:
       *   GE_HAT_LEFTUP   GE_HAT_UP       GE_HAT_RIGHTUP
       *   GE_HAT_LEFT     GE_HAT_CENTERED GE_HAT_RIGHT
       *   GE_HAT_LEFTDOWN GE_HAT_DOWN     GE_HAT_RIGHTDOWN
       *  Note that zero means the POV is centered.
       */
} GE_JoyHatEvent;

typedef struct GE_JoyButtonEvent {
  uint8_t type; /**< GE_JOYBUTTONDOWN or GE_JOYBUTTONUP */
  uint8_t which;  /**< The joystick device index */
  uint8_t button; /**< The joystick button index */
  uint8_t state;  /**< GE_PRESSED or GE_RELEASED */
} GE_JoyButtonEvent;

typedef union GE_Event {
  uint8_t type;
  GE_KeyboardEvent key;
  GE_MouseMotionEvent motion;
  GE_MouseButtonEvent button;
  GE_JoyAxisEvent jaxis;
  GE_JoyHatEvent jhat;
  GE_JoyButtonEvent jbutton;
} GE_Event;

#define EVENT_BUFFER_SIZE 256

#define AXIS_X 0
#define AXIS_Y 1

#define MOUSE_AXIS_X "x"
#define MOUSE_AXIS_Y "y"

#ifdef __cplusplus
extern "C" {
#endif

extern int merge_all_devices;

int GE_initialize();
void GE_grab_toggle();
void GE_grab();
void GE_release_unused();
void GE_quit();
void GE_FreeMKames();

char* GE_MouseName(int);
char* GE_KeyboardName(int);
int GE_MouseVirtualId(int);
int GE_KeyboardVirtualId(int);
int GE_GetDeviceId(GE_Event*);
char* GE_JoystickName(int);
int GE_JoystickVirtualId(int);
void GE_SetJoystickUsed(int);
int GE_IsSixaxis(int);

const char* GE_MouseButtonName(int);
int GE_MouseButtonId(const char*);
const char* GE_KeyName(uint16_t);
uint16_t GE_KeyId(const char*);

#ifndef WIN32
void GE_SetCallback(int(*)(GE_Event*));
void GE_TimerStart(struct timespec*);
void GE_TimerClose();
#endif
void GE_PumpEvents();
int GE_PeepEvents(GE_Event*, int);
int GE_PushEvent(GE_Event*);

#ifdef __cplusplus
}
#endif

#endif /* GE_H_ */
