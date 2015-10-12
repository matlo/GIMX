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
  int fd; // the opened joystick, or -1 in case the joystick was created using the js_register() function
  char* name; // the name of the joystick
  struct
  {
    unsigned short button_nb; // the base index of the generated hat buttons equals the number of physical buttons
    int hat_value[ABS_HAT3Y-ABS_HAT0X]; // the current hat values
    uint8_t ax_map[AXMAP_SIZE]; // the axis map
  } hat_info; // allows to convert hat axes to buttons
  struct
  {
    int fd;
    int rumble_id;
    int (*rumble_cb)(int index, unsigned short weak, unsigned short strong);
  } force_feedback;
  int uhid_id;
} joystick[GE_MAX_DEVICES] = {};

static int j_num; // the number of joysticks

/*
 * This initializes the data before any other function of this file gets called.
 */
void js_init_static(void) __attribute__((constructor (101)));
void js_init_static(void)
{
  j_num = 0;
  int i;
  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    joystick[i].fd = -1;
    joystick[i].force_feedback.fd = -1;
    joystick[i].uhid_id = -1;
  }
}

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
    evt.jbutton.which = device;
    evt.jbutton.button = je->number;
  }
  else if(je->type & JS_EVENT_AXIS)
  {
    int axis = joystick[device].hat_info.ax_map[je->number];
    if(axis >= ABS_HAT0X && axis <= ABS_HAT3Y)
    {
      // convert hat axes to buttons
      evt.type = je->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
      int button;
      int value;
      axis -= ABS_HAT0X;
      if(!je->value)
      {
        value = joystick[device].hat_info.hat_value[axis];
        joystick[device].hat_info.hat_value[axis] = 0;
      }
      else
      {
        value = je->value/32767;
        joystick[device].hat_info.hat_value[axis] = value;
      }
      button = axis + value + 2*(axis/2);
      if(button < 4*(axis/2))
      {
        button += 4;
      }
      evt.jbutton.which = device;
      evt.jbutton.button = button + joystick[device].hat_info.button_nb;
    }
    else
    {
      evt.type = GE_JOYAXISMOTION;
      evt.jaxis.which = device;
      evt.jaxis.axis = je->number;
      evt.jaxis.value = je->value;
      /*
       * Ugly patch for the sixaxis.
       */
      if(GE_GetJSType(evt.jaxis.which) == GE_JS_SIXAXIS && evt.jaxis.axis > 3 && evt.jaxis.axis < 23)
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
  unsigned int j;
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
#define JS_DEV_NAME "js%u"
#define EV_DEV_NAME "event%u"

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

static int is_js_device(const struct dirent *dir) {
  unsigned int num;
  if(dir->d_type == DT_CHR && sscanf(dir->d_name, JS_DEV_NAME, &num) == 1 && num < 256) {
    return 1;
  }
  return 0;
}

static int is_event_dir(const struct dirent *dir) {
  unsigned int num;
  if(dir->d_type == DT_DIR && sscanf(dir->d_name, EV_DEV_NAME, &num) == 1 && num < 256) {
    return 1;
  }
  return 0;
}

static int open_evdev(const char * js_name)
{
  char dir_event[sizeof("/sys/class/input/js255/device/")];
  char event[sizeof("/sys/class/input/js255/device/event255")];
  struct dirent **namelist_ev;
  int n_ev;
  int j;
  int fd_ev = -1;

  snprintf(dir_event, sizeof(dir_event), "/sys/class/input/%s/device/", js_name);

  // scan /sys/class/input/jsX/device/ for eventY devices
  n_ev = scandir(dir_event, &namelist_ev, is_event_dir, alphasort);
  if (n_ev >= 0)
  {
    for(j=0; j<n_ev; ++j)
    {
      if(fd_ev == -1)
      {
        snprintf(event, sizeof(event), "%s/%s", DEV_INPUT, namelist_ev[j]->d_name);
        // open the eventY device
        fd_ev = open (event, O_RDWR | O_NONBLOCK);
      }
      free(namelist_ev[j]);
    }
    free(namelist_ev);
  }
  return fd_ev;
}

static void get_uhid_id(int index, int fd_ev)
{
  char uniq[64] = {};
  if (ioctl(fd_ev, EVIOCGUNIQ(sizeof(uniq)), &uniq) == -1)
  {
    return;
  }
  pid_t pid;
  int uhid_id;
  if(sscanf(uniq, "GIMX %d %d", &pid, &uhid_id) == 2)
  {
    if(pid == getpid())
    {
      joystick[index].uhid_id = uhid_id;
    }
  }
}

static int start_ff(int index, int fd_ev)
{
  unsigned long features[4];
  if (ioctl(fd_ev, EVIOCGBIT(EV_FF, sizeof(features)), features) == -1)
  {
    perror("ioctl EV_FF");
    return -1;
  }
  if (test_bit(FF_RUMBLE, features))
  {
    // Upload a rumble effect.
    struct ff_effect rumble =
    {
      .type = FF_RUMBLE,
      .id = -1
    };
    if (ioctl(fd_ev, EVIOCSFF, &rumble) == -1)
    {
      perror("ioctl EVIOCSFF");
      return -1;
    }
    // Store the ids so that the effects can be updated and played later.
    joystick[index].force_feedback.fd = fd_ev;
    joystick[index].force_feedback.rumble_id = rumble.id;
    return 0;
  }
  return -1;
}

int js_init()
{
  int ret = 0;
  int i;
  int fd_js;
  char js_file[sizeof("/dev/input/js255")];
  char name[1024] = {0};

  struct dirent **namelist_js;
  int n_js;

  // scan /dev/input for jsX devices
  n_js = scandir(DEV_INPUT, &namelist_js, is_js_device, alphasort);
  if (n_js >= 0)
  {
    for(i=0; i<n_js && !ret; ++i)
    {
      snprintf(js_file, sizeof(js_file), "%s/%s", DEV_INPUT, namelist_js[i]->d_name);

      // open the jsX device
      fd_js = open (js_file, O_RDONLY | O_NONBLOCK);
      if(fd_js != -1)
      {
        // get the device name
        if (ioctl(fd_js, JSIOCGNAME(sizeof(name) - 1), name) < 0) {
          fprintf(stderr, "ioctl EVIOCGNAME failed: %s\n", strerror(errno));
          close(fd_js);
          continue;
        }
        // get the number of buttons and the axis map, to allow converting hat axes to buttons
        unsigned char buttons;
        if (ioctl (fd_js, JSIOCGBUTTONS, &buttons) >= 0 && ioctl (fd_js, JSIOCGAXMAP, &joystick[j_num].hat_info.ax_map) >= 0)
        {
          joystick[j_num].name = strdup(name);
          joystick[j_num].fd = fd_js;
          joystick[j_num].hat_info.button_nb = buttons;
          ev_register_source(joystick[j_num].fd, j_num, &js_process_events, NULL, &js_close);

          int fd_ev = open_evdev(namelist_js[i]->d_name);
          if(fd_ev >= 0)
          {
            get_uhid_id(j_num, fd_ev);
            if(start_ff(j_num, fd_ev) == -1)
            {
              close(fd_ev); //no need to keep it opened
            }
          }

          j_num++;
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
  if(index < j_num)
  {
    return (joystick[index].force_feedback.fd != -1) || (joystick[index].force_feedback.rumble_cb);
  }
  else
  {
    return -1;
  }
}

int js_set_ff_rumble(int index, unsigned short weak, unsigned short strong)
{
  if(index < 0 || index >= j_num)
  {
    return -1;
  }

  int ret = 0;

  int fd = joystick[index].force_feedback.fd;

  if(fd >= 0)
  {
    struct ff_effect effect =
    {
      .type = FF_RUMBLE
    };

    struct input_event play =
    {
      .type = EV_FF,
      .value = 1 /* play: 1, stop: 0 */
    };

    // Update the effect.
    effect.id = joystick[index].force_feedback.rumble_id;
    effect.u.rumble.strong_magnitude = strong;
    effect.u.rumble.weak_magnitude   = weak;
    if (ioctl(fd, EVIOCSFF, &effect) == -1)
    {
      perror("ioctl EVIOCSFF");
      ret = -1;
    }
    // Play the effect.
    play.code =  effect.id;
    if (write(fd, (const void*) &play, sizeof(play)) == -1)
    {
      perror("write");
      ret = -1;
    }
  }
  else if(joystick[index].force_feedback.rumble_cb)
  {
    ret = joystick[index].force_feedback.rumble_cb(index, weak, strong);
  }
  else
  {
    ret = -1;
  }

  return ret;
}

int js_get_uhid_id(int index)
{
  return joystick[index].uhid_id;
}

int js_close(int index)
{
  if(index < j_num)
  {
    free(joystick[index].name);
    joystick[index].name = NULL;

    joystick[index].uhid_id = -1;

    if(joystick[index].fd >= 0)
    {
      ev_remove_source(joystick[index].fd);
      close(joystick[index].fd);
      joystick[index].fd = -1;
    }
    if(joystick[index].force_feedback.fd >= 0)
    {
      close(joystick[index].force_feedback.fd);
      joystick[index].force_feedback.fd = -1;
    }
  }

  return 0;
}

void js_quit()
{
  int i;
  for(i=0; i<j_num; ++i)
  {
    js_close(i);
  }

  j_num = 0;
}

const char* js_get_name(int index)
{
  if(index < j_num)
  {
    return joystick[index].name;
  }
  else
  {
    return NULL;
  }
}

int js_register(const char* name, int (*rumble_cb)(int, unsigned short, unsigned short))
{
  int index = -1;
  if(j_num < GE_MAX_DEVICES)
  {
    index = j_num;
    joystick[index].name = strdup(name);
    joystick[index].force_feedback.rumble_cb = rumble_cb;
    ++j_num;
  }
  return index;
}
