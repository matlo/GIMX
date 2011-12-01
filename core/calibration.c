/*
 * calibration.c
 *
 *  Created on: 1 juin 2011
 *      Author: Matlo
 *
 *  License: GPLv3
 */

#include <SDL/SDL.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include "config.h"
#include "sdl_tools.h"
#include "config_writter.h"

#define DEFAULT_MULTIPLIER_STEP 0.01
#define EXPONENT_STEP 0.01
#define DURATION 1000000 //1s
#define STEPS 720

extern int refresh;
extern int mean_axis_value;
extern int display;
extern char* config_file;

int current_mouse = 0;
int current_conf = 0;
e_current_cal current_cal = NONE;

static int lctrl = 0;
static int rctrl = 0;
static int lshift = 0;
static int rshift = 0;
static int lalt = 0;
static int ralt = 0;

static const double pi = 3.14159265;

static int test_time = 1000;

/*
 * Used to calibrate mouse controls.
 */
s_mouse_cal mouse_cal[MAX_DEVICES][MAX_CONFIGURATIONS] = {};

int mouse_controller[MAX_DEVICES];

inline int cal_get_controller(int controller)
{
  return mouse_controller[controller];
}

void cal_init()
{
  memset(mouse_cal, 0x00, sizeof(mouse_cal));
}

inline s_mouse_cal* cal_get_mouse(int mouse, int conf)
{
  return &(mouse_cal[mouse][conf]);
}

/*
 * Test translation acceleration.
 */
static void auto_test()
{
  int i, k;
  double d = 0.1;//0.1 inches
  int dots;
  int dpi;

  SDL_Event mouse_evt = { };

  dpi = cal_get_mouse(current_mouse, current_conf)->dpi;
  
  if(dpi <= 0)
  {
    return;
  }

  for (k = 0; k < 6; ++k)
  {
    dots = d*dpi;

    for (i = 0; i < dots; i++)
    {
      mouse_evt.motion.xrel = 1;
      mouse_evt.motion.which = current_mouse;
      mouse_evt.type = SDL_MOUSEMOTION;
      SDL_PushEvent(&mouse_evt);
      usleep(DURATION/dots);
    }

    usleep(1000000);

    for (i = 0; i < dots/2; i++)
    {
      mouse_evt.motion.xrel = -2 * 1;
      mouse_evt.motion.which = current_mouse;
      mouse_evt.type = SDL_MOUSEMOTION;
      SDL_PushEvent(&mouse_evt);
      usleep(DURATION/dots);
    }

    usleep(1000000);

    d = d*2;
  }

  /*if(mouse_cal[current_mouse][current_conf].dzx)
   {
   state[0].user.axis[1][0] = *mouse_cal[current_mouse][current_conf].dzx;
   controller[0].send_command = 1;

   usleep(test_time*1000);

   state[0].user.axis[1][0] = 0;
   controller[0].send_command = 1;
   }*/

  /*if(mouse_cal[current_mouse][current_conf].dzy)
   {
   state[0].user.axis[1][1] = *mouse_cal[current_mouse][current_conf].dzy;
   controller[0].send_command = 1;

   usleep(test_time*1000);

   state[0].user.axis[1][1] = 0;
   controller[0].send_command = 1;

   usleep(500*1000);

   state[0].user.axis[1][1] = *mouse_cal[current_mouse][current_conf].dzy;
   controller[0].send_command = 1;

   usleep(1000*1000);

   state[0].user.axis[1][1] = 0;
   controller[0].send_command = 1;

   usleep(500*1000);

   state[0].user.axis[1][1] = - *mouse_cal[current_mouse][current_conf].dzy;
   controller[0].send_command = 1;

   usleep(test_time*1000);

   state[0].user.axis[1][1] = 0;
   controller[0].send_command = 1;

   usleep(500*1000);

   state[0].user.axis[1][1] = - *mouse_cal[current_mouse][current_conf].dzy;
   controller[0].send_command = 1;

   usleep(1000*1000);

   state[0].user.axis[1][1] = 0;
   controller[0].send_command = 1;
   }*/
}

/*
 *
 */
static void circle_test()
{
  int i, j;
  int dpi;
  SDL_Event mouse_evt = { };
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);
  int step;

  dpi = mcal->dpi;
  
  if(dpi <= 0)
  {
    dpi = 5700;
  }

  while(current_cal == RD || current_cal == VEL)
  {
    step = mcal->vel;
    for (i = step; i < STEPS && current_cal != NONE; i += step)
    {
      for (j = 0; j < DEFAULT_REFRESH_PERIOD / refresh && current_cal != NONE; ++j)
      {
        mouse_evt.motion.xrel = round(mcal->rd * pow((double)dpi/5700, *mcal->ex) * (cos(i * 2 * pi / STEPS) - cos((i - step) * 2 * pi / STEPS)));
        mouse_evt.motion.yrel = round(mcal->rd * pow((double)dpi/5700, *mcal->ex) * (sin(i * 2 * pi / STEPS) - sin((i - step) * 2 * pi / STEPS)));
        mouse_evt.motion.which = current_mouse;
        mouse_evt.type = SDL_MOUSEMOTION;
        SDL_PushEvent(&mouse_evt);
        usleep(refresh / 2);
      }
    }
  }
}

/*
 * Display calibration info.
 */
static void display_calibration()
{
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  if (current_cal != NONE)
  {
    printf("calibrating mouse %s %d\n", sdl_get_mouse_name(current_mouse),
        sdl_get_mouse_virtual_id(current_mouse));
    printf("calibrating conf %d\n", current_conf + 1);
    printf("multiplier_x:");
    if (mcal->mx)
    {
      printf(" %.2f\n", *mcal->mx);
    }
    else
    {
      printf(" NA\n");
    }
    printf("x/y_ratio:");
    if (mcal->mx && mcal->my)
    {
      printf(" %.2f\n", *mcal->my / *mcal->mx);
    }
    else
    {
      printf(" NA\n");
    }
    printf("dead_zone_x:");
    if (mcal->dzx)
    {
      printf(" %d\n", *mcal->dzx);
    }
    else
    {
      printf(" NA\n");
    }
    printf("dead_zone_y:");
    if (mcal->dzy)
    {
      printf(" %d\n", *mcal->dzy);
    }
    else
    {
      printf(" NA\n");
    }
    printf("shape:");
    if (mcal->dzs)
    {
      if (*mcal->dzs == E_SHAPE_CIRCLE)
        printf(" Circle\n");
      else
        printf(" Rectangle\n");
    }
    else
    {
      printf(" NA\n");
    }
    printf("exponent_x:");
    if (mcal->ex)
    {
      printf(" %.2f\n", *mcal->ex);
    }
    else
    {
      printf(" NA\n");
    }
    printf("exponent_y:");
    if (mcal->ey)
    {
      printf(" %.2f\n", *mcal->ey);
    }
    else
    {
      printf(" NA\n");
    }
    printf("radius: %d\n", mcal->rd);
    printf("velocity: %d\n", mcal->vel);
    printf("time: %d\n", test_time);
  }
}

/*
 * Use keys to calibrate the mouse.
 */
void cal_key(int device_id, int sym, int down)
{
  pthread_t thread;
  pthread_attr_t thread_attr;
  s_mouse_control* mc = cfg_get_mouse_control(current_mouse);

  switch (sym)
  {
    case SDLK_LCTRL:
      lctrl = down ? 1 : 0;
      break;
    case SDLK_RCTRL:
      rctrl = down ? 1 : 0;
      break;

    case SDLK_LSHIFT:
      lshift = down ? 1 : 0;
      break;
    case SDLK_RSHIFT:
      rshift = down ? 1 : 0;
      break;

    case SDLK_LALT:
      lalt = down ? 1 : 0;
      break;
    case SDLK_MODE:
      ralt = down ? 1 : 0;
      break;
  }

  switch (sym)
  {
    case SDLK_ESCAPE:
      /*
       * Ugly stuff to prevent emuclient to crash in case a test thread is running.
       * Threads will be removed in a future release.
       */
      if(current_cal != NONE)
      {
        current_cal = NONE;
        sleep(5);
      }
      break;
    case SDLK_F1:
      if (down && rctrl)
      {
        if (current_cal == NONE)
        {
          current_cal = MC;
          printf("mouse selection\n");
          display_calibration();
        }
        else
        {
          current_cal = NONE;
          if (cfgw_modify_file(config_file))
          {
            printf("error writting the config file %s\n", config_file);
          }
          printf("calibration done\n");
        }
      }
      break;
    case SDLK_F2:
      if (down && rctrl)
      {
        if (current_cal == CC)
        {
          current_cal = MC;
          printf("mouse selection\n");
        }
        else if (current_cal >= MC)
        {
          current_cal = CC;
          printf("config selection\n");
        }
      }
      break;
    case SDLK_F3:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating multiplier x\n");
          current_cal = MX;
        }
      }
      break;
    case SDLK_F4:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating x/y ratio\n");
          current_cal = MY;
        }
      }
      break;
    case SDLK_F5:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating dead zone x\n");
          current_cal = DZX;
          mc->merge_x[mc->index] = 1;
          mc->merge_y[mc->index] = 0;
          mc->change = 1;
        }
      }
      break;
    case SDLK_F6:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating dead zone y\n");
          current_cal = DZY;
          mc->merge_x[mc->index] = 0;
          mc->merge_y[mc->index] = 1;
          mc->change = 1;
        }
      }
      break;
    case SDLK_F7:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating dead zone shape\n");
          current_cal = DZS;
          mc->merge_x[mc->index] = 1;
          mc->merge_y[mc->index] = 1;
          mc->change = 1;
        }
      }
      break;
    case SDLK_F8:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating exponent x\n");
          current_cal = EX;
        }
      }
      break;
    case SDLK_F9:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          printf("calibrating exponent y\n");
          current_cal = EY;
        }
      }
      break;
    case SDLK_F10:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          if(current_cal != RD && current_cal != VEL)
          {
            current_cal = RD;
            pthread_attr_init(&thread_attr);
            pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&thread, &thread_attr, (void*) circle_test, NULL);
          }

          printf("adjusting circle test radius\n");
          current_cal = RD;
        }
      }
      break;
    case SDLK_F11:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          if(current_cal != RD && current_cal != VEL)
          {
            current_cal = VEL;
            pthread_attr_init(&thread_attr);
            pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&thread, &thread_attr, (void*) circle_test, NULL);
          }

          printf("adjusting circle test velocity\n");
          current_cal = VEL;
        }
      }
      break;
    case SDLK_F12:
      if (down && rctrl && current_cal != NONE)
      {
        if (current_conf >= 0 && current_mouse >= 0)
        {
          current_cal = TEST;
        }
        pthread_attr_init(&thread_attr);
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&thread, &thread_attr, (void*) auto_test, NULL);
      }
      break;
  }

  /*
   * Following code is not calibration code... it should be moved somewhere else!
   */
  if (lshift && rshift)
  {
    if (display)
    {
      display = 0;
    }
    else
    {
      display = 1;
    }
  }

  if (lalt && ralt)
  {
    sdl_grab_toggle();
  }
}

/*
 * Use the mouse wheel to calibrate the mouse.
 */
void cal_button(int which, int button)
{
  double ratio;
  s_mouse_control* mc = cfg_get_mouse_control(current_mouse);
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  switch (button)
  {
    case SDL_BUTTON_WHEELUP:
      switch (current_cal)
      {
        case MC:
          current_mouse += 1;
          if (!sdl_get_mouse_name(current_mouse))
          {
            current_mouse -= 1;
          }
          break;
        case CC:
          current_conf += 1;
          if (current_conf > MAX_CONFIGURATIONS - 1)
          {
            current_conf = MAX_CONFIGURATIONS - 1;
          }
          break;
        case MX:
          if (mcal->mx && mcal->my)
          {
            ratio = *mcal->my / *mcal->mx;
            *mcal->mx += DEFAULT_MULTIPLIER_STEP;
            *mcal->my = *mcal->mx * ratio;
          }
          break;
        case MY:
          if (mcal->mx && mcal->my)
          {
            ratio = *mcal->my / *mcal->mx;
            ratio += DEFAULT_MULTIPLIER_STEP;
            *mcal->my = *mcal->mx * ratio;
          }
          break;
        case DZX:
          if (mcal->dzx)
          {
            *mcal->dzx += 1;
            if (*mcal->dzx > mean_axis_value)
            {
              *mcal->dzx = mean_axis_value;
            }
            mc->merge_x[mc->index] = 1;
            mc->merge_y[mc->index] = 0;
            mc->change = 1;
          }
          break;
        case DZY:
          if (mcal->dzy)
          {
            *mcal->dzy += 1;
            if (*mcal->dzy > mean_axis_value)
            {
              *mcal->dzy = mean_axis_value;
            }
            mc->merge_x[mc->index] = 0;
            mc->merge_y[mc->index] = 1;
            mc->change = 1;
          }
          break;
        case DZS:
          if (mcal->dzs)
          {
            if (*mcal->dzs == E_SHAPE_CIRCLE)
            {
              *mcal->dzs = E_SHAPE_RECTANGLE;
            }
            else
            {
              *mcal->dzs = E_SHAPE_CIRCLE;
            }
            mc->merge_x[mc->index] = 1;
            mc->merge_y[mc->index] = 1;
            mc->change = 1;
          }
          break;
        case RD:
          mcal->rd += 1;
          break;
        case VEL:
          mcal->vel += 1;
          break;
        case EX:
          if (mcal->ex)
          {
            *mcal->ex += EXPONENT_STEP;
          }
          break;
        case EY:
          if (mcal->ey)
          {
            *mcal->ey += EXPONENT_STEP;
          }
          break;
        case TEST:
          test_time += 10;
          break;
        case NONE:
          break;
      }
      break;
    case SDL_BUTTON_WHEELDOWN:
      switch (current_cal)
      {
        case MC:
          if (current_mouse > 0)
          {
            current_mouse -= 1;
          }
          break;
        case CC:
          if (current_conf > 0)
          {
            current_conf -= 1;
          }
          break;
        case MX:
          if (mcal->mx && mcal->my)
          {
            ratio = *mcal->my / *mcal->mx;
            *mcal->mx -= DEFAULT_MULTIPLIER_STEP;
            *mcal->my = *mcal->mx * ratio;
          }
          break;
        case MY:
          if (mcal->mx && mcal->my)
          {
            ratio = *mcal->my / *mcal->mx;
            ratio -= DEFAULT_MULTIPLIER_STEP;
            *mcal->my = *mcal->mx * ratio;
          }
          break;
        case DZX:
          if (mcal->dzx)
          {
            *mcal->dzx -= 1;
            if (*mcal->dzx < 0)
            {
              *mcal->dzx = 0;
            }
            mc->merge_x[mc->index] = -1;
            mc->merge_y[mc->index] = 0;
            mc->change = 1;
          }
          break;
        case DZY:
          if (mcal->dzy)
          {
            *mcal->dzy -= 1;
            if (*mcal->dzy < 0)
            {
              *mcal->dzy = 0;
            }
            mc->merge_x[mc->index] = 0;
            mc->merge_y[mc->index] = -1;
            mc->change = 1;
          }
          break;
        case DZS:
          if (mcal->dzs)
          {
            if (*mcal->dzs == E_SHAPE_CIRCLE)
            {
              *mcal->dzs = E_SHAPE_RECTANGLE;
            }
            else
            {
              *mcal->dzs = E_SHAPE_CIRCLE;
            }
            mc->merge_x[mc->index] = -1;
            mc->merge_y[mc->index] = -1;
            mc->change = 1;
          }
          break;
        case RD:
          mcal->rd -= 1;
          if(mcal->rd < 1)
          {
            mcal->rd = 1;
          }
          break;
        case VEL:
          mcal->vel -= 1;
          if(mcal->vel < 1)
          {
            mcal->vel = 1;
          }
          break;
        case EX:
          if (mcal->ex)
          {
            *mcal->ex -= EXPONENT_STEP;
          }
          break;
        case EY:
          if (mcal->ey)
          {
            *mcal->ey -= EXPONENT_STEP;
          }
          break;
        case TEST:
          test_time -= 10;
          break;
        case NONE:
          break;
      }
      break;
  }

  display_calibration();
}

/*
 * If calibration is on, all mouse wheel events are skipped.
 */
int cal_skip_event(SDL_Event* event)
{
  return current_cal != NONE
      && event->type == SDL_MOUSEBUTTONDOWN
      && (event->button.button == SDL_BUTTON_WHEELDOWN || event->button.button == SDL_BUTTON_WHEELUP);
}
