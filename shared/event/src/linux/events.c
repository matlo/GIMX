/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include "events.h"
#include <timer.h>
#include <sys/signalfd.h>
#include <math.h>
#include <GE.h>
#include <string.h>
#include "mkb.h"
#include "js.h"

#define eprintf(...) if(debug) printf(__VA_ARGS__)

static int debug = 0;

static GE_Event evqueue[MAX_EVENTS];
static unsigned char evqueue_index_first = 0;
static unsigned char evqueue_index_last = 0;

static GE_Event next_event = {};

inline void ev_set_next_event(GE_Event* event)
{
  memcpy(&next_event, &event, sizeof(next_event));
}

typedef struct
{
  int id;
  int fd;
  int (*fp_read)(int);
  int (*fp_write)(int);
  int (*fd_cleanup)(int);
  short int event;
} s_source;

static s_source sources[FD_SETSIZE] = {};
static int max_source = 0;

void ev_register_source(SOURCE source, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  if(source < FD_SETSIZE)
  {
    sources[source].id = id;
    sources[source].fd = source;
    if(fp_read)
    {
      sources[source].event |= POLLIN;
      sources[source].fp_read = fp_read;
    }
    if(fp_write)
    {
      sources[source].event |= POLLOUT;
      sources[source].fp_write = fp_write;
    }
    sources[source].fd_cleanup = fp_cleanup;
    if(source > max_source)
    {
      max_source = source;
    }
  }
}

void ev_remove_source(int fd)
{
  if(fd < FD_SETSIZE)
  {
    memset(sources+fd, 0x00, sizeof(*sources));
  }
}

int ev_push_event(GE_Event* ev)
{
  int ret = -1;
  eprintf("first: %d last: %d\n", evqueue_index_first, evqueue_index_last);
  if(evqueue_index_last >= evqueue_index_first)
  {
    evqueue[evqueue_index_last] = *ev;
    evqueue_index_last++;
    evqueue_index_last%=MAX_EVENTS;
    if(evqueue_index_last == evqueue_index_first)
    {
      evqueue_index_last = MAX_EVENTS-1;
    }
    ret = 0;
  }
  else if(evqueue_index_last+1 < evqueue_index_first)
  {
    evqueue[evqueue_index_last] = *ev;
    evqueue_index_last++;
    ret = 0;
  }
  eprintf("ret: %d first: %d last: %d\n", ret, evqueue_index_first, evqueue_index_last);
  return ret;
}

int ev_peep_events(GE_Event *events, int numevents)
{
  int i;
  int j = 0;
  if(evqueue_index_first > evqueue_index_last)
  {
    for(i=evqueue_index_first; i<MAX_EVENTS; ++i)
    {
      eprintf("peep: %d\n", i);
      events[j] = evqueue[i];
      ++evqueue_index_first;
      evqueue_index_first%=MAX_EVENTS;
      ++j;
      if(j == numevents)
      {
        return numevents;
      }
    }
  }
  for(i=evqueue_index_first; i<evqueue_index_last; ++i)
  {
    eprintf("peep: %d\n", i);
    events[j] = evqueue[i];
    ++evqueue_index_first;
    evqueue_index_first%=MAX_EVENTS;
    ++j;
    if(j == numevents)
    {
      return numevents;
    }
  }
  return j;
}

int ev_init()
{
  int i;
  for(i=0; i<FD_SETSIZE; ++i)
  {
    sources[i].fd = -1;
  }
  
  int ret = mkb_init();

  if(ret < 0)
  {
    fprintf(stderr, "evdev_init failed.\n");
    return 0;
  }

  ret = js_init();

  if(ret < 0)
  {
    fprintf(stderr, "jsdev_init failed.\n");
    return 0;
  }

  evqueue_index_first = 0;
  evqueue_index_last = 0;

  return 1;
}

void ev_joystick_close(int id)
{
  js_close(id);
}

void ev_grab_input(int mode)
{
  mkb_grab(mode);
}

void ev_quit(void)
{
  mkb_quit();
  js_quit();
}

const char* ev_joystick_name(int index)
{
  return js_get_name(index);
}

const char* ev_mouse_name(int id)
{
  return mkb_get_m_name(id);
}

const char* ev_keyboard_name(int id)
{
  return mkb_get_k_name(id);
}

#ifndef WIN32
int ev_joystick_has_ff_rumble(int joystick)
{
  return js_has_ff_rumble(joystick);
}

int ev_joystick_set_ff_rumble(int joystick, unsigned short weak_timeout, unsigned short weak, unsigned short strong_timeout, unsigned short strong)
{
  return js_set_ff_rumble(joystick, weak_timeout, weak, strong_timeout, strong);
}
#endif

static int (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
  mkb_set_callback(fp);
  js_set_callback(fp);
}

static unsigned int fill_fds(nfds_t nfds, struct pollfd fds[nfds])
{
  unsigned int pos = 0;

  int i;
  for(i=0; i<nfds; ++i)
  {
    if(sources[i].event)
    {
      fds[pos].fd = i;
      fds[pos].events = sources[i].event;
      ++pos;
    }
  }

  return pos;
}

/*
 * If a timer wasn't set (tfd < 0):
 * - this function blocks until an event is received
 * - it only reads a single event and returns
 * If a timer was set (tfd > 0):
 * - this function processes each received event
 * - it returns as soon as the timer expires
 */
void ev_pump_events(void)
{
  int i;
  int res;

  if(event_callback == NULL)
  {
    fprintf(stderr, "ev_set_callback should be called first!\n");
    return;
  }

  int tfd = timer_get();
  
  /*
   * If tfd is not set, ev_pump_event only reads a single event for each call.
   * This generates up button events for mouse wheels.
   */
  if(tfd < 0 && next_event.type != GE_NOEVENT)
  {
    event_callback(&next_event);
    next_event.type = GE_NOEVENT;
    return;
  }

  while(1)
  {
    struct pollfd fds[max_source+1];

    nfds_t nfds = fill_fds(max_source+1, fds);

    if(poll(fds, nfds, -1) > 0)
    {
      for(i=0; i<nfds; ++i)
      {
        if(fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
        {
          res = sources[fds[i].fd].fd_cleanup(sources[fds[i].fd].id);
          ev_remove_source(fds[i].fd);
          if(res)
          {
            return;
          }
          continue;
        }
        if(fds[i].revents & POLLIN)
        {
          if(sources[fds[i].fd].fp_read(sources[fds[i].fd].id))
		  		{
            return;
          }
        }
        if(fds[i].revents & POLLOUT)
        {
          if(sources[fds[i].fd].fp_write(sources[fds[i].fd].id))
          {
            return;
          }
        }
      }
    }
  }

}
