/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <queue.h>
#include "mkb.h"
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <timer.h>
#include <dirent.h>

#define eprintf(...) if(debug) printf(__VA_ARGS__)

static int debug = 0;

#define MAX_KEYNAMES (KEY_MICMUTE+1)

#define DEVTYPE_KEYBOARD 0x01
#define DEVTYPE_MOUSE    0x02
#define DEVTYPE_NB       2

static struct
{
  unsigned char type;
  int fd;
  int id[DEVTYPE_NB];
  char* name;
} devices[GE_MAX_DEVICES];

static int k_num;
static int m_num;
static int max_device_id;

static int grab = 0;

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

static inline int BitIsSet(const unsigned long *array, int bit)
{
    return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

static int mkb_read_type(int index, int fd)
{
  char name[1024]                             = {0};
  unsigned long key_bitmask[NLONGS(KEY_CNT)] = {0};
  unsigned long rel_bitmask[NLONGS(REL_CNT)] = {0};
  int i, len;
  int has_rel_axes = 0;
  int has_keys = 0;
  int has_scroll = 0;

  if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) < 0) {
    fprintf(stderr, "ioctl EVIOCGNAME failed: %s\n", strerror(errno));
    return -1;
  }

  len = ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel_bitmask)), rel_bitmask);
  if (len < 0) {
    fprintf(stderr, "ioctl EVIOCGBIT failed: %s\n", strerror(errno));
    return -1;
  }

  len = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
  if (len < 0) {
    fprintf(stderr, "ioctl EVIOCGBIT failed: %s\n", strerror(errno));
    return -1;
  }

  for (i = 0; i < REL_MAX; i++) {
      if (BitIsSet(rel_bitmask, i)) {
          has_rel_axes = 1;
          break;
      }
  }

  if (has_rel_axes) {
      if (BitIsSet(rel_bitmask, REL_WHEEL) ||
          BitIsSet(rel_bitmask, REL_HWHEEL) ||
          BitIsSet(rel_bitmask, REL_DIAL)) {
          has_scroll = 1;
      }
  }

  for (i = 0; i < BTN_MISC; i++) {
      if (BitIsSet(key_bitmask, i)) {
          has_keys = 1;
          break;
      }
  }

  if(!has_rel_axes && !has_keys && !has_scroll)
  {
    return -1;
  }

  if(has_keys)
  {
    devices[index].type = DEVTYPE_KEYBOARD;
    devices[index].id[0] = k_num;
    k_num++;
  }
  if(has_rel_axes || has_scroll)
  {
    devices[index].type |= DEVTYPE_MOUSE;
    devices[index].id[1] = m_num;
    m_num++;
  }

  devices[index].name = strdup(name);

  return 0;
}

static int (*event_callback)(GE_Event*) = NULL;

void mkb_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

static void mkb_process_event(int device, struct input_event* ie)
{
  GE_Event evt = {};

  switch(ie->type)
  {
    case EV_KEY:
      if(ie->value > 1)
      {
        return;
      }
      break;
    case EV_MSC:
      if(ie->value > 1)
      {
        return;
      }
      if(ie->value == 2)
      {
        return;
      }
      break;
    case EV_REL:
    case EV_ABS:
      break;
    default:
      return;
  }
  if((devices[device].type & DEVTYPE_KEYBOARD))
  {
    if(ie->type == EV_KEY)
    {
      if(ie->code > 0 && ie->code < MAX_KEYNAMES)
      {
        evt.type = ie->value ? GE_KEYDOWN : GE_KEYUP;
        evt.key.which = devices[device].id[0];
        evt.key.keysym = ie->code;
      }
    }
  }
  if(devices[device].type & DEVTYPE_MOUSE)
  {
    if(ie->type == EV_KEY)
    {
      if(ie->code >= BTN_LEFT && ie->code <= BTN_TASK)
      {
        evt.type = ie->value ? GE_MOUSEBUTTONDOWN : GE_MOUSEBUTTONUP;
        evt.button.which = devices[device].id[1];
        evt.button.button = ie->code - BTN_MOUSE;
      }
    }
    else if(ie->type == EV_REL)
    {
      if(ie->code == REL_X)
      {
        evt.type = GE_MOUSEMOTION;
        evt.motion.which = devices[device].id[1];
        evt.motion.xrel = ie->value;
      }
      else if(ie->code == REL_Y)
      {
        evt.type = GE_MOUSEMOTION;
        evt.motion.which = devices[device].id[1];
        evt.motion.yrel = ie->value;
      }
      else if(ie->code == REL_WHEEL)
      {
        evt.type = GE_MOUSEBUTTONDOWN;
        evt.button.which = devices[device].id[1];
        evt.button.button = (ie->value > 0) ? GE_BTN_WHEELUP : GE_BTN_WHEELDOWN;
      }
      else if(ie->code == REL_HWHEEL)
      {
        evt.type = GE_MOUSEBUTTONDOWN;
        evt.button.which = devices[device].id[1];
        evt.button.button = (ie->value > 0) ? GE_BTN_WHEELRIGHT : GE_BTN_WHEELLEFT;
      }
    }
  }

  /*
   * Process evt.
   */
  if(evt.type != GE_NOEVENT)
  {
    eprintf("event from device: %s\n", devices[device].name);
    eprintf("type: %d code: %d value: %d\n", ie->type, ie->code, ie->value);
    event_callback(&evt);
    if(evt.type == GE_MOUSEBUTTONDOWN)
    {
      if(ie->code == REL_WHEEL || ie->code == REL_HWHEEL)
      {
        evt.type = GE_MOUSEBUTTONUP;
        queue_push_event(&evt);
      }
    }
  }
}

static struct input_event ie[MAX_EVENTS];

static int mkb_process_events(int device)
{
  unsigned int size = sizeof(ie);
  unsigned int j;
  int r;

  int tfd = timer_get();

  if(tfd < 0)
  {
    //read a single event
    size = sizeof(*ie);
  }

  if((r = read(devices[device].fd, ie, size)) > 0)
  {
    for(j=0; j<r/sizeof(*ie); ++j)
    {
      mkb_process_event(device, ie+j);

      if(event_callback == GE_PushEvent)
      {
        return 1;
      }
    }

    if(r < 0 && errno != EAGAIN)
    {
      mkb_close_device(device);
    }
  }
  return 0;
}

#define DEV_INPUT "/dev/input"
#define EV_DEV_NAME "event%u"

static int is_event_file(const struct dirent *dir) {
  unsigned int num;
  if(dir->d_type == DT_CHR && sscanf(dir->d_name, EV_DEV_NAME, &num) == 1 && num < 256) {
    return 1;
  }
  return 0;
}

int mkb_init()
{
  int ret = 0;
  int i, j;
  int fd;
  char device[sizeof("/dev/input/event255")];

  memset(devices, 0x00, sizeof(devices));
  max_device_id = -1;
  k_num = 0;
  m_num = 0;

  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    devices[i].fd = -1;

    for(j=0; j<DEVTYPE_NB; ++j)
    {
      devices[i].id[j] = -1;
    }
  }

  struct dirent **namelist;
  int n;

  n = scandir(DEV_INPUT, &namelist, is_event_file, alphasort);
  if (n >= 0)
  {
    for(i=0; i<n && !ret; ++i)
    {
      snprintf(device, sizeof(device), "%s/%s", DEV_INPUT, namelist[i]->d_name);

      fd = open (device, O_RDONLY | O_NONBLOCK);
      if(fd != -1)
      {
        if(mkb_read_type(i, fd) != -1)
        {
          devices[i].fd = fd;
          if(grab)
          {
            ioctl(devices[i].fd, EVIOCGRAB, (void *)1);
          }
          max_device_id = i;
          ev_register_source(devices[i].fd, i, &mkb_process_events, NULL, &mkb_close_device);
        }
        else
        {
          close(fd);
        }
      }
      else if(errno == EACCES)
      {
        fprintf(stderr, "can't open %s: %s\n", device, strerror(errno));
        ret = -1;
      }

      free(namelist[i]);
    }
    free(namelist);
  }
  else
  {
    fprintf(stderr, "can't scan directory %s: %s\n", DEV_INPUT, strerror(errno));
    ret = -1;
  }

  return ret;
}

static char* mkb_get_name(unsigned char devtype, int index)
{
  int i;
  int nb = 0;
  for(i=0; i<=max_device_id; ++i)
  {
    if(devices[i].type & devtype)
    {
      if(index == nb)
      {
        return devices[i].name;
      }
      nb++;
    }
  }
  return NULL;
}

char* mkb_get_k_name(int index)
{
  return mkb_get_name(DEVTYPE_KEYBOARD, index);
}

char* mkb_get_m_name(int index)
{
  return mkb_get_name(DEVTYPE_MOUSE, index);
}

int mkb_close_device(int id)
{
  free(devices[id].name);
  devices[id].name = NULL;
  if(devices[id].fd >= 0)
  {
    ev_remove_source(devices[id].fd);
    close(devices[id].fd);
    devices[id].fd = -1;
  }
  return 0;
}

void mkb_quit()
{
  int i;
  for(i=0; i<=max_device_id; ++i)
  {
    if(grab)
    {
      ioctl(devices[i].fd, EVIOCGRAB, (void *)0);
    }
    mkb_close_device(i);
  }

  if(!grab)
  {
    tcflush(STDIN_FILENO, TCIFLUSH);
  }
}

void mkb_grab(int mode)
{
  int i;
  int one = 1;
  int* enable = NULL;
  if(mode == GE_GRAB_ON)
  {
    enable = &one;
  }
  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    if(devices[i].fd > -1)
    {
      ioctl(devices[i].fd, EVIOCGRAB, enable);
    }
  }
}

