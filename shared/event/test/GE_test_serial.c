/*
 * linux_test.c
 *
 *  Created on: 13 janv. 2013
 *      Author: matlo
 */

#include <ginput.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <serialasync.h>
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

static int serial = -1;

static unsigned char packet[64] = {};
static unsigned char result[sizeof(packet)] = {};
static unsigned char read = 0;

int serial_read(int user, const void * buf, unsigned int count)
{
  memcpy(result + read, buf, count);
  read += count;

  serialasync_set_read_size(serial, sizeof(packet) - read);

  if(read < sizeof(packet))
  {
    return 0;
  }

  printf("user: %d\n", user);
  dump((unsigned char *)result, sizeof(packet));

  if(memcmp(result, packet, sizeof(packet)))
  {
    fprintf(stderr, "bad packet content\n");
    done = 1;
  }
  else
  {
    unsigned int i;
    for(i = 0; i < sizeof(packet); ++i)
    {
      packet[i]++;
    }
    serialasync_write(serial, packet, sizeof(packet));

    read = 0;
    serialasync_set_read_size(serial, sizeof(packet));
  }

  return 0;
}

int serial_close(int user)
{
  printf("close user: %d\n", user);
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

  serial = serialasync_open("/dev/ttyUSB0", 500000);

  if(serial >= 0)
  {
    serialasync_register(serial, 42, serial_read, NULL, serial_close, REGISTER_FUNCTION);
    
    serialasync_set_read_size(serial, sizeof(packet));

    GE_SetCallback(process_event);

    GE_TimerStart(PERIOD);

    serialasync_write(serial, packet, sizeof(packet));

    while(!done)
    {
      GE_PumpEvents();

      //do something periodically
    }

    GE_TimerClose();
    
    serialasync_close(serial);
  }
  else
  {
    fprintf(stderr, "error opening serial device\n");
  }

  GE_quit();

  printf("Exiting\n");

  return 0;
}
