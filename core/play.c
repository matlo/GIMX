/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <ginput.h>
#include <gpoll.h>
#include <gtimer.h>
#include <gusb.h>
#include <unistd.h>
#include "gimx.h"
#include <stdio.h>
#include <adapter.h>
#include <connectors/usb_con.h>
#include <report2event/report2event.h>
#include "connectors/sixaxis.h"

void play()
{
  int wait_sec = 5;

  /*
   * gpoll should always be executed as it drives the period.
   */
  gpoll();
  gpoll(); // second call to init the second adapter

  gprintf("waiting %ds for connection\n", wait_sec);
  sleep(wait_sec);
  gprintf("------------- play!\n"); 
  gimx_start_timer();

  if(sixaxis_play_events() <= 0)
  {
    fprintf(stderr, "error playing events\n");
  }
}
