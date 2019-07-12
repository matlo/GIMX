/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "calibration.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "config_writter.h"
#include "gimx.h"
#include <controller.h>
#include <gimxinput/include/ginput.h>
#include "display.h"
#include "mainloop.h"

#define DEFAULT_MULTIPLIER_STEP 0.01
#define EXPONENT_STEP 0.01
#define DURATION 500000 //1s
#define STEPS 720

static struct gcalibration calibration =
{
    .mouse = 0,
    .profile = 0,
    .mode = MODE_STATUS,
    .step = STEP_1,
    .cal_modes_max_step = {2, 4, 6, 1}
};

unsigned int steps_cal_map[5][GSTEPS_MAX] =
{
    {CC},
    {MX},
    {DZX,DZY},
    {DZX,EX,EY,TEST}, // Not used for now
    {DZX,EX,EY,TEST}
};

static int lctrl = 0;
static int rctrl = 0;

static const double pi = 3.14159265;

static int test_time = 1000;


/*
 * Used to calibrate mouse controls.
 */
s_mouse_cal mouse_cal[MAX_DEVICES][MAX_PROFILES] = {};

int mouse_controller[MAX_DEVICES];

inline int cal_get_controller(int mouse)
{
  return mouse_controller[mouse];
}

void cal_set_controller(int mouse, int controller)
{
  mouse_controller[mouse] = controller;
}

void update_cal_data()
{
    calibration.mouse_data = cal_get_mouse(calibration.mouse, calibration.profile);
}

void cal_init()
{
  memset(mouse_cal, 0x00, sizeof(mouse_cal));

  update_cal_data();
}

inline s_mouse_cal* cal_get_mouse(int mouse, int conf)
{
  return &(mouse_cal[mouse][conf]);
}

inline void cal_set_mouse(s_config_entry* entry)
{
  mouse_cal[entry->device.id][entry->profile_id].options = entry->params.mouse_options;
}

static double distance = 0.1; //0.1 inches
static int dots = 0;
static int direction = 1;
static int step = 1;

static int delay = 0;

static void translation_test()
{
  int dpi;
  int dz;
  double mul;
  double exp;

  GE_Event mouse_evt = { };

  s_mouse_cal* mc = cal_get_mouse(calibration.mouse, calibration.profile);

  if(!mc->dzx || !mc->mx || !mc->ex)
  {
    return;
  }

  dpi = mc->dpi;
  dz = *mc->dzx;
  mul = *mc->mx;
  exp = *mc->ex;

  if (dpi <= 0)
  {
    return;
  }

  e_controller_type ctype = adapter_get(cal_get_controller(calibration.mouse))->ctype;

  if(ctype == C_TYPE_NONE)
  {
    return;
  }

  if (dots <= 0)
  {
    dots = distance * dpi;
  }

  if(delay > 0)
  {
    delay--;
    return;
  }

  mouse_evt.motion.xrel = direction * step;
  mouse_evt.motion.which = calibration.mouse;
  mouse_evt.type = GE_MOUSEMOTION;
  process_event(&mouse_evt);

  dots -= step;

  if (dots <= 0)
  {
    delay = DURATION / gimx_params.refresh_period;
    step *= 2;
    direction *= -1;
    if (direction > 0)
    {
      if ((dz - mul + mul * pow(step * 2 * gimx_params.frequency_scale, exp)) * controller_get_axis_scale(ctype, rel_axis_2) > controller_get_mean_unsigned(ctype, rel_axis_2))
      {
        step = 1;
        distance = 0.1;
      }
      else
      {
        distance = distance * 3;
      }
    }
  }
}

static int circle_step = 0;

static void circle_test()
{
  GE_Event mouse_evt = { };
  s_mouse_cal* mcal = cal_get_mouse(calibration.mouse, calibration.profile);
  int step;

  if(!mcal->ex)
  {
    return;
  }

  int dpi = mcal->dpi;

  if (dpi <= 0)
  {
    dpi = 5700;
  }

  step = mcal->vel;
  mouse_evt.motion.xrel = round(mcal->rd * pow((double) dpi / 5700, *mcal->ex) * (cos(circle_step * 2 * pi / STEPS) - cos((circle_step - step) * 2 * pi / STEPS)));
  mouse_evt.motion.yrel = round(mcal->rd * pow((double) dpi / 5700, *mcal->ex) * (sin(circle_step * 2 * pi / STEPS) - sin((circle_step - step) * 2 * pi / STEPS)));
  mouse_evt.motion.which = calibration.mouse;
  mouse_evt.type = GE_MOUSEMOTION;
  process_event(&mouse_evt);

  circle_step += step;
  circle_step = circle_step % STEPS;
}

void calibration_test()
{
  if(calibration.current == RD || calibration.current == VEL)
  {
    circle_test();
  }
  else if(calibration.current == TEST)
  {
    translation_test();
  }
}

/*
 * Initial status screen for calibration
 */
void cal_status()
{
    // Values are already set, just run the display function
    display_calibration(&calibration);
}

/*
 * Display calibration info.
 */
static void cal_display()
{
  s_mouse_cal* mcal = cal_get_mouse(calibration.mouse, calibration.profile);

  if (calibration.current != NONE)
  {
    ginfo(_("calibrating mouse %s (%d)\n"), ginput_mouse_name(calibration.mouse), ginput_mouse_virtual_id(calibration.mouse));
    ginfo(_("calibrating conf %d\n"), calibration.profile + 1);
    ginfo(_("sensibility:"));
    if (mcal->mx)
    {
      ginfo(" %.2f\n", *mcal->mx);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("x/y ratio:"));
    if (mcal->mx && mcal->my)
    {
      ginfo(" %.2f\n", *mcal->my / *mcal->mx);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("dead zone x:"));
    if (mcal->dzx)
    {
      ginfo(" %d\n", *mcal->dzx);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("dead zone y:"));
    if (mcal->dzy)
    {
      ginfo(" %d\n", *mcal->dzy);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("shape:"));
    if (mcal->dzs)
    {
      if (*mcal->dzs == E_SHAPE_CIRCLE)
      {
        ginfo(_(" Circle\n"));
      }
      else
      {
        ginfo(_(" Rectangle\n"));
      }
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("acceleration x:"));
    if (mcal->ex)
    {
      ginfo(" %.2f\n", *mcal->ex);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("acceleration y:"));
    if (mcal->ey)
    {
      ginfo(" %.2f\n", *mcal->ey);
    }
    else
    {
      ginfo(_(" NA\n"));
    }
    ginfo(_("radius: %d\n"), mcal->rd);
    ginfo(_("velocity: %d\n"), mcal->vel);
    ginfo(_("time: %d\n"), test_time);
  }
}

int find_last_cal_index(e_cal_steps step)
{
   int i;
   for (i=0; i<GSTEPS_MAX; i++)
   {
     if (steps_cal_map[step][i] == 0)
     {
        return(i-1);  /* it was found */
     }
   }
   return(-1);  /* if it was not found */
}

int find_cal_index(e_cal_steps step, e_current_cal curr_cal)
{
    int i;
    for (i=0; i<GSTEPS_MAX; i++)
    {
      if (steps_cal_map[step][i] == curr_cal)
      {
         return(i);  /* it was found */
      }
    }
    return(-1);  /* if it was not found */
}

void cal_return_to_menu()
{
    calibration.mode = MODE_STATUS;
    calibration.step = STEP_1;
    calibration.current = NONE;
}

void cal_previous_step()
{
    // if not first step
    if (calibration.step != STEP_1)
    {
        calibration.step--;
        int last_cal = find_last_cal_index(calibration.step);
        calibration.current = steps_cal_map[calibration.step][last_cal];
    }
    else // If first then return to menu
    {
        cal_return_to_menu();
    }
}

void cal_next_step()
{
    if (calibration.step != (unsigned int) calibration.cal_modes_max_step[calibration.mode] -1)
    {
        calibration.step++;
        calibration.current = steps_cal_map[calibration.step][0];
    }
    else // If last then return to menu
    {
        cal_return_to_menu();
    }
}



void cal_handle_params_change(int sym, int down)
{
    if (down)
    {
        //Next/Previous step
        if (sym == GE_KEY_F2)
        {
            int current_index = find_cal_index(calibration.step, calibration.current);
            // How many cals are in that step
            int max_index = find_last_cal_index(calibration.step);

            // If selected cal isn't the last in this step
            if(current_index != max_index)
                calibration.current = steps_cal_map[calibration.step][current_index + 1];
            else
                cal_next_step(); // Move to next step
        }

        if (sym == GE_KEY_F1)
        {
            int current_index = find_cal_index(calibration.step, calibration.current);

            // If selected cal isn't first in this step
            if(current_index != 0)
                calibration.current = steps_cal_map[calibration.step][current_index - 1];
            else
                cal_previous_step(); // Move to previous step
        }
    }
}

/*
 * TURNED OFF
 * Handles steps change controls in every mode except MODE_STATUS
 */
void cal_handle_steps_change(int sym, int down)
{
    if(down)
    {
        //Next/Previous step
        if (sym == GE_KEY_F4)
        {
            cal_next_step();
        }
        else if (sym == GE_KEY_F1)
        {
            cal_previous_step();
        }
    }
}

/*
 * Handles returning to menu on Ctrl + F5. Not sure if this is useful now but it was before I did steps handling
 */
void cal_handle_menu_return(int sym, int down)
{

    if (down) {
        // Return to menu
        if (rctrl || lctrl) {
            if (sym == GE_KEY_F5) {
                calibration.mode = MODE_STATUS;
                calibration.current = NONE;
            }
        }
    }

}

/*
 * Use keys to calibrate the mouse.
 */
void cal_key(int sym, int down)
{
  //s_mouse_control* mc = cfg_get_mouse_control(calibration.mouse);

  update_cal_data();

  e_current_cal prev = calibration.current;
  e_cal_modes prev_mode = calibration.mode;
  e_cal_steps prev_step = calibration.step;

  e_cal_modes curr_mode = calibration.mode;
  e_cal_steps curr_step = calibration.step;

  switch (sym)
  {
    case GE_KEY_LEFTCTRL:
      lctrl = down ? 1 : 0;
      break;
    case GE_KEY_RIGHTCTRL:
      rctrl = down ? 1 : 0;
      break;
  }

  /*
   * On/Off for Edit mode
   * it's always possible to activate it except in MODE_STATUS so it's outside main cal input switch
   * while not in edit mode it's not possible to edit values, exiting edit mode sets current_cal to NONE
   */
  /*if(down && sym == GE_KEY_F1 && calibration.cal_mode != MODE_STATUS)
  {
      if (!rctrl && !lctrl)
      {
          if(calibration.isEditEnabled)
          {
              calibration.isEditEnabled = false;
              calibration.current_cal = NONE;
          }
          else
              calibration.isEditEnabled = true;
      }
  }*/

  /*
   * Main data loop that controls how input is managed and what actions to do
   */
    switch (curr_step) {
    case STEP_1:
        if (curr_mode == MODE_STATUS) {
            if (down) {
                // Initial status screen controls
                if (rctrl || lctrl) {
                    if (sym == GE_KEY_F1)
                        calibration.mode = MODE_BASIC;
                    else if (sym == GE_KEY_F2)
                        calibration.mode = MODE_ADVANCED;
                    else if (sym == GE_KEY_F3)
                        calibration.mode = MODE_EXPERT;

                    // If mode changed
                    if (prev_mode != calibration.mode)
                        calibration.current = steps_cal_map[STEP_1][0]; // select first parameter in step_1 by default

                }
            }
        }
        if (curr_mode != MODE_STATUS) {

            // Next/Previous handling step
            //cal_handle_steps_change(sym, down);

            // Menu return handling
            cal_handle_menu_return(sym, down);

            cal_handle_params_change(sym, down);

        }
        break;
    case STEP_2:

        // Next/Previous handling step
        //cal_handle_steps_change(sym, down);

        // Menu return handling
        cal_handle_menu_return(sym, down);

        cal_handle_params_change(sym, down);

        break;

    case STEP_3:
        // Next/Previous handling step
        //cal_handle_steps_change(sym, down);

        // Menu return handling
        cal_handle_menu_return(sym, down);

        cal_handle_params_change(sym, down);
        break;

        // Suppress compiler warning about unused enum values
    default:
        break;
    }

  // TODO, update
  if(prev != calibration.current || prev_mode != calibration.mode || prev_step != calibration.step)
  {
    if(gimx_params.curses)
    {
      display_calibration(&calibration);
    }
    else
    {
      cal_display();
    }
  }
}

#define DEADZONE_MAX(AXIS) \
    (controller_get_mean_unsigned(ctype, AXIS) / controller_get_axis_scale(ctype, AXIS) / 2)

/*
 * Use the mouse wheel to calibrate the mouse.
 */
void cal_button(int button)
{
  update_cal_data();

  double ratio;
  s_mouse_control* mc = cfg_get_mouse_control(calibration.mouse);
  s_mouse_cal* mcal = cal_get_mouse(calibration.mouse, calibration.profile);

  //update_cal_data();

  int controller = cal_get_controller(calibration.mouse);
  e_controller_type ctype = adapter_get(controller)->ctype;

  if(ctype == C_TYPE_NONE)
  {
    return;
  }

  switch (button)
  {
    case GE_BTN_WHEELUP:
      switch (calibration.current)
      {
        case MC:
          calibration.mouse += 1;
          if (!ginput_mouse_name(calibration.mouse))
          {
            calibration.mouse -= 1;
          }
          break;
        case CC:
          if (calibration.profile < MAX_PROFILES - 1)
          {
            calibration.profile += 1;
            cfg_set_profile(controller, calibration.profile);
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
          if (mcal->dzx && *mcal->dzx < DEADZONE_MAX(rel_axis_rstick_x))
          {
            *mcal->dzx += 1;
            mc->merge[mc->index].x = 1;
            mc->merge[mc->index].y = 0;
            mc->change = 1;
          }
          break;
        case DZY:
          if (mcal->dzy && *mcal->dzy < DEADZONE_MAX(rel_axis_rstick_y))
          {
            *mcal->dzy += 1;
            mc->merge[mc->index].x = 0;
            mc->merge[mc->index].y = 1;
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
            mc->merge[mc->index].x = 1;
            mc->merge[mc->index].y = 1;
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
      if(calibration.current != NONE)
      {
        if(gimx_params.curses)
        {
            display_calibration(&calibration);
        }
        else
        {
          cal_display();
        }
      }
      break;
    case GE_BTN_WHEELDOWN:
      switch (calibration.current)
      {
        case MC:
          if (calibration.mouse > 0)
          {
            calibration.mouse -= 1;
          }
          break;
        case CC:
          if (calibration.profile > 0)
          {
            calibration.profile -= 1;
            cfg_set_profile(controller, calibration.profile);
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
          if (mcal->dzx && *mcal->dzx > - DEADZONE_MAX(rel_axis_rstick_x))
          {
            *mcal->dzx -= 1;
            mc->merge[mc->index].x = -1;
            mc->merge[mc->index].y = 0;
            mc->change = 1;
          }
          break;
        case DZY:
          if (mcal->dzy && *mcal->dzy > - DEADZONE_MAX(rel_axis_rstick_y))
          {
            *mcal->dzy -= 1;
            mc->merge[mc->index].x = 0;
            mc->merge[mc->index].y = -1;
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
            mc->merge[mc->index].x = -1;
            mc->merge[mc->index].y = -1;
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
      if(calibration.current != NONE)
      {
        if(gimx_params.curses)
        {
            display_calibration(&calibration);
        }
        else
        {
          cal_display();
        }
      }
      break;
    default:
      break;
  }
}

/*
 * If calibration is on, all mouse wheel events are skipped.
 */
int cal_skip_event(GE_Event* event)
{
  return calibration.current != NONE
      && event->type == GE_MOUSEBUTTONDOWN
      && (event->button.button == GE_BTN_WHEELUP || event->button.button == GE_BTN_WHEELDOWN);
}

int calibration_get_mouse() {
    return calibration.mouse;
}

void calibration_set_mouse(int mouse) {
    calibration.mouse = mouse;
}

e_current_cal calibration_get_current() {
    return calibration.current;
}
