#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>

#include <sys/select.h>
#include <sched.h>

#include <SDL/SDL.h>

#include "events.h"
#include "emuclient.h"
#include "calibration.h"
#include "macros.h"
#include "config.h"

#include <sys/signalfd.h>

#include <math.h>

#define MAX_EVENTS 256

#define eprintf(...) if(debug) printf(__VA_ARGS__)

int debug = 0;
static int grab = 0;

SDL_Event evqueue[MAX_EVENTS];
unsigned char evqueue_index_first = 0;
unsigned char evqueue_index_last = 0;

int SDL_PushEvent(SDL_Event* ev)
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

int SDL_PeepEvents(SDL_Event *events, int numevents, SDL_eventaction action, Uint32 mask)
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

const char *SDL_JoystickName(int id)
{
  return ev_get_name(DEVTYPE_JOYSTICK, id);
}

SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
{
  if(mode == SDL_GRAB_ON)
  {
    grab = 1;
  }
  else
  {
    grab = 0;
  }
  return mode;
}

void SDL_FreeSurface (SDL_Surface *surface)
{

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

int k_num = 0;
int m_num = 0;
int j_num = 0;

unsigned short joystickButtonIds[MAX_DEVICES][KEY_MAX-BTN_JOYSTICK] = {{0}};
unsigned short joystickButtonNb[MAX_DEVICES] = {0};
int joystickHatValue[MAX_DEVICES][ABS_HAT3Y-ABS_HAT0X] = {{0}};
double joystickAxisScale[MAX_DEVICES][ABS_MAX] = {{0}};
double joystickAxisShift[MAX_DEVICES][ABS_MAX] = {{0}};
unsigned short joystickAxisIds[MAX_DEVICES][ABS_MAX] = {{0}};
unsigned short joystickAxisNb[MAX_DEVICES] = {0};

int max_device_id = 0;

int read_device_type(int index, int fd)
{
  char name[1024]                             = {0};
  unsigned long key_bitmask[NLONGS(KEY_CNT)] = {0};
  unsigned long rel_bitmask[NLONGS(REL_CNT)] = {0};
  unsigned long abs_bitmask[NLONGS(ABS_CNT)] = {0};
  struct input_absinfo absinfo;
  int i, len;
  int has_rel_axes = 0;
  int has_abs_axes = 0;
  int has_keys = 0;
  int has_joy_keys = 0;
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

  len = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
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
  //printf("%s\n", name);
  for (i = 0; i < ABS_MAX; i++) {
      if (BitIsSet(abs_bitmask, i)) {
          has_abs_axes = 1;
          if (ioctl(fd, EVIOCGABS(i), &absinfo)) {
            fprintf(stderr, "ioctl EVIOCGBIT failed: %s\n", strerror(errno));
            return -1;
          }
          if(i < ABS_MISC)
          {
            joystickAxisShift[j_num][i] = (double) (absinfo.maximum - absinfo.minimum) / 2;
            if(joystickAxisShift[j_num][i])
            {
              joystickAxisScale[j_num][i] = (double) 32767 / joystickAxisShift[j_num][i];
            }
            //printf("%d %.04f %.04f\n", i, joystickAxisShift[j_num][i], joystickAxisScale[j_num][i]);
          }
          else
          {
            joystickAxisScale[j_num][i] = (double) 32767 / absinfo.maximum;
          }
          joystickAxisIds[j_num][i] = joystickAxisNb[j_num];
          joystickAxisNb[j_num]++;
          /*printf("%d %d (min:%d max:%d flat:%d fuzz:%d)\n",
              i,
              absinfo.value,
              absinfo.minimum,
              absinfo.maximum,
              absinfo.flat,
              absinfo.fuzz);*/
      }
  }

  for (i = 0; i < BTN_MISC; i++) {
      if (BitIsSet(key_bitmask, i)) {
          has_keys = 1;
          break;
      }
  }
  for (i = BTN_JOYSTICK; i < KEY_MAX; i++) {
      if (BitIsSet(key_bitmask, i)) {
          has_joy_keys = 1;
          joystickButtonIds[j_num][i-BTN_JOYSTICK] = joystickButtonNb[j_num];
          joystickButtonNb[j_num]++;
      }
  }

  if(!has_rel_axes && !has_keys && !has_scroll && !has_abs_axes && !has_joy_keys)
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
  if(has_abs_axes || has_joy_keys)
  {
    device_type[index] |= DEVTYPE_JOYSTICK;
    device_id[index][2] = j_num;
    j_num++;
  }

  device_name[index] = strdup(name);

  return 0;
}

void display_devices()
{
  int i;
  char* name;

  for(i=0; i<=max_device_id; ++i)
  {
    if((name = ev_get_name(DEVTYPE_KEYBOARD, i)))
    {
      eprintf("keyboard %d is named [%s]\n", i, name);
    }
    if((name = ev_get_name(DEVTYPE_MOUSE, i)))
    {
      eprintf("mouse %d is named [%s]\n", i, name);
    }
    if((name = ev_get_name(DEVTYPE_JOYSTICK, i)))
    {
      eprintf("joystick %d is named [%s]\n", i, name);
    }
  }
}

void ev_init()
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

void ev_quit()
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

void SDL_Quit(void)
{
  ev_quit();
}

char* ev_get_name(unsigned char devtype, int index)
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

const char* SDL_GetMouseName(int id)
{
  return ev_get_name(DEVTYPE_MOUSE, id);
}

const char* SDL_GetKeyboardName(int id)
{
  return ev_get_name(DEVTYPE_KEYBOARD, id);
}

void SDL_PumpEvents(void)
{
  int i, j;

  int nfds;
  fd_set rfds;
  int r;

  struct input_event ie[MAX_EVENTS];
  SDL_Event evt;

  while(1)
  {
    FD_ZERO(&rfds);
    FD_SET(tfd, &rfds);
    nfds = tfd+1;
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
                      evt.type = ie[j].value ? SDL_KEYDOWN : SDL_KEYUP;
                      evt.key.keysym.sym = ie[j].code;
                    }
                  }
                }
                if((device_type[i] & DEVTYPE_JOYSTICK))
                {
                  if(ie[j].type == EV_KEY)
                  {
                    if(ie[j].code >= BTN_JOYSTICK && ie[j].code < KEY_MAX)
                    {
                      evt.jbutton.which = device_id[i][2];
                      evt.type = ie[j].value ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
                      evt.jbutton.button = joystickButtonIds[device_id[i][2]][ie[j].code-BTN_JOYSTICK];
                    }
                  }
                  else if(ie[j].type == EV_ABS)
                  {
                    if(ie[j].code >= ABS_HAT0X && ie[j].code <= ABS_HAT3Y)
                    {
                      evt.type = ie[j].value ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
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
                      evt.type = SDL_JOYAXISMOTION;
                      evt.jaxis.which = device_id[i][2];
                      evt.jaxis.axis = joystickAxisIds[device_id[i][2]][ie[j].code];
                      evt.jaxis.value = (ie[j].value - joystickAxisShift[device_id[i][2]][ie[j].code]) * joystickAxisScale[device_id[i][2]][ie[j].code];
                    }
                  }
                }
                if(device_type[i] & DEVTYPE_MOUSE)
                {
                  if(ie[j].type == EV_KEY)
                  {
                    if(ie[j].code >= BTN_LEFT && ie[j].code <= BTN_TASK)
                    {
                      evt.type = ie[j].value ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
                      evt.button.button = ie[j].code - BTN_MOUSE;
                    }
                  }
                  else if(ie[j].type == EV_REL)
                  {
                    if(ie[j].code == 0)
                    {
                      evt.type = SDL_MOUSEMOTION;
                      evt.motion.xrel = ie[j].value;
                    }
                    else if(ie[j].code == 1)
                    {
                      evt.type = SDL_MOUSEMOTION;
                      evt.motion.yrel = ie[j].value;
                    }
                    else if(ie[j].code == 8)
                    {
                      evt.type = SDL_MOUSEBUTTONDOWN;
                      evt.button.button = (ie[j].value > 0) ? 8 : 9;
                    }
                    else if(ie[j].code == 6)
                    {
                      evt.type = SDL_MOUSEBUTTONDOWN;
                      evt.button.button = (ie[j].value > 0) ? 10 : 11;
                    }
                  }
                }

                /*
                 * Process evt.
                 */
                if(evt.type != SDL_NOEVENT)
                {
                  eprintf("event from device: %s\n", device_name[i]);
                  eprintf("type: %d code: %d value: %d\n", ie[j].type, ie[j].code, ie[j].value);
                  SDL_ProcessEvent(&evt);
                  if(evt.type == SDL_MOUSEBUTTONDOWN)
                  {
                    if(evt.button.button >= 8 && evt.button.button <= 11)
                    {
                      evt.type = SDL_MOUSEBUTTONUP;
                      SDL_ProcessEvent(&evt);
                    }
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
      if(FD_ISSET(tfd, &rfds))
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
          break; //a report has to be sent!
        }
      }
    }
  }
}

void SDL_ProcessEvent(SDL_Event* event)
{
  if (event->type != SDL_MOUSEMOTION)
  {
    if (!cal_skip_event(event))
    {
      cfg_process_event(event);
    }
  }
  else
  {
    cfg_process_motion_event(event);
  }

  cfg_trigger_lookup(event);
  cfg_intensity_lookup(event);

  switch (event->type)
  {
    case SDL_MOUSEBUTTONDOWN:
      cal_button(event->button.which, event->button.button);
      break;
    case SDL_KEYDOWN:
      cal_key(event->key.which, event->key.keysym.sym, 1);
      break;
    case SDL_KEYUP:
      cal_key(event->key.which, event->key.keysym.sym, 0);
      break;
  }

  macro_lookup(event);
}
