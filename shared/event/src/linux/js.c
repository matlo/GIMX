/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#include <GE.h>
#include <events.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/joystick.h>
#include <poll.h>
#include "js.h"
#include <timer.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define eprintf(...) if(debug) printf(__VA_ARGS__)

static int debug = 0;

#define AXMAP_SIZE (ABS_MAX + 1)

static int joystick_fd[GE_MAX_DEVICES];
static int joystick_id[GE_MAX_DEVICES];
static char* joystick_name[GE_MAX_DEVICES];
static unsigned short joystick_button_nb[GE_MAX_DEVICES] = {0};
static int joystick_hat_value[GE_MAX_DEVICES][ABS_HAT3Y-ABS_HAT0X] = {{0}};
static uint8_t joystick_ax_map[GE_MAX_DEVICES][AXMAP_SIZE] = {{0}};
static int j_num;
static int max_joystick_id;

static int (*event_callback)(GE_Event*) = NULL;

void js_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

static void js_process_event(int device, struct js_event* je)
{
  GE_Event evt = {};

  if(je->type & JS_EVENT_INIT)
  {
    return;
  }

  if(je->type & JS_EVENT_BUTTON)
  {
    evt.type = je->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
    evt.jbutton.which = joystick_id[device];
    evt.jbutton.button = je->number;
  }
  else if(je->type & JS_EVENT_AXIS)
  {
    int axis = joystick_ax_map[device][je->number];
    if(axis >= ABS_HAT0X && axis <= ABS_HAT3Y)
    {
      evt.type = je->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
      int button;
      int value;
      axis -= ABS_HAT0X;
      if(!je->value)
      {
        value = joystick_hat_value[device][axis];
        joystick_hat_value[device][axis] = 0;
      }
      else
      {
        value = je->value/32767;
        joystick_hat_value[device][axis] = value;
      }
      button = axis + value + 2*(axis/2);
      if(button < 4*(axis/2))
      {
        button += 4;
      }
      evt.jbutton.which = joystick_id[device];
      evt.jbutton.button = button + joystick_button_nb[device];
    }
    else
    {
      evt.type = GE_JOYAXISMOTION;
      evt.jaxis.which = joystick_id[device];
      evt.jaxis.axis = je->number;
      evt.jaxis.value = je->value;
      /*
       * Ugly patch for the sixaxis.
       */
      if(GE_IsSixaxis(evt.jaxis.which) && evt.jaxis.axis > 3 && evt.jaxis.axis < 23)
      {
        evt.jaxis.value = (evt.jaxis.value + 32767) / 2;
      }
    }
  }

  /*
   * Process evt.
   */
  if(evt.type != GE_NOEVENT)
  {
    eprintf("event from joystick: %s\n", joystick_name[device]);
    eprintf("type: %d number: %d value: %d\n", je->type, je->number, je->value);
    event_callback(&evt);
  }
}

static struct js_event je[MAX_EVENTS];

static int js_process_events(int joystick)
{
  unsigned int size = sizeof(je);
  int j;
  int r;

  int tfd = timer_getfd();

  if(tfd < 0)
  {
    size = sizeof(*je);
  }

  if((r = read(joystick_fd[joystick], je, size)) > 0)
  {
    for(j=0; j<r/sizeof(*je); ++j)
    {
      js_process_event(joystick, je+j);

      if(event_callback == GE_PushEvent)
      {
        return 1;
      }
    }

    if(r < 0 && errno != EAGAIN)
    {
      js_close(joystick);
    }
  }
  
  return 0;
}

#define DEV_INPUT "/dev/input"
#define JS_DEV_NAME "js"

static int is_event_device(const struct dirent *dir) {
  return strncmp(JS_DEV_NAME, dir->d_name, sizeof(JS_DEV_NAME)-1) == 0;
}

int js_init()
{
  int ret = 0;
  int i;
  int fd;
  char joystick[sizeof("/dev/input/js255")];
  char name[1024] = {0};

  j_num = 0;
  max_joystick_id = -1;
  memset(joystick_name, 0x00, sizeof(joystick_name));
  memset(joystick_button_nb, 0x00, sizeof(joystick_button_nb));
  memset(joystick_hat_value, 0x00, sizeof(joystick_hat_value));
  memset(joystick_ax_map, 0x00, sizeof(joystick_ax_map));

  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    joystick_fd[i] = -1;
    joystick_id[i] = -1;
  }

  struct dirent **namelist;
  int n;

  n = scandir(DEV_INPUT, &namelist, is_event_device, alphasort);
  if (n >= 0)
  {
    for(i=0; i<n && !ret; ++i)
    {
      snprintf(joystick, sizeof(joystick), "%s/%s", DEV_INPUT, namelist[i]->d_name);

      fd = open (joystick, O_RDONLY | O_NONBLOCK);
      if(fd != -1)
      {
        if (ioctl(fd, JSIOCGNAME(sizeof(name) - 1), name) < 0) {
          fprintf(stderr, "ioctl EVIOCGNAME failed: %s\n", strerror(errno));
          close(fd);
          continue;
        }
        //printf("%s\n", name);
        unsigned char buttons;
        if (ioctl (fd, JSIOCGBUTTONS, &buttons) >= 0 && ioctl (fd, JSIOCGAXMAP, &joystick_ax_map[i]) >= 0)
        {
          joystick_name[i] = strdup(name);
          joystick_fd[i] = fd;
          joystick_button_nb[i] = buttons;
          max_joystick_id = i;
          joystick_id[i] = j_num;
          j_num++;
          ev_register_source(joystick_fd[i], POLLIN, i, &js_process_events, &js_close);
        }
        else
        {
          close(fd);
        }
      }
      else if(errno == EACCES)
      {
        fprintf(stderr, "can't open %s: %s\n", joystick, strerror(errno));
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

int js_close(int id)
{
  free(joystick_name[id]);
  joystick_name[id] = NULL;

  if(joystick_fd[id] >= 0)
  {
    close(joystick_fd[id]);
    joystick_fd[id] = -1;
  }
  return 0;
}

void js_quit()
{
  int i;
  for(i=0; i<=max_joystick_id; ++i)
  {
    js_close(i);
  }
}

const char* js_get_name(int index)
{
  int i;
  for(i=0; i<=max_joystick_id; ++i)
  {
    if(joystick_id[i] == index)
    {
      return joystick_name[i];
    }
  }
  return NULL;
}


