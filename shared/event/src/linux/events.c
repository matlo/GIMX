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

#define MAX_EVENTS 256

#define AXMAP_SIZE (ABS_MAX + 1)

#define eprintf(...) if(debug) printf(__VA_ARGS__)

int debug = 0;
static int grab = 0;

GE_Event evqueue[MAX_EVENTS];
unsigned char evqueue_index_first = 0;
unsigned char evqueue_index_last = 0;

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

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)

static inline int BitIsSet(const unsigned long *array, int bit)
{
    return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
}

unsigned char device_type[MAX_DEVICES] = {0};
int device_fd[MAX_DEVICES];
int device_id[MAX_DEVICES][DEVTYPE_NB];
char* device_name[MAX_DEVICES] = {0};

int joystick_fd[MAX_DEVICES];
int joystick_id[MAX_DEVICES];
char* joystick_name[MAX_DEVICES] = {0};

int k_num = 0;
int m_num = 0;
int j_num = 0;

//unsigned short joystickButtonIds[MAX_DEVICES][KEY_MAX-BTN_JOYSTICK] = {{0}};
unsigned short joystickButtonNb[MAX_DEVICES] = {0};
int joystickHatValue[MAX_DEVICES][ABS_HAT3Y-ABS_HAT0X] = {{0}};
uint8_t joystickAxMap[MAX_DEVICES][AXMAP_SIZE] = {{0}};

int max_device_id = 0;
int max_joystick_id = 0;

void ev_grab_input(int mode)
{
  int i;
  for(i=0; i<MAX_DEVICES; ++i)
  {
    if(device_fd[i] > -1)
    {
      if(mode == GE_GRAB_OFF)
      {
        ioctl(device_fd[i], EVIOCGRAB, (void *)0);
      }
      else
      {
        ioctl(device_fd[i], EVIOCGRAB, (void *)1);
      }
    }
  }
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
          joystickButtonIds[j_num][i-BTN_JOYSTICK] = joystickButtonNb[j_num];
          joystickButtonNb[j_num]++;
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
  for(i=0; i<MAX_DEVICES; ++i)
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

static void jsdev_init()
{
  int i;
  int fd;
  char joystick[sizeof("/dev/input/js255")];
  char name[1024] = {0};

  for(i=0; i<MAX_DEVICES; ++i)
  {
    joystick_fd[i] = -1;
    joystick_id[i] = -1;
  }

  for(i=0; i<MAX_DEVICES; ++i)
  {
    sprintf(joystick, "/dev/input/js%d", i);
    if((fd = open (joystick, O_RDONLY | O_NONBLOCK)) != -1)
    {
      if (ioctl(fd, JSIOCGNAME(sizeof(name) - 1), name) < 0) {
        fprintf(stderr, "ioctl EVIOCGNAME failed: %s\n", strerror(errno));
        close(fd);
        continue;
      }
      //printf("%s\n", name);
      unsigned char buttons;
      if (ioctl (fd, JSIOCGBUTTONS, &buttons) >= 0 && ioctl (fd, JSIOCGAXMAP, &joystickAxMap[i]) >= 0)
      {
        joystick_name[i] = strdup(name);
        joystick_fd[i] = fd;
        joystickButtonNb[i] = buttons;
        max_joystick_id = i;
        joystick_id[i] = j_num;
        j_num++;
        //printf("js=%d id=%d nb_buttons=%d\n", i, joystick_id[i], joystickButtonNb[i]);
      }
      else
      {
        close(fd);
      }
    }
  }
}

static void jsdev_quit()
{
  int i;
  for(i=0; i<MAX_DEVICES; ++i)
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

static void evdev_init()
{
  int i, j;
  int fd;
  char device[sizeof("/dev/input/event255")];

  /*
   * Avoid the enter key from being still pressed after the process exit.
   */
  sleep(1);

  struct termios term;
  tcgetattr(STDOUT_FILENO, &term);
  term.c_lflag &= ~ECHO;
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);

  for(i=0; i<MAX_DEVICES; ++i)
  {
    device_fd[i] = -1;

    for(j=0; j<DEVTYPE_NB; ++j)
    {
      device_id[i][j] = -1;
    }
  }

  for(i=0; i<MAX_DEVICES; ++i)
  {
    sprintf(device, "/dev/input/event%d", i);
    if((fd = open (device, O_RDONLY | O_NONBLOCK)) != -1)
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
  }
  display_devices();
}

int ev_init()
{
  evdev_init();
  jsdev_init();

  return 1;
}

void ev_set_caption (const char *title, const char *icon)
{

}

static void evdev_quit()
{
  int i;
  for(i=0; i<MAX_DEVICES; ++i)
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
  for(i=0; i<MAX_DEVICES; ++i)
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

void (*event_callback)(GE_Event*) = NULL;

void ev_set_callback(void (*fp)(GE_Event*))
{
  event_callback = fp;
}

void ev_pump_events(void)
{
  int i, j;

  int nfds = 0;
  fd_set rfds;
  int r;

  struct input_event ie[MAX_EVENTS];
  struct js_event je[MAX_EVENTS];
  GE_Event evt;

  if(event_callback == NULL)
  {
    fprintf(stderr, "ev_set_callback should be called first!\n");
    return;
  }

  int tfd = timer_getfd();
  
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
        if(nfds > -1 && device_fd[i] > nfds)
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
        if(nfds > -1 && joystick_fd[i] > nfds)
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
            if((r = read(device_fd[i], ie, sizeof(ie))) > 0)
            {
              for(j=0; j<r/sizeof(ie[0]); ++j)
              {
                memset(&evt, 0x00, sizeof(evt));

                switch(ie[j].type)
                {
                  case EV_KEY:
                    if(ie[j].value > 1)
                    {
                      continue;
                    }
                    break;
                  case EV_MSC:
                    if(ie[j].value > 1)
                    {
                      continue;
                    }
                    if(ie[j].value == 2)
                    {
                      continue;
                    }
                    break;
                  case EV_REL:
                  case EV_ABS:
                    break;
                  default:
                    continue;
                }
                if((device_type[i] & DEVTYPE_KEYBOARD))
                {
                  if(ie[j].type == EV_KEY)
                  {
                    if(ie[j].code > 0 && ie[j].code < MAX_KEYNAMES)
                    {
                      evt.type = ie[j].value ? GE_KEYDOWN : GE_KEYUP;
                      evt.key.keysym = ie[j].code;
                    }
                  }
                }
                /*if((device_type[i] & DEVTYPE_JOYSTICK))
                {
                  if(ie[j].type == EV_KEY)
                  {
                    if(ie[j].code >= BTN_JOYSTICK && ie[j].code < KEY_MAX)
                    {
                      evt.jbutton.which = device_id[i][2];
                      evt.type = ie[j].value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
                      evt.jbutton.button = joystickButtonIds[device_id[i][2]][ie[j].code-BTN_JOYSTICK];
                    }
                  }
                  else if(ie[j].type == EV_ABS)
                  {
                    if(ie[j].code >= ABS_HAT0X && ie[j].code <= ABS_HAT3Y)
                    {
                      evt.type = ie[j].value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
                      int button;
                      int value;
                      int axis = ie[j].code-ABS_HAT0X;
                      if(!ie[j].value)
                      {
                        value = joystickHatValue[device_id[i][2]][axis];
                        joystickHatValue[device_id[i][2]][axis] = 0;
                      }
                      else
                      {
                        value = ie[j].value;
                        joystickHatValue[device_id[i][2]][axis] = value;
                      }
                      button = axis + value + 2*(axis/2);
                      if(button < 4*(axis/2))
                      {
                        button += 4;
                      }
                      evt.jbutton.which = device_id[i][2];
                      evt.jbutton.button = button + joystickButtonNb[device_id[i][2]];
                    }
                    else
                    {
                      evt.type = GE_JOYAXISMOTION;
                      evt.jaxis.which = device_id[i][2];
                      evt.jaxis.axis = joystickAxisIds[device_id[i][2]][ie[j].code];
                      evt.jaxis.value = (ie[j].value - joystickAxisShift[device_id[i][2]][ie[j].code]) * joystickAxisScale[device_id[i][2]][ie[j].code];
                    }
                  }
                }*/
                if(device_type[i] & DEVTYPE_MOUSE)
                {
                  if(ie[j].type == EV_KEY)
                  {
                    if(ie[j].code >= BTN_LEFT && ie[j].code <= BTN_TASK)
                    {
                      evt.type = ie[j].value ? GE_MOUSEBUTTONDOWN : GE_MOUSEBUTTONUP;
                      evt.button.button = ie[j].code - BTN_MOUSE;
                    }
                  }
                  else if(ie[j].type == EV_REL)
                  {
                    if(ie[j].code == 0)
                    {
                      evt.type = GE_MOUSEMOTION;
                      evt.motion.xrel = ie[j].value;
                    }
                    else if(ie[j].code == 1)
                    {
                      evt.type = GE_MOUSEMOTION;
                      evt.motion.yrel = ie[j].value;
                    }
                    else if(ie[j].code == 8)
                    {
                      evt.type = GE_MOUSEBUTTONDOWN;
                      evt.button.button = (ie[j].value > 0) ? 8 : 9;
                    }
                    else if(ie[j].code == 6)
                    {
                      evt.type = GE_MOUSEBUTTONDOWN;
                      evt.button.button = (ie[j].value > 0) ? 10 : 11;
                    }
                  }
                }

                /*
                 * Process evt.
                 */
                if(evt.type != GE_NOEVENT)
                {
                  eprintf("event from device: %s\n", device_name[i]);
                  eprintf("type: %d code: %d value: %d\n", ie[j].type, ie[j].code, ie[j].value);
                  if(tfd > -1)
                  {
                    event_callback(&evt);
                    if(evt.type == GE_MOUSEBUTTONDOWN)
                    {
                      if(evt.button.button >= 8 && evt.button.button <= 11)
                      {
                        evt.type = GE_MOUSEBUTTONUP;
                        event_callback(&evt);
                      }
                    }
                  }
                  else
                  {
                    event_callback(&evt);
                    return;
                  }
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
            if((r = read(joystick_fd[i], je, sizeof(je))) > 0)
            {
              for(j=0; j<r/sizeof(je[0]); ++j)
              {
                memset(&evt, 0x00, sizeof(evt));

                if(je[j].type & JS_EVENT_BUTTON)
                {
                  evt.jbutton.which = joystick_id[i];
                  evt.type = je[j].value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
                  evt.jbutton.button = je[j].number;
                }
                else if(je[j].type & JS_EVENT_AXIS)
                {
                  int axis = joystickAxMap[i][je[j].number];
                  if(axis >= ABS_HAT0X && axis <= ABS_HAT3Y)
                  {
                    evt.type = je[j].value ? GE_JOYBUTTONDOWN : GE_JOYBUTTONUP;
                    int button;
                    int value;
                    axis -= ABS_HAT0X;
                    if(!je[j].value)
                    {
                      value = joystickHatValue[i][axis];
                      joystickHatValue[i][axis] = 0;
                    }
                    else
                    {
                      value = je[j].value/32767;
                      joystickHatValue[i][axis] = value;
                    }
                    button = axis + value + 2*(axis/2);
                    if(button < 4*(axis/2))
                    {
                      button += 4;
                    }
                    evt.jbutton.which = joystick_id[i];
                    evt.jbutton.button = button + joystickButtonNb[i];
                  }
                  else
                  {
                    evt.type = GE_JOYAXISMOTION;
                    evt.jaxis.which = joystick_id[i];
                    evt.jaxis.axis = je[j].number;
                    evt.jaxis.value = je[j].value;
                  }
                }

                /*
                 * Process evt.
                 */
                if(evt.type != GE_NOEVENT)
                {
                  eprintf("event from joystick: %s\n", joystick_name[i]);
                  eprintf("type: %d number: %d value: %d\n", je[j].type, je[j].number, je[j].value);
                  if(tfd > -1)
                  {
                    event_callback(&evt);
                  }
                  else
                  {
                    event_callback(&evt);
                    return;
                  }
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
