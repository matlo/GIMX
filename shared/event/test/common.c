/*
 * common.c
 *
 *  Created on: 14 janv. 2013
 *      Author: matlo
 */

#include <stdio.h>
#include "common.h"

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
