/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include "common.h"

#define PERIOD 10000//microseconds

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#endif

static void terminate(int sig)
{
  done = 1;
  fprintf(stderr, "%s\n", __func__);
}

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

int main(int argc, char* argv[])
{
  int mkb_source = mkb_select();

  if (mkb_source < 0)
  {
    exit(-1);
  }

  if (ginput_init(mkb_source, process_event) < 0)
  {
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

  display_devices();

  int timer = gtimer_start(42, PERIOD, timer_read, timer_close, REGISTER_FUNCTION);
  if (timer < 0) {
    done = 1;
  }
  
  ginput_grab();

  while(!done)
  {
    gpoll();

    ginput_sync_process();

    //do something periodically
  }

  if (timer >= 0) {
    gtimer_close(timer);
  }

  ginput_quit();

  printf("Exiting\n");fflush(stdout);

  return 0;
}
