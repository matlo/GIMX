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

static struct
{
  int fd;
  int id;
  char* name;
  unsigned short button_nb;
  int hat_value[ABS_HAT3Y-ABS_HAT0X];
  uint8_t ax_map[AXMAP_SIZE];
  int ff_fd;
  int weak_id;
  int strong_id;
} joystick[GE_MAX_DEVICES] = {};

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
    evt.jbutton.which = joystick[device].id;
    evt.jbutton.button = je->number;
  }
  else if(je->type & JS_EVENT_AXIS)
  {
    int axis = joystick[device].ax_map[je->number];
    if(axis >= ABS_HAT0X && axis <= ABS_HAT3Y)
    {
      evt.type = je->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
      int button;
      int value;
      axis -= ABS_HAT0X;
      if(!je->value)
      {
        value = joystick[device].hat_value[axis];
        joystick[device].hat_value[axis] = 0;
      }
      else
      {
        value = je->value/32767;
        joystick[device].hat_value[axis] = value;
      }
      button = axis + value + 2*(axis/2);
      if(button < 4*(axis/2))
      {
        button += 4;
      }
      evt.jbutton.which = joystick[device].id;
      evt.jbutton.button = button + joystick[device].button_nb;
    }
    else
    {
      evt.type = GE_JOYAXISMOTION;
      evt.jaxis.which = joystick[device].id;
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
    eprintf("event from joystick: %s\n", joystick[device].name);
    eprintf("type: %d number: %d value: %d\n", je->type, je->number, je->value);
    event_callback(&evt);
  }
}

static struct js_event je[MAX_EVENTS];

static int js_process_events(int index)
{
  unsigned int size = sizeof(je);
  int j;
  int r;

  int tfd = timer_get();

  if(tfd < 0)
  {
    //read a single event
    size = sizeof(*je);
  }

  if((r = read(joystick[index].fd, je, size)) > 0)
  {
    for(j=0; j<r/sizeof(*je); ++j)
    {
      js_process_event(index, je+j);

      if(event_callback == GE_PushEvent)
      {
        return 1;
      }
    }

    if(r < 0 && errno != EAGAIN)
    {
      js_close(index);
    }
  }
  
  return 0;
}

#define DEV_INPUT "/dev/input"
#define JS_DEV_NAME "js"
#define EV_DEV_NAME "event"

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

static int is_js_device(const struct dirent *dir) {
  return strncmp(JS_DEV_NAME, dir->d_name, sizeof(JS_DEV_NAME)-1) == 0;
}

static int is_ev_device(const struct dirent *dir) {
  return strncmp(EV_DEV_NAME, dir->d_name, sizeof(EV_DEV_NAME)-1) == 0;
}

int js_init()
{
  int ret = 0;
  int i, j;
  int fd_js, fd_ev;
  char js_file[sizeof("/dev/input/js255")];
  char dir_event[sizeof("/sys/class/input/js255/device/")];
  char event[sizeof("/sys/class/input/js255/device/event255")];
  char name[1024] = {0};

  j_num = 0;
  max_joystick_id = -1;

  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    joystick[i].fd = -1;
    joystick[i].ff_fd = -1;
    joystick[i].id = -1;
  }

  struct dirent **namelist_js;
  int n_js;

  struct dirent **namelist_ev;
  int n_ev;

  n_js = scandir(DEV_INPUT, &namelist_js, is_js_device, alphasort);
  if (n_js >= 0)
  {
    for(i=0; i<n_js && !ret; ++i)
    {
      snprintf(js_file, sizeof(js_file), "%s/%s", DEV_INPUT, namelist_js[i]->d_name);

      fd_js = open (js_file, O_RDONLY | O_NONBLOCK);
      if(fd_js != -1)
      {
        if (ioctl(fd_js, JSIOCGNAME(sizeof(name) - 1), name) < 0) {
          fprintf(stderr, "ioctl EVIOCGNAME failed: %s\n", strerror(errno));
          close(fd_js);
          continue;
        }
        unsigned char buttons;
        if (ioctl (fd_js, JSIOCGBUTTONS, &buttons) >= 0 && ioctl (fd_js, JSIOCGAXMAP, &joystick[i].ax_map) >= 0)
        {
          joystick[i].name = strdup(name);
          joystick[i].fd = fd_js;
          joystick[i].button_nb = buttons;
          max_joystick_id = i;
          joystick[i].id = j_num;
          j_num++;
          ev_register_source(joystick[i].fd, i, &js_process_events, NULL, &js_close);

          snprintf(dir_event, sizeof(dir_event), "/sys/class/input/%s/device/", namelist_js[i]->d_name);
          
          n_ev = scandir(dir_event, &namelist_ev, is_ev_device, alphasort);
          if (n_ev >= 0)
          {
            for(j=0; j<n_ev && !ret; ++j)
            {
              snprintf(event, sizeof(event), "%s/%s", DEV_INPUT, namelist_ev[j]->d_name);

              fd_ev = open (event, O_RDWR | O_NONBLOCK);
              if(fd_ev != -1)
              {
                unsigned long features[4];
                if (ioctl(fd_ev, EVIOCGBIT(EV_FF, sizeof(features)), features) == -1)
                {
                  perror("ioctl EV_FF");
                  close(fd_ev);
                  continue;
                }
                if (test_bit(FF_RUMBLE, features))
                {
                  /*
                   * Upload a "weak" effect.
                   */

                  struct ff_effect weak =
                  {
                    .type = FF_RUMBLE,
                    .id = -1
                  };

                  if (ioctl(fd_ev, EVIOCSFF, &weak) == -1)
                  {
                    perror("ioctl EVIOCSFF");
                    close(fd_ev);
                    continue;
                  }

                  /*
                   * Upload a "strong" effect.
                   */

                  struct ff_effect strong =
                  {
                    .type = FF_RUMBLE,
                    .id = -1,
                  };

                  if (ioctl(fd_ev, EVIOCSFF, &strong) == -1)
                  {
                    perror("ioctl EVIOCSFF");
                    close(fd_ev);
                    continue;
                  }

                  /*
                   * Store the ids so that the effects can be updated and played later.
                   */

                  joystick[i].weak_id = weak.id;
                  joystick[i].strong_id = strong.id;

                  joystick[i].ff_fd = fd_ev;
                }
                else
                {
                  close(fd_ev);
                }
              }
              free(namelist_ev[j]);
            }
            free(namelist_ev);
          }
        }
        else
        {
          close(fd_js);
        }
      }
      else if(errno == EACCES)
      {
        fprintf(stderr, "can't open %s: %s\n", js_file, strerror(errno));
        ret = -1;
      }

      free(namelist_js[i]);
    }
    free(namelist_js);
  }
  else
  {
    fprintf(stderr, "can't scan directory %s: %s\n", DEV_INPUT, strerror(errno));
    ret = -1;
  }

  return ret;
}

int js_has_ff_rumble(int index)
{
  return (joystick[index].ff_fd != -1);
}

int js_set_ff_rumble(int index, unsigned short weak_timeout, unsigned short weak, unsigned short strong_timeout, unsigned short strong)
{
  int ret = 0;

    struct ff_effect effect =
    {
      .type = FF_RUMBLE
    };

    struct input_event play =
    {
      .type = EV_FF,
      .value = 1 /* play: 1, stop: 0 */
    };

    int fd = joystick[index].ff_fd;

    if(fd < 0)
    {
      return -1;
    }

    if(weak_timeout)
    {
      /*
       * Update the effect.
       */

      effect.id = joystick[index].weak_id;
      effect.u.rumble.strong_magnitude = 0;
      effect.u.rumble.weak_magnitude   = weak;
      effect.replay.length = weak_timeout;

      if (ioctl(fd, EVIOCSFF, &effect) == -1)
      {
        perror("ioctl EVIOCSFF");
        ret = -1;
      }

      /*
       * Play the effect.
       */

      play.code =  effect.id;

      if (write(fd, (const void*) &play, sizeof(play)) == -1)
      {
        perror("write");
        exit(1);
      }
    }

    if(strong_timeout)
    {
      /*
       * Update the effect.
       */

      effect.id = joystick[index].strong_id;
      effect.u.rumble.strong_magnitude = strong;
      effect.u.rumble.weak_magnitude   = 0;
      effect.replay.length = strong_timeout;

      if (ioctl(fd, EVIOCSFF, &effect) == -1)
      {
        perror("ioctl EVIOCSFF");
        ret = -1;
      }

      /*
       * Play the effect.
       */

      play.code =  effect.id;

      if (write(fd, (const void*) &play, sizeof(play)) == -1)
      {
        perror("write");
        exit(1);
      }
    }

    return ret;
}

int js_close(int index)
{
  free(joystick[index].name);
  joystick[index].name = NULL;

  if(joystick[index].fd >= 0)
  {
    ev_remove_source(joystick[index].fd);
    close(joystick[index].fd);
    joystick[index].fd = -1;
  }
  if(joystick[index].ff_fd >= 0)
  {
    close(joystick[index].ff_fd);
    joystick[index].ff_fd = -1;
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
    if(joystick[i].id == index)
    {
      return joystick[i].name;
    }
  }
  return NULL;
}
