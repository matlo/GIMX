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
#include "xinput.h"
#include <queue.h>

static struct
{
  int id;
  int (*fp_read)(int);
  int (*fp_write)(int);
  int (*fp_cleanup)(int);
  short int event;
} sources[FD_SETSIZE] = {};

static int max_source = 0;

static unsigned char mkb_source;

void ev_register_source(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fp_cleanup)(int))
{
  if(!fp_cleanup)
  {
    fprintf(stderr, "%s: the cleanup function is mandatory.", __FUNCTION__);
    return;
  }
  if(fd < FD_SETSIZE)
  {
    sources[fd].id = id;
    if(fp_read)
    {
      sources[fd].event |= POLLIN;
      sources[fd].fp_read = fp_read;
    }
    if(fp_write)
    {
      sources[fd].event |= POLLOUT;
      sources[fd].fp_write = fp_write;
    }
    sources[fd].fp_cleanup = fp_cleanup;
    if(fd > max_source)
    {
      max_source = fd;
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

int ev_init(unsigned char mkb_src)
{
  int ret;

  mkb_source = mkb_src;

  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    ret = mkb_init();

    if(ret < 0)
    {
      fprintf(stderr, "mkb_init failed.\n");
      return 0;
    }
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    ret = xinput_init();

    if(ret < 0)
    {
      fprintf(stderr, "xinput_init failed.\n");
      return 0;
    }
  }

  ret = js_init();

  if(ret < 0)
  {
    fprintf(stderr, "jsdev_init failed.\n");
    return 0;
  }

  queue_init();

  return 1;
}

int ev_joystick_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short))
{
  return js_register(name, rumble_cb);
}

void ev_joystick_close(int id)
{
  js_close(id);
}

void ev_grab_input(int mode)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    mkb_grab(mode);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    xinput_grab(mode);
  }
}

void ev_quit(void)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    mkb_quit();
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    xinput_quit();
  }
  js_quit();
}

const char* ev_joystick_name(int index)
{
  return js_get_name(index);
}

const char* ev_mouse_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return mkb_get_m_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    return xinput_get_mouse_name(id);
  }
  return NULL;
}

const char* ev_keyboard_name(int id)
{
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    return mkb_get_k_name(id);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    return xinput_get_keyboard_name(id);
  }
  return NULL;
}

int ev_joystick_has_ff_rumble(int joystick)
{
  return js_has_ff_rumble(joystick);
}

int ev_joystick_set_ff_rumble(int joystick, unsigned short weak, unsigned short strong)
{
  return js_set_ff_rumble(joystick, weak, strong);
}

int ev_joystick_get_uhid_id(int joystick)
{
  return js_get_uhid_id(joystick);
}

static int (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
  if(mkb_source == GE_MKB_SOURCE_PHYSICAL)
  {
    mkb_set_callback(fp);
  }
  else if(mkb_source == GE_MKB_SOURCE_WINDOW_SYSTEM)
  {
    xinput_set_callback(fp);
  }
  js_set_callback(fp);
}

static unsigned int fill_fds(nfds_t nfds, struct pollfd fds[nfds])
{
  unsigned int pos = 0;

  unsigned int i;
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
  unsigned int i;
  int res;

  if(event_callback == NULL)
  {
    fprintf(stderr, "ev_set_callback should be called first!\n");
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
          res = sources[fds[i].fd].fp_cleanup(sources[fds[i].fd].id);
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
