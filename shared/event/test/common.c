/*
 * common.c
 *
 *  Created on: 14 janv. 2013
 *      Author: matlo
 */

#include <stdio.h>
#include "common.h"

int done = 0;

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

  i = 0;
  printf("Mice:\n");
  while((name = GE_MouseName(i)))
  {
    printf("  mouse %d is named %s\n", i, name);
    ++i;
  }

  i = 0;
  printf("Joysticks:\n");
  while((name = GE_JoystickName(i)))
  {
    printf("  joystick %d is named %s\n", i, name);
    ++i;
  }
}

int process_event(GE_Event* event)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      printf("Keyboard: %s - key down: %s\n", GE_KeyboardName(event->key.which), GE_KeyName(event->key.keysym));
      if(event->key.keysym == GE_KEY_ESC)
      {
        done = 1;
      }
      break;
    case GE_KEYUP:
      printf("Keyboard: %s - key up: %s\n", GE_KeyboardName(event->key.which), GE_KeyName(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      printf("Mouse: %s - button down: %s\n", GE_MouseName(event->button.which), GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      printf("Mouse: %s - button up: %s\n", GE_MouseName(event->button.which), GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        printf("Mouse: %s - axis x: %d\n", GE_MouseName(event->motion.which), event->motion.xrel);
      }
      if(event->motion.yrel)
      {
        printf("Mouse: %s - axis y: %d\n", GE_MouseName(event->motion.which), event->motion.yrel);
      }
      break;
    case GE_JOYBUTTONDOWN:
      printf("Joystick: %s - button down: %d\n", GE_JoystickName(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      printf("Joystick: %s - button up: %d\n", GE_JoystickName(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      printf("Joystick: %s - axis: %d value: %d\n", GE_JoystickName(event->jaxis.which), event->jaxis.axis, event->jaxis.value);
      break;
  }
  return 0;
}
