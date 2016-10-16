/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "common.h"

#include <stdio.h>
#include <limits.h>
#include <ghid.h>
#include <string.h>
#include <signal.h>

volatile int done = 0;

int is_done() {

    return done;
}

void set_done() {

    done = 1;
}

static void terminate(int sig __attribute__((unused))) {

    done = 1;
}

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwType) {

    switch (dwType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:

        done = 1; //signal the main thread to terminate

        //Returning would make the process exit!
        //We just make the handler sleep until the main thread exits,
        //or until the maximum execution time for this handler is reached.
        Sleep(10000);

        return TRUE;
    default:
        break;
    }
    return FALSE;
}
#endif

void setup_handlers() {

    (void) signal(SIGINT, terminate);
    (void) signal(SIGTERM, terminate);
#ifndef WIN32
    (void) signal(SIGHUP, terminate);
#else
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == 0)
    {
      fprintf(stderr, "SetConsoleCtrlHandler failed\n");
      exit(-1);
    }
  #endif
}

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

int timer_close(int user __attribute__((unused))) {
  done = 1;
  return 1;
}

int timer_read(int user __attribute__((unused))) {
  /*
   * Returning a non-zero value makes gpoll return, allowing to check the 'done' variable.
   */
  return 1;
}

char * hid_select() {

  char * path = NULL;

  struct ghid_device * hid_devs = ghid_enumerate(0x0000, 0x0000);
  if (hid_devs == NULL) {
    fprintf(stderr, "No HID device detected!\n");
    return NULL;
  }
  printf("Available HID devices:\n");
  unsigned int index = 0;
  struct ghid_device * current;
  for (current = hid_devs; current != NULL; current = current->next) {
    printf("%d VID 0x%04x PID 0x%04x PATH %s\n", index++, current->vendor_id, current->product_id, current->path);
  }

  printf("Select the HID device number: ");
  fflush(stdout);
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice < index) {
    current = hid_devs;
    while(choice > 0) {
        current = current->next;
        --choice;
    }
    path = strdup(current->path);
    if(path == NULL) {
      fprintf(stderr, "can't duplicate path.\n");
    }
  } else {
    fprintf(stderr, "Invalid choice.\n");
  }

  ghid_free_enumeration(hid_devs);

  return path;
}
