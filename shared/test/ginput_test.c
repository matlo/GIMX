/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include "common.h"

#define PERIOD 10000//microseconds

int mkb_select() {

  printf("Available mouse and keyboard input methods:\n");
  printf("%d none\n", GE_MKB_SOURCE_NONE);
  printf("%d physical\n", GE_MKB_SOURCE_PHYSICAL);
  printf("%d window system\n", GE_MKB_SOURCE_WINDOW_SYSTEM);

  printf("Select the input method: ");
  fflush(stdout);
  unsigned int choice = UINT_MAX;
  if (scanf("%d", &choice) == 1 && choice <= GE_MKB_SOURCE_WINDOW_SYSTEM) {
    return choice;
  }

  fprintf(stderr, "Invalid choice.\n");
  return -1;
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
  setup_handlers();

  int mkb_source = mkb_select();

  if (mkb_source < 0)
  {
    exit(-1);
  }

  GPOLL_INTERFACE poll_interface = {
          .fp_register = REGISTER_FUNCTION,
          .fp_remove = REMOVE_FUNCTION
  };
  if (ginput_init(&poll_interface, mkb_source, process_event) < 0)
  {
    exit(-1);
  }

  display_devices();

  GTIMER_CALLBACKS timer_callbacks = {
          .fp_read = timer_read,
          .fp_close = timer_close,
          .fp_register = REGISTER_FUNCTION,
          .fp_remove = REMOVE_FUNCTION,
  };
  int timer = gtimer_start(42, PERIOD, &timer_callbacks);
  if (timer < 0) {
    set_done();
  }
  
  ginput_grab();

  while(!is_done())
  {
    gpoll();

    ginput_periodic_task();

    //do something periodically
  }

  if (timer >= 0) {
    gtimer_close(timer);
  }

  ginput_quit();

  printf("Exiting\n");fflush(stdout);

  return 0;
}
