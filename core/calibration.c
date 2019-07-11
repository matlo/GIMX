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

int current_mouse = 0;
int current_conf = 0;
e_current_cal current_cal = NONE;

static struct gcalibration calibration =
{
    .mouse             = &current_mouse,
    .config         = &current_conf,
    .cal_mode         = MODE_STATUS,
    .cal_step         = STEP_1,
    .is_edit_enabled     = 0,
    .cal_modes_max_step = {2,4,6,1}
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
    calibration.mouse_cal = cal_get_mouse(current_mouse, current_conf);
    calibration.current_cal = &current_cal;
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

  s_mouse_cal* mc = cal_get_mouse(current_mouse, current_conf);

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

  e_controller_type ctype = adapter_get(cal_get_controller(current_mouse))->ctype;

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
  mouse_evt.motion.which = current_mouse;
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
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);
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
  mouse_evt.motion.which = current_mouse;
  mouse_evt.type = GE_MOUSEMOTION;
  process_event(&mouse_evt);

  circle_step += step;
  circle_step = circle_step % STEPS;
}

void calibration_test()
{
  if(current_cal == RD || current_cal == VEL)
  {
    circle_test();
  }
  else if(current_cal == TEST)
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
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  if (current_cal != NONE)
  {
    ginfo(_("calibrating mouse %s (%d)\n"), ginput_mouse_name(current_mouse), ginput_mouse_virtual_id(current_mouse));
    ginfo(_("calibrating conf %d\n"), current_conf + 1);
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

void cal_previous_step()
{
    // if not first step
    if (calibration.cal_step != STEP_1)
    {
        calibration.cal_step--;
        int last_cal = find_last_cal_index(calibration.cal_step);
        *calibration.current_cal = steps_cal_map[calibration.cal_step][last_cal];
    }
    else // If first then return to menu
    {
        calibration.cal_mode = MODE_STATUS;
        *calibration.current_cal = NONE;
    }
}

void cal_next_step()
{
    if (calibration.cal_step != (unsigned int) calibration.cal_modes_max_step[calibration.cal_mode] -1)
    {
        calibration.cal_step++;
        *calibration.current_cal = steps_cal_map[calibration.cal_step][0];
    }
}

void cal_handle_params_change(int sym, int down)
{
    if (down)
    {
        //Next/Previous step
        if (sym == GE_KEY_F3)
        {
            int current_index = find_cal_index(calibration.cal_step, *calibration.current_cal);
            // How many cals are in that step
            int max_index = find_last_cal_index(calibration.cal_step);

            // If selected cal isn't the last in this step
            if(current_index != max_index)
                *calibration.current_cal = steps_cal_map[calibration.cal_step][current_index + 1];
            else
                cal_next_step(); // Move to next step
        }

        if (sym == GE_KEY_F2)
        {
            int current_index = find_cal_index(calibration.cal_step, *calibration.current_cal);

            // If selected cal isn't first in this step
            if(current_index != 0)
                *calibration.current_cal = steps_cal_map[calibration.cal_step][current_index - 1];
            else
                cal_previous_step(); // Move to previous step
        }
    }
}

/*
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
                calibration.cal_mode = MODE_STATUS;
                calibration.is_edit_enabled = 0;
                *calibration.current_cal = NONE;
            }
        }
    }

}

/*
 * Use keys to calibrate the mouse.
 */
void cal_key(int sym, int down)
{
  //s_mouse_control* mc = cfg_get_mouse_control(current_mouse);

  update_cal_data();

  e_current_cal prev = current_cal;
  e_cal_modes prev_mode = calibration.cal_mode;
  e_cal_steps prev_step = calibration.cal_step;
  int prev_edit = calibration.is_edit_enabled;

  e_cal_modes curr_mode = calibration.cal_mode;
  e_cal_steps curr_step = calibration.cal_step;

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
              *calibration.current_cal = NONE;
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
                        calibration.cal_mode = MODE_BASIC;
                    else if (sym == GE_KEY_F2)
                        calibration.cal_mode = MODE_ADVANCED;
                    else if (sym == GE_KEY_F3)
                        calibration.cal_mode = MODE_EXPERT;

                    // If mode changed
                    if (prev_mode != calibration.cal_mode)
                        *calibration.current_cal = steps_cal_map[STEP_1][0]; // select first parameter in step_1 by default

                }
            }
        }
        if (curr_mode != MODE_STATUS) {

            // Next/Previous handling step
            cal_handle_steps_change(sym, down);

            // Menu return handling
            cal_handle_menu_return(sym, down);

            cal_handle_params_change(sym, down);

        }
        break;
    case STEP_2:

        // Next/Previous handling step
        cal_handle_steps_change(sym, down);

        // Menu return handling
        cal_handle_menu_return(sym, down);

        cal_handle_params_change(sym, down);

        break;

    case STEP_3:
        // Next/Previous handling step
        cal_handle_steps_change(sym, down);

        // Menu return handling
        cal_handle_menu_return(sym, down);

        cal_handle_params_change(sym, down);
        break;

        // Suppress compiler warning about unused enum values
    default:
        break;
    }

  /*switch (sym)
  {
    case GE_KEY_ESC:
      if(current_cal != NONE)
      {
        current_cal = NONE;
      }
      break;
    case GE_KEY_F1:
      if (down)
      {
        if(rctrl || lctrl)
        {
          if (current_cal == NONE)
          {
            if(ginput_get_mk_mode() == GE_MK_MODE_MULTIPLE_INPUTS)
            {
              current_cal = MC;
              ginfo(_("mouse selection\n"));
            }
            else
            {
              current_cal = CC;
              ginfo(_("profile selection\n"));
            }
          }
          else
          {
            current_cal = NONE;
            if (cfgw_modify_file(gimx_params.config_file))
            {
              gwarn(_("error writting the config file %s\n"), gimx_params.config_file);
            }
            ginfo(_("calibration done\n"));
          }
        }
        else if(current_cal != NONE)
        {
          if(ginput_get_mk_mode() == GE_MK_MODE_MULTIPLE_INPUTS)
          {
            current_cal = MC;
            ginfo(_("mouse selection\n"));
          }
        }
      }
      break;
    case GE_KEY_F2:
      if (down && current_cal != NONE)
      {
            if (calibration.cal_mode == MODE_BASIC)
                calibration.cal_mode = MODE_ADVANCED;
            else if (calibration.cal_mode == MODE_ADVANCED)
                calibration.cal_mode = MODE_STATUS;
            else if (calibration.cal_mode == MODE_STATUS)
                calibration.cal_mode = MODE_BASIC;

            current_cal = CC;
            ginfo(_("profile selection\n"));
      }
      break;
    case GE_KEY_F9:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating sensitivity\n"));
        current_cal = MX;
      }
      break;
    case GE_KEY_F12:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating x/y ratio\n"));
        current_cal = MY;
      }
      break;
    case GE_KEY_F3:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating dead zone x\n"));
        current_cal = DZX;
        mc->merge[mc->index].x = 1;
        mc->merge[mc->index].y = 0;
        mc->change = 1;
      }
      break;
    case GE_KEY_F4:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating dead zone y\n"));
        current_cal = DZY;
        mc->merge[mc->index].x = 0;
        mc->merge[mc->index].y = 1;
        mc->change = 1;
      }
      break;
    case GE_KEY_F5:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating dead zone shape\n"));
        current_cal = DZS;
        mc->merge[mc->index].x = 1;
        mc->merge[mc->index].y = 1;
        mc->change = 1;
      }
      break;
    case GE_KEY_F7:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating acceleration x\n"));
        current_cal = EX;
      }
      break;
    case GE_KEY_F8:
      if (down && current_cal != NONE)
      {
        ginfo(_("calibrating acceleration y\n"));
        current_cal = EY;
      }
      break;
    case GE_KEY_F10:
      if (down && current_cal != NONE)
      {
        if(current_cal != RD && current_cal != VEL)
        {
          circle_step = 0;
        }
        ginfo(_("adjusting circle test radius\n"));
        current_cal = RD;
      }
      break;
    case GE_KEY_F11:
      if (down && current_cal != NONE)
      {
        if(current_cal != RD && current_cal != VEL)
        {
          circle_step = 0;
        }
        ginfo(_("adjusting circle test velocity\n"));
        current_cal = VEL;
      }
      break;
    case GE_KEY_F6:
      if (down && current_cal != NONE)
      {
        if(current_cal != TEST)
        {
          distance = 0.1; //0.1 inches
          dots = 0;
          direction = 1;
          step = 1;
        }
        ginfo(_("translation test started\n"));
        current_cal = TEST;
      }
      break;
  }*/

  // TODO, update
  if(prev != current_cal || prev_mode != calibration.cal_mode || prev_step != calibration.cal_step || prev_edit != calibration.is_edit_enabled)
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
  s_mouse_control* mc = cfg_get_mouse_control(current_mouse);
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  //update_cal_data();

  int controller = cal_get_controller(current_mouse);
  e_controller_type ctype = adapter_get(controller)->ctype;

  if(ctype == C_TYPE_NONE)
  {
    return;
  }

  switch (button)
  {
    case GE_BTN_WHEELUP:
      switch (current_cal)
      {
        case MC:
          current_mouse += 1;
          if (!ginput_mouse_name(current_mouse))
          {
            current_mouse -= 1;
          }
          break;
        case CC:
          if (current_conf < MAX_PROFILES - 1)
          {
            current_conf += 1;
            cfg_set_profile(controller, current_conf);
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
      if(current_cal != NONE)
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
            cfg_set_profile(controller, current_conf);
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
      if(current_cal != NONE)
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
  return current_cal != NONE
      && event->type == GE_MOUSEBUTTONDOWN
      && (event->button.button == GE_BTN_WHEELUP || event->button.button == GE_BTN_WHEELDOWN);
}
