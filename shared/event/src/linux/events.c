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
#include <sys/select.h>
#include "events.h"
#include <timer.h>
#include <sys/signalfd.h>
#include <math.h>
#include <GE.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <string.h>

#define MAX_KEYNAMES (KEY_MICMUTE+1)

#define DEVTYPE_KEYBOARD 0x01
#define DEVTYPE_MOUSE    0x02
#define DEVTYPE_JOYSTICK 0x04
#define DEVTYPE_NB       3

#define MAX_EVENTS 256

#define AXMAP_SIZE (ABS_MAX + 1)

#define eprintf(...) if(debug) printf(__VA_ARGS__)

static int debug = 0;
static int grab = 0;

static unsigned char device_type[GE_MAX_DEVICES];
static int device_fd[GE_MAX_DEVICES];
static int device_id[GE_MAX_DEVICES][DEVTYPE_NB];
static char* device_name[GE_MAX_DEVICES];
static int k_num;
static int m_num;
static int max_device_id;

static int joystick_fd[GE_MAX_DEVICES];
static int joystick_id[GE_MAX_DEVICES];
static char* joystick_name[GE_MAX_DEVICES];
//static unsigned short joystick_button_ids[MAX_DEVICES][KEY_MAX-BTN_JOYSTICK] = {{0}};
static unsigned short joystick_button_nb[GE_MAX_DEVICES] = {0};
static int joystick_hat_value[GE_MAX_DEVICES][ABS_HAT3Y-ABS_HAT0X] = {{0}};
static uint8_t joystick_ax_map[GE_MAX_DEVICES][AXMAP_SIZE] = {{0}};
static int j_num;
static int max_joystick_id;

static GE_Event evqueue[MAX_EVENTS];
static unsigned char evqueue_index_first = 0;
static unsigned char evqueue_index_last = 0;
static GE_Event next_event = {};

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

void ev_grab_input(int mode)
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
    if(device_fd[i] > -1)
    {
      ioctl(device_fd[i], EVIOCGRAB, enable);
    }
  }
}

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

static inline int BitIsSet(const unsigned long *array, int bit)
{
    return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

static int read_device_type(int index, int fd)
{
  char name[1024]                             = {0};
  unsigned long key_bitmask[NLONGS(KEY_CNT)] = {0};
  unsigned long rel_bitmask[NLONGS(REL_CNT)] = {0};
  //unsigned long abs_bitmask[NLONGS(ABS_CNT)] = {0};
  //struct input_absinfo absinfo;
  int i, len;
  int has_rel_axes = 0;
  //int has_abs_axes = 0;
  int has_keys = 0;
  //int has_joy_keys = 0;
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

  /*len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
  if (len < 0) {
    fprintf(stderr, "ioctl EVIOCGBIT failed: %s\n", strerror(errno));
    return -1;
  }*/

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
  /*printf("%s\n", name);
  for (i = 0; i < ABS_MAX; i++) {
      if (BitIsSet(abs_bitmask, i)) {
          has_abs_axes = 1;
          if (ioctl(fd, EVIOCGABS(i), &absinfo)) {
            fprintf(stderr, "ioctl EVIOCGBIT failed: %s\n", strerror(errno));
            return -1;
          }
          if(i < ABS_MISC)
          {
            if(absinfo.minimum > 0)
            {
              joystickAxisShift[j_num][i] = (double) (absinfo.maximum - absinfo.minimum) / 2;
            }
            else
            {
              joystickAxisShift[j_num][i] = (double) (absinfo.maximum + absinfo.minimum) / 2;
            }
            int rad = absinfo.maximum - joystickAxisShift[j_num][i];
            if(rad)
            {
              joystickAxisScale[j_num][i] = (double) 32767 / rad;
            }
            else
            {
              joystickAxisScale[j_num][i] = 1;
            }
            printf("%d %.04f %.04f\n", i, joystickAxisShift[j_num][i], joystickAxisScale[j_num][i]);
          }
          else
          {
            joystickAxisScale[j_num][i] = (double) 32767 / absinfo.maximum;
          }
          joystickAxisIds[j_num][i] = joystickAxisNb[j_num];
          joystickAxisNb[j_num]++;
          printf("%d %d (min:%d max:%d flat:%d fuzz:%d)\n",
              i,
              absinfo.value,
              absinfo.minimum,
              absinfo.maximum,
              absinfo.flat,
              absinfo.fuzz);
      }
  }*/

  for (i = 0; i < BTN_MISC; i++) {
      if (BitIsSet(key_bitmask, i)) {
          has_keys = 1;
          break;
      }
  }
  /*for (i = BTN_JOYSTICK; i < KEY_MAX; i++) {
      if (BitIsSet(key_bitmask, i)) {
          has_joy_keys = 1;
          joystick_button_ids[j_num][i-BTN_JOYSTICK] = joystick_button_nb[j_num];
          joystick_button_nb[j_num]++;
      }
  }*/

  if(!has_rel_axes && !has_keys && !has_scroll/* && !has_abs_axes && !has_joy_keys*/)
  {
    return -1;
  }

  if(has_keys)
  {
    device_type[index] = DEVTYPE_KEYBOARD;
    device_id[index][0] = k_num;
    k_num++;
  }
  if(has_rel_axes || has_scroll)
  {
    device_type[index] |= DEVTYPE_MOUSE;
    device_id[index][1] = m_num;
    m_num++;
  }
  /*if(has_abs_axes || has_joy_keys)
  {
    device_type[index] |= DEVTYPE_JOYSTICK;
    device_id[index][2] = j_num;
    j_num++;
  }*/

  device_name[index] = strdup(name);

  return 0;
}

static char* evdev_get_name(unsigned char devtype, int index)
{
  int i;
  int nb = 0;
  for(i=0; i<=max_device_id; ++i)
  {
    if(device_type[i] & devtype)
    {
      if(index == nb)
      {
        return device_name[i];
      }
      nb++;
    }
  }
  return NULL;
}

static void display_devices()
{
  int i;
  char* name;

  for(i=0; i<=max_device_id; ++i)
  {
    if((name = evdev_get_name(DEVTYPE_KEYBOARD, i)))
    {
      eprintf("keyboard %d is named [%s]\n", i, name);
    }
    if((name = evdev_get_name(DEVTYPE_MOUSE, i)))
    {
      eprintf("mouse %d is named [%s]\n", i, name);
    }
  }
  for(i=0; i<=max_joystick_id; ++i)
  {
    if((name = joystick_name[i]))
    {
      eprintf("joystick %d is named [%s]\n", i, name);
    }
  }
}

static int jsdev_init()
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

  for(i=0; i<GE_MAX_DEVICES && !ret; ++i)
  {
    sprintf(joystick, "/dev/input/js%d", i);
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
        //printf("js=%d id=%d nb_buttons=%d\n", i, joystick_id[i], joystick_button_nb[i]);
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
  }

  return ret;
}

void ev_joystick_close(int id)
{

}

static void jsdev_quit()
{
  int i;
  for(i=0; i<=max_joystick_id; ++i)
  {
    if(joystick_name[i])
    {
      free(joystick_name[i]);
    }
    if(joystick_fd[i] > -1)
    {
      close(joystick_fd[i]);
    }
  }
}

static int evdev_init()
{
  int ret = 0;
  int i, j;
  int fd;
  char device[sizeof("/dev/input/event255")];

  /*
   * Avoid the enter key from being still pressed after the process exit.
   * This is only done if the process is launched in a terminal.
   */
  if(isatty(fileno(stdin)))
  {
    sleep(1);
  }

  struct termios term;
  tcgetattr(STDOUT_FILENO, &term);
  term.c_lflag &= ~ECHO;
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);

  memset(device_type, 0x00, sizeof(device_type));
  memset(device_name, 0x00, sizeof(device_name));
  max_device_id = -1;
  k_num = 0;
  m_num = 0;

  for(i=0; i<GE_MAX_DEVICES; ++i)
  {
    device_fd[i] = -1;

    for(j=0; j<DEVTYPE_NB; ++j)
    {
      device_id[i][j] = -1;
    }
  }

  for(i=0; i<GE_MAX_DEVICES && !ret; ++i)
  {
    sprintf(device, "/dev/input/event%d", i);
    fd = open (device, O_RDONLY | O_NONBLOCK);
    if(fd != -1)
    {
      if(read_device_type(i, fd) != -1)
      {
        device_fd[i] = fd;
        if(grab)
        {
          ioctl(device_fd[i], EVIOCGRAB, (void *)1);
        }
        max_device_id = i;
        //printf("%d %d\n", i, fd);
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
  }

  if(ret < 0)
  {
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag |= ECHO;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term);
  }

  return ret;
}

int ev_init()
{
  int ret = evdev_init();

  if(ret < 0)
  {
    fprintf(stderr, "evdev_init failed.\n");
    return 0;
  }

  ret = jsdev_init();

  if(ret < 0)
  {
    fprintf(stderr, "jsdev_init failed.\n");
    return 0;
  }

  evqueue_index_first = 0;
  evqueue_index_last = 0;

  return 1;
}

static void evdev_quit()
{
  int i;
  for(i=0; i<=max_device_id; ++i)
  {
    if(device_name[i])
    {
      free(device_name[i]);
    }
    if(device_fd[i] > -1)
    {
      if(grab)
      {
        ioctl(device_fd[i], EVIOCGRAB, (void *)0);
      }
      close(device_fd[i]);
    }
  }

  struct termios term;
  tcgetattr(STDOUT_FILENO, &term);
  term.c_lflag |= ECHO;
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);
  if(!grab)
  {
    tcflush(STDIN_FILENO, TCIFLUSH);
  }
}

void ev_quit(void)
{
  evdev_quit();
  jsdev_quit();
}

const char* ev_joystick_name(int index)
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

const char* ev_mouse_name(int id)
{
  return evdev_get_name(DEVTYPE_MOUSE, id);
}

const char* ev_keyboard_name(int id)
{
  return evdev_get_name(DEVTYPE_KEYBOARD, id);
}

static int (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(int (*fp)(GE_Event*))
{
  event_callback = fp;
}

static void ev_process_input_events(int device, struct input_event* ie)
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
  if((device_type[device] & DEVTYPE_KEYBOARD))
  {
    if(ie->type == EV_KEY)
    {
      if(ie->code > 0 && ie->code < MAX_KEYNAMES)
      {
        evt.type = ie->value ? GE_KEYDOWN : GE_KEYUP;
        evt.key.which = device_id[device][0];
        evt.key.keysym = ie->code;
      }
    }
  }
  /*if((device_type[device] & DEVTYPE_JOYSTICK))
  {
    if(ie->type == EV_KEY)
    {
      if(ie->code >= BTN_JOYSTICK && ie->code < KEY_MAX)
      {
        evt.jbutton.which = device_id[device][2];
        evt.type = ie->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
        evt.jbutton.button = joystick_button_ids[device_id[device][2]][ie->code-BTN_JOYSTICK];
      }
    }
    else if(ie->type == EV_ABS)
    {
      if(ie->code >= ABS_HAT0X && ie->code <= ABS_HAT3Y)
      {
        evt.type = ie->value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
        int button;
        int value;
        int axis = ie->code-ABS_HAT0X;
        if(!ie->value)
        {
          value = joystick_hat_value[device_id[device][2]][axis];
          joystick_hat_value[device_id[device][2]][axis] = 0;
        }
        else
        {
          value = ie->value;
          joystick_hat_value[device_id[device][2]][axis] = value;
        }
        button = axis + value + 2*(axis/2);
        if(button < 4*(axis/2))
        {
          button += 4;
        }
        evt.jbutton.which = device_id[device][2];
        evt.jbutton.button = button + joystick_button_nb[device_id[device][2]];
      }
      else
      {
        evt.type = GE_JOYAXISMOTION;
        evt.jaxis.which = device_id[device][2];
        evt.jaxis.axis = joystickAxisIds[device_id[device][2]][ie->code];
        evt.jaxis.value = (ie->value - joystickAxisShift[device_id[device][2]][ie->code]) * joystickAxisScale[device_id[device][2]][ie->code];
      }
    }
  }*/
  if(device_type[device] & DEVTYPE_MOUSE)
  {
    if(ie->type == EV_KEY)
    {
      if(ie->code >= BTN_LEFT && ie->code <= BTN_TASK)
      {
        evt.type = ie->value ? GE_MOUSEBUTTONDOWN : GE_MOUSEBUTTONUP;
        evt.button.which = device_id[device][1];
        evt.button.button = ie->code - BTN_MOUSE;
      }
    }
    else if(ie->type == EV_REL)
    {
      if(ie->code == REL_X)
      {
        evt.type = GE_MOUSEMOTION;
        evt.motion.which = device_id[device][1];
        evt.motion.xrel = ie->value;
      }
      else if(ie->code == REL_Y)
      {
        evt.type = GE_MOUSEMOTION;
        evt.motion.which = device_id[device][1];
        evt.motion.yrel = ie->value;
      }
      else if(ie->code == REL_WHEEL)
      {
        evt.type = GE_MOUSEBUTTONDOWN;
        evt.button.which = device_id[device][1];
        evt.button.button = (ie->value > 0) ? GE_BTN_WHEELUP : GE_BTN_WHEELDOWN;
      }
      else if(ie->code == REL_HWHEEL)
      {
        evt.type = GE_MOUSEBUTTONDOWN;
        evt.button.which = device_id[device][1];
        evt.button.button = (ie->value > 0) ? GE_BTN_WHEELRIGHT : GE_BTN_WHEELLEFT;
      }
    }
  }

  /*
   * Process evt.
   */
  if(evt.type != GE_NOEVENT)
  {
    eprintf("event from device: %s\n", device_name[device]);
    eprintf("type: %d code: %d value: %d\n", ie->type, ie->code, ie->value);
    event_callback(&evt);
    if(evt.type == GE_MOUSEBUTTONDOWN)
    {
      if(ie->code == REL_WHEEL || ie->code == REL_HWHEEL)
      {
        evt.type = GE_MOUSEBUTTONUP;
        memcpy(&next_event, &evt, sizeof(next_event));
      }
    }
  }
}

static void ev_process_js_events(int device, struct js_event* je)
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
  int i, j;

  int nfds = 0;
  fd_set rfds;
  int r;

  struct input_event ie[MAX_EVENTS];
  struct js_event je[MAX_EVENTS];

  if(event_callback == NULL)
  {
    fprintf(stderr, "ev_set_callback should be called first!\n");
    return;
  }

  int tfd = timer_getfd();
  
  if(tfd < 0 && next_event.type != GE_NOEVENT)
  {
    event_callback(&next_event);
    next_event.type = GE_NOEVENT;
    return;
  }

  while(1)
  {
    FD_ZERO(&rfds);
    if(tfd > -1)
    {
      FD_SET(tfd, &rfds);
      nfds = tfd+1;
    }
    for(i=0; i<=max_device_id; ++i)
    {
      if(device_fd[i] > -1)
      {
        FD_SET(device_fd[i], &rfds);
        if(nfds > -1 && device_fd[i] >= nfds)
        {
          nfds = device_fd[i]+1;
        }
      }
    }
    for(i=0; i<=max_joystick_id; ++i)
    {
      if(joystick_fd[i] > -1)
      {
        FD_SET(joystick_fd[i], &rfds);
        if(nfds > -1 && joystick_fd[i] >= nfds)
        {
          nfds = joystick_fd[i]+1;
        }
      }
    }

    if(select(nfds, &rfds, NULL, NULL, NULL) > 0)
    {
      for(i=0; i<=max_device_id; ++i)
      {
        if(device_fd[i] > -1)
        {
          if(FD_ISSET(device_fd[i], &rfds))
          {
            unsigned int size = sizeof(ie);

            if(tfd < 0)
            {
              size = sizeof(*ie);
            }

            if((r = read(device_fd[i], ie, size)) > 0)
            {
              for(j=0; j<r/sizeof(*ie); ++j)
              {
                ev_process_input_events(i, ie+j);

                if(tfd < 0)
                {
                  return;
                }
              }

              if(r < 0 && errno != EAGAIN)
              {
                //printf("%d %s %d\n", device_fd[i], ev_get_name(device_type[i], i), errno);
                close(device_fd[i]);
                device_fd[i] = -1;
                //display_devices();
              }
            }
          }
        }
      }
      for(i=0; i<=max_joystick_id; ++i)
      {
        if(joystick_fd[i] > -1)
        {
          if(FD_ISSET(joystick_fd[i], &rfds))
          {
            unsigned int size = sizeof(ie);

            if(tfd < 0)
            {
              size = sizeof(*ie);
            }

            if((r = read(joystick_fd[i], je, size)) > 0)
            {
              for(j=0; j<r/sizeof(*je); ++j)
              {
                ev_process_js_events(i, je+j);

                if(tfd < 0)
                {
                  return;
                }
              }

              if(r < 0 && errno != EAGAIN)
              {
                //printf("%d %s %d\n", device_fd[i], ev_get_name(device_type[i], i), errno);
                close(device_fd[i]);
                device_fd[i] = -1;
                //display_devices();
              }
            }
          }
        }
      }
      if(tfd > -1 && FD_ISSET(tfd, &rfds))
      {
        uint64_t exp;

        ssize_t res;

        res = read (tfd, &exp, sizeof(exp));

        if (res != sizeof(exp)) {
          fprintf (stderr, "Wrong timer fd read...\n");
        }
        else
        {
          if(exp > 1)
          {
            fprintf (stderr, "Timer fired several times...\n");
          }
          return; //a report has to be sent!
        }
      }
    }
  }
}
