/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "common.h"
#include <stdio.h>
#include <limits.h>
#include <hidasync.h>
#include <string.h>

volatile int done = 0;

void display_devices()
{
  int i;
  char* name;

  i = 0;
  printf("Keyboards:\n");
  while((name = GE_KeyboardName(i)))
  {
    printf("  keyboard %d is named %s\n", i, name);
    ++i;
  }
  
  if(!i)
  {
    printf("  none\n");
  }

  i = 0;
  printf("Mice:\n");
  while((name = GE_MouseName(i)))
  {
    printf("  mouse %d is named %s\n", i, name);
    ++i;
  }
  
  if(!i)
  {
    printf("  none\n");
  }

  i = 0;
  printf("Joysticks:\n");
  while((name = GE_JoystickName(i)))
  {
    printf("  joystick %d is named %s\n", i, name);
    ++i;
  }
  
  if(!i)
  {
    printf("  none\n");
  }
  
#ifdef WIN32
  fflush(stdout);
#endif
}

int process_event(GE_Event* event)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      printf("Keyboard: %s (%u) - key down: %s\n", GE_KeyboardName(event->key.which), GE_KeyboardVirtualId(event->key.which), GE_KeyName(event->key.keysym));
      if(event->key.keysym == GE_KEY_ESC)
      {
        done = 1;
      }
      break;
    case GE_KEYUP:
      printf("Keyboard: %s (%u) - key up: %s\n", GE_KeyboardName(event->key.which), GE_KeyboardVirtualId(event->key.which), GE_KeyName(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      printf("Mouse: %s (%u) - button down: %s\n", GE_MouseName(event->button.which), GE_MouseVirtualId(event->button.which), GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      printf("Mouse: %s (%u) - button up: %s\n", GE_MouseName(event->button.which), GE_MouseVirtualId(event->button.which), GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        printf("Mouse: %s (%u) - axis x: %d\n", GE_MouseName(event->motion.which), GE_MouseVirtualId(event->motion.which), event->motion.xrel);
      }
      if(event->motion.yrel)
      {
        printf("Mouse: %s (%u) - axis y: %d\n", GE_MouseName(event->motion.which), GE_MouseVirtualId(event->motion.which), event->motion.yrel);
      }
      break;
    case GE_JOYBUTTONDOWN:
      printf("Joystick: %s (%u) - button down: %d\n", GE_JoystickName(event->jbutton.which), GE_JoystickVirtualId(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      printf("Joystick: %s (%u) - button up: %d\n", GE_JoystickName(event->jbutton.which), GE_JoystickVirtualId(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      printf("Joystick: %s (%u) - axis: %d value: %d\n", GE_JoystickName(event->jaxis.which), GE_JoystickVirtualId(event->jaxis.which), event->jaxis.axis, event->jaxis.value);
      break;
  }
#ifdef WIN32
  fflush(stdout);
#endif
  return 0;
}

int timer_close(int user) {
  done = 1;
  return 1;
}

int timer_read(int user) {
  /*
   * Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
   */
  return 1;
}

char * hid_select() {

  char * path = NULL;

  s_hid_dev * hid_devs = hidasync_enumerate(0x0000, 0x0000);
  if (hid_devs == NULL) {
    fprintf(stderr, "No HID device detected!\n");
    return NULL;
  }
  printf("Available HID devices:\n");
  unsigned int index = 0;
  s_hid_dev * current;
  for (current = hid_devs; current != NULL; ++current) {
    printf("%d VID 0x%04x PID 0x%04x PATH %s\n", index++, current->vendor_id, current->product_id, current->path);
    if (current->next == 0) {
      break;
    }
  }

  printf("Select the HID device number: ");
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice < index) {
    path = strdup(hid_devs[choice].path);
    if(path == NULL) {
      fprintf(stderr, "can't duplicate path.\n");
    }
  } else {
    fprintf(stderr, "Invalid choice.\n");
  }

  hidasync_free_enumeration(hid_devs);

  return path;
}
