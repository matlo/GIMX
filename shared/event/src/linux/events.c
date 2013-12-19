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
  void (*fd_read)(int);
  void (*fd_cleanup)(int);
} s_source;

static s_source sources[FD_SETSIZE] = {};

void ev_register_source(int fd, int id, void (*fd_read)(int), void (*fd_cleanup)(int))
{
  if(fd < FD_SETSIZE)
  {
    sources[fd].id = id;
    sources[fd].fd = fd;
    sources[fd].fd_read = fd_read;
    sources[fd].fd_cleanup = fd_cleanup;
  }
}

void ev_remove_source(int fd)
{
  if(fd < FD_SETSIZE)
  {
    sources[fd].id = 0;
    sources[fd].fd = 0;
    sources[fd].fd_read = NULL;
    sources[fd].fd_cleanup = NULL;
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

static int (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
  mkb_set_callback(fp);
  js_set_callback(fp);
}

static int fill_fds(nfds_t nfds, struct pollfd fds[nfds])
{
  int pos = 0;

  int tfd = timer_getfd();

  /*
   * Add tfd at index 0!
   */
  if(tfd >= 0)
  {
    fds[pos].fd = tfd;
    fds[pos].events = POLLIN;
    ++pos;
  }

  pos += mkb_fill_fds(nfds-pos, fds+pos);

  pos += js_fill_fds(nfds-pos, fds+pos);

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

  nfds_t nfds = mkb_get_nfds() + js_get_nfds();

  if(event_callback == NULL)
  {
    fprintf(stderr, "ev_set_callback should be called first!\n");
    return;
  }

  int tfd = timer_getfd();
  
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

  if(tfd >= 0)
  {
    ++nfds;
  }

  struct pollfd fds[nfds];

  while(1)
  {
    nfds = fill_fds(nfds, fds);

    if(poll(fds, nfds, -1) > 0)
    {
      for(i=0; i<nfds; ++i)
      {
        if(fds[i].revents & POLLERR)
        {
          sources[fds[i].fd].fd_cleanup(sources[fds[i].fd].id);
        }
        else if(fds[i].revents & POLLIN)
        {
          sources[fds[i].fd].fd_read(sources[fds[i].fd].id);

          if(tfd < 0)
          {
            return;
          }
        }
        if(i == 0 && timer_getstatus())
        {
          return;
        }
      }
    }
  }
}
