/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <gimxinput/include/ginput.h>

void display_devices()
{
  int i;
  const char * name;

  i = 0;
  printf("Keyboards:\n");
  while((name = ginput_keyboard_name(i)))
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
  while((name = ginput_mouse_name(i)))
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
  while((name = ginput_joystick_name(i)))
  {
    printf("  joystick %d is named %s\n", i, name);
    ++i;
  }
  
  if(!i)
  {
    printf("  none\n");
  }
  
  fflush(stdout);
}

int process_event(GE_Event* event)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      printf("Keyboard: %s (%u) - key down: %s\n", ginput_keyboard_name(event->key.which), ginput_keyboard_virtual_id(event->key.which), ginput_key_name(event->key.keysym));
      if(event->key.keysym == GE_KEY_ESC)
      {
        done = 1;
      }
      break;
    case GE_KEYUP:
      printf("Keyboard: %s (%u) - key up: %s\n", ginput_keyboard_name(event->key.which), ginput_keyboard_virtual_id(event->key.which), ginput_key_name(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      printf("Mouse: %s (%u) - button down: %s\n", ginput_mouse_name(event->button.which), ginput_mouse_virtual_id(event->button.which), ginput_mouse_button_name(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      printf("Mouse: %s (%u) - button up: %s\n", ginput_mouse_name(event->button.which), ginput_mouse_virtual_id(event->button.which), ginput_mouse_button_name(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        printf("Mouse: %s (%u) - axis x: %d\n", ginput_mouse_name(event->motion.which), ginput_mouse_virtual_id(event->motion.which), event->motion.xrel);
      }
      if(event->motion.yrel)
      {
        printf("Mouse: %s (%u) - axis y: %d\n", ginput_mouse_name(event->motion.which), ginput_mouse_virtual_id(event->motion.which), event->motion.yrel);
      }
      break;
    case GE_JOYBUTTONDOWN:
      printf("Joystick: %s (%u) - button down: %d\n", ginput_joystick_name(event->jbutton.which), ginput_joystick_virtual_id(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      printf("Joystick: %s (%u) - button up: %d\n", ginput_joystick_name(event->jbutton.which), ginput_joystick_virtual_id(event->jbutton.which), event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      printf("Joystick: %s (%u) - axis: %d value: %d\n", ginput_joystick_name(event->jaxis.which), ginput_joystick_virtual_id(event->jaxis.which), event->jaxis.axis, event->jaxis.value);
      break;
  }
  fflush(stdout);
  return 0;
}

int ignore_event(GE_Event* event __attribute__((unused))) {
  return 0;
}
