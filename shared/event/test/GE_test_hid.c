/*
 * linux_test.c
 *
 *  Created on: 13 janv. 2013
 *      Author: matlo
 */

#include <GE.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <hidasync.h>
#include "common.h"

#define PERIOD 10000//microseconds

#ifdef WIN32
#define REGISTER_FUNCTION GE_AddSourceHandle
#else
#define REGISTER_FUNCTION GE_AddSource
#endif

static void terminate(int sig)
{
  done = 1;
}

static void dump(const unsigned char * packet, unsigned char length)
{
  int i;
  for(i=0; i<length; ++i)
  {
    if(i && !(i%8))
    {
      printf("\n");
    }
    printf("0x%02x ", packet[i]);
  }
  printf("\n");
}

int hid_read(int user, const void * buf, unsigned int count)
{
  printf("read user: %d\n", user);
  dump((unsigned char *)buf, count);
  return 0;
}

int hid_write(int user)
{
  printf("write user: %d\n", user);
  return 0;
}

int hid_close(int user)
{
  printf("close user: %d\n", user);
  done = 1;
  return 0;
}

int main(int argc, char* argv[])
{
  if (!GE_initialize(GE_MKB_SOURCE_NONE))
  {
    fprintf(stderr, "GE_initialize failed\n");
    exit(-1);
  }

  (void) signal(SIGINT, terminate);

#ifndef WIN32
  setlinebuf(stdout);
#endif

  //Open Logitech RumblePad 2
  int hid = hidasync_open_ids(0x046d, 0xc218);

  if(hid >= 0)
  {
    if(hidasync_register(hid, 42, hid_read, hid_write, hid_close, REGISTER_FUNCTION) != -1)
    {
      GE_SetCallback(process_event);

      GE_TimerStart(PERIOD);

      while(!done)
      {
        GE_PumpEvents();

        //do something periodically

        static int i = 1;
        if(i == 1) {
          //Download and Play Force               weak        strong
          unsigned char ff[] = {0x00, 0x51, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x00};
          hidasync_write(hid, ff, sizeof(ff));
        }
        else if(i == 100) {
          //Stop Force
          unsigned char stop[] = {0x00, 0xf3};
          hidasync_write(hid, stop, sizeof(stop));
        }
        else if(i == 200) {
          i = 0;
        }
        ++i;
      }

      GE_TimerClose();

      //Stop Force
      unsigned char stop[] = {0x00, 0xf3};
      hidasync_write_timeout(hid, stop, sizeof(stop), 1);
    }
    
    hidasync_close(hid);
  }
  else
  {
    fprintf(stderr, "hid device not found\n");
  }

  GE_quit();

  printf("Exiting\n");

  return 0;
}
