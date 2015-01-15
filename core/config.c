/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <GE.h>
#include "calibration.h"
#include "gimx.h"
#include <adapter.h>

/*
 * This tells what's the current config of each controller.
 */
static int current_config[MAX_CONTROLLERS];

/*
 * This tells what's the next config of each controller.
 */
static int next_config[MAX_CONTROLLERS];
static int config_delay[MAX_CONTROLLERS];

/*
 * This tells what's the previous config of each controller.
 * Hackish way to switch back to previous config.
 */
static int previous_config[MAX_CONTROLLERS];

/*
 * This lists config triggers for all controllers.
 */
static s_trigger triggers[MAX_CONTROLLERS][MAX_CONFIGURATIONS];

/*
 * This lists controller stick intensity modifiers.
 */
static s_intensity axis_intensity[MAX_CONTROLLERS][MAX_CONFIGURATIONS][AXIS_MAX];

/*
 * This lists controls of each controller configuration for all keyboards.
 */
static s_mapper* keyboard_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

/*
 * This lists controls of each controller configuration for all mice.
 */
static s_mapper* mouse_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

static s_mapper* mouse_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

/*
 * Used to tweak mouse controls.
 */
static s_mouse_control mouse_control[MAX_DEVICES] = {};

/*
 * This lists controls of each controller configuration for all joysticks.
 */
static s_mapper* joystick_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];
static s_mapper* joystick_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

inline s_mapper** cfg_get_joystick_axes(int device, int controller, int config)
{
  return &(joystick_axes[device][controller][config]);
}

inline s_mapper** cfg_get_joystick_buttons(int device, int controller, int config)
{
  return &(joystick_buttons[device][controller][config]);
}

inline s_mapper** cfg_get_mouse_axes(int device, int controller, int config)
{
  return &(mouse_axes[device][controller][config]);
}

inline s_mapper** cfg_get_mouse_buttons(int device, int controller, int config)
{
  return &(mouse_buttons[device][controller][config]);
}

inline s_mapper** cfg_get_keyboard_buttons(int device, int controller, int config)
{
  return &(keyboard_buttons[device][controller][config]);
}

inline s_trigger* cfg_get_trigger(int controller, int config)
{
  return &(triggers[controller][config]);
}

inline s_intensity* cfg_get_axis_intensity(int controller, int config, int axis)
{
  return &(axis_intensity[controller][config][axis]);
}

static struct
{
  unsigned char nb;
  unsigned long weak;
  unsigned long strong;
  unsigned char falling; //indicates that weak and strong were null in the last event
  unsigned char off;
} joystick_rumble[MAX_DEVICES] = {};

inline void cfg_process_rumble_event(GE_Event* event)
{
  joystick_rumble[event->jrumble.which].weak += event->jrumble.weak;
  joystick_rumble[event->jrumble.which].strong += event->jrumble.strong;
  joystick_rumble[event->jrumble.which].nb++;
  joystick_rumble[event->jrumble.which].falling =
      (!event->jrumble.weak && !event->jrumble.strong)
      && !joystick_rumble[event->jrumble.which].off;
}

void cfg_process_rumble()
{
  int i;
  for (i = 0; i < MAX_DEVICES; ++i)
  {
    unsigned char nb = joystick_rumble[i].nb;

    if(nb)
    {
      unsigned short weak = joystick_rumble[i].weak / nb;
      unsigned short strong = joystick_rumble[i].strong / nb;

      joystick_rumble[i].off = !weak && !strong;

      if(!joystick_rumble[i].off || joystick_rumble[i].falling)
      {
        GE_JoystickSetRumble(i, weak, strong);

        joystick_rumble[i].falling = 0;
      }

      joystick_rumble[i].nb = 0;
      joystick_rumble[i].weak = 0;
      joystick_rumble[i].strong = 0;
    }
  }
}

int cfg_is_joystick_used(int id)
{
  int j, k;
  int used = 0;
  for(j=0; j<MAX_CONTROLLERS && !used; ++j)
  {
    for(k=0; k<MAX_CONFIGURATIONS && !used; ++k)
    {
      if((joystick_buttons[id][j][k] && joystick_buttons[id][j][k]->nb_mappers)
          || (joystick_axes[id][j][k] && joystick_axes[id][j][k]->nb_mappers))
      {
        used = 1;
      }
    }
  }
  return used;
}

inline s_mouse_control* cfg_get_mouse_control(int id)
{
  if(id >= 0)
  {
    return mouse_control + id;
  }
  return NULL;
}

void cfg_process_motion_event(GE_Event* event)
{
  s_mouse_control* mc = cfg_get_mouse_control(GE_GetDeviceId(event));
  if(mc)
  {
    mc->merge_x[mc->index] += event->motion.xrel;
    mc->merge_y[mc->index] += event->motion.yrel;
    mc->change = 1;
  }
}

void cfg_process_motion()
{
  int i, j, k;
  int weight;
  int divider;
  s_mouse_control* mc;
  s_mouse_cal* mcal;
  GE_Event mouse_evt = { };
  /*
   * Process a single (merged) motion event for each mouse.
   */
  for (i = 0; i < MAX_DEVICES; ++i)
  {
    mc = cfg_get_mouse_control(i);
    mcal = cal_get_mouse(i, current_config[cal_get_controller(i)]);
    if(!mc->change && mcal->mode == E_MOUSE_MODE_DRIVING)
    {
      //no auto-center
      continue;
    }
    if (mc->changed || mc->change)
    {
      if (gimx_params.subpositions)
      {
        /*
         * Add the residual motion vector from the last iteration.
         */
        mc->merge_x[mc->index] += mc->residue_x;
        mc->merge_y[mc->index] += mc->residue_y;
        /*
         * If no motion was received this iteration, the residual motion vector from the last iteration is reset.
         */
        if (!mc->change)
        {
          mc->residue_x = 0;
          mc->residue_y = 0;
        }
      }

      mc->x = 0;
      weight = 1;
      divider = 0;
      for(j=0; j<mcal->buffer_size; ++j)
      {
        k = mc->index - j;
        if (k < 0)
        {
          k += MAX_BUFFERSIZE;
        }
        mc->x += (mc->merge_x[k]*weight);
        divider += weight;
        weight *= mcal->filter;
      }
      mc->x /= divider;

      mc->y = 0;
      weight = 1;
      divider = 0;
      for(j=0; j<mcal->buffer_size; ++j)
      {
        k = mc->index - j;
        if (k < 0)
        {
          k += MAX_BUFFERSIZE;
        }
        mc->y += (mc->merge_y[k]*weight);
        divider += weight;
        weight *= mcal->filter;
      }
      mc->y /= divider;

      mouse_evt.motion.which = i;
      mouse_evt.type = GE_MOUSEMOTION;
      cfg_process_event(&mouse_evt);
    }
    mc->index++;
    mc->index %= MAX_BUFFERSIZE;
    mc->merge_x[mc->index] = 0;
    mc->merge_y[mc->index] = 0;
    mc->changed = mc->change;
    mc->change = 0;
    if (i == current_mouse && (current_cal == DZX || current_cal == DZY || current_cal == DZS))
    {
      mc->changed = 0;
    }
  }
}

/*
 * This updates the stick according to the intensity shape.
 * It only does something for axes in {rel_axis_lstick_x, rel_axis_lstick_y, rel_axis_rstick_x, rel_axis_rstick_y}
 */
static void update_stick(int c_id, int axis)
{
  if(axis <= rel_axis_rstick_y)
  {
    if(axis & 1)
    {
      axis = axis - 1;
    }
  }
  
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];
  double value = intensity->value;

  if(intensity->down_button == -1 && intensity->up_button == -1)
  {
    return;
  }

  if (axis <= rel_axis_rstick_y && intensity->shape == E_SHAPE_CIRCLE)
  {
    if (adapter_get(c_id)->axis[axis] && adapter_get(c_id)->axis[axis+1])
    {
      value = sqrt(value * value / 2);
    }
  }

  if (adapter_get(c_id)->axis[axis] > 0)
  {
    adapter_get(c_id)->axis[axis] = round(value);
    adapter_get(c_id)->send_command = 1;
  }
  else if (adapter_get(c_id)->axis[axis] < 0)
  {
    adapter_get(c_id)->axis[axis] = -round(value);
    adapter_get(c_id)->send_command = 1;
  }
  if (axis <= rel_axis_rstick_y)
  {
    if (adapter_get(c_id)->axis[axis+1] > 0)
    {
      adapter_get(c_id)->axis[axis+1] = round(value);
      adapter_get(c_id)->send_command = 1;
    }
    else if (adapter_get(c_id)->axis[axis+1] < 0)
    {
      adapter_get(c_id)->axis[axis+1] = -round(value);
      adapter_get(c_id)->send_command = 1;
    }
  }
}

/*
 * Update an axis intensity.
 */
static int update_intensity(int device_type, int device_id, int button, int c_id, int axis)
{
  int ret = 0;
  
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];

  if (intensity->device_up_type == device_type && device_id == intensity->device_up_id && button == intensity->up_button)
  {
    intensity->value += intensity->step;
    if (intensity->value > intensity->max_value)
    {
      if (intensity->down_button != -1)
      {
        intensity->value = intensity->max_value;
      }
      else
      {
        intensity->value = (double) intensity->dead_zone + intensity->step;
      }
    }
    ret = 1;
  }
  else if (intensity->device_down_type == device_type && device_id == intensity->device_down_id && button == intensity->down_button)
  {
    intensity->value -= intensity->step;
    if (intensity->value < intensity->dead_zone + intensity->step)
    {
      if (intensity->up_button != -1)
      {
        intensity->value = (double) intensity->dead_zone + intensity->step;
      }
      else
      {
        intensity->value = intensity->max_value;
      }
    }
    ret = 1;
  }

  return ret;
}

/*
 * Check if axis intensities need to be updated.
 */
void cfg_intensity_lookup(GE_Event* e)
{
  int c_id, a_id;
  int device_type;
  int button_id;
  unsigned int device_id = GE_GetDeviceId(e);

  switch( e->type )
  {
    case GE_JOYBUTTONDOWN:
      device_type = E_DEVICE_TYPE_JOYSTICK;
      button_id = e->jbutton.button;
      break;
    case GE_KEYDOWN:
      device_type = E_DEVICE_TYPE_KEYBOARD;
      button_id = e->key.keysym;
      break;
    case GE_MOUSEBUTTONDOWN:
      device_type = E_DEVICE_TYPE_MOUSE;
      button_id = e->button.button;
      break;
    default:
      return;
  }

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    for(a_id=0; a_id<AXIS_MAX; ++a_id)
    {
      if(update_intensity(device_type, device_id, button_id, c_id, a_id))
      {
        update_stick(c_id, a_id);
        gprintf(_("controller %d configuration %d axis %s intensity: %.0f\n"), c_id, current_config[c_id], control_get_name(adapter_get(c_id)->type, a_id), axis_intensity[c_id][current_config[c_id]][a_id].value);
      }
    }
  }
}

/*
 * Initialize next_config and prev_config tables.
 */
void cfg_trigger_init()
{
  int i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    next_config[i] = -1;
    previous_config[i] = -1;
  }
}

/*
 * Check if current configurations of controllers need to be updated.
 */
void cfg_trigger_lookup(GE_Event* e)
{
  int i, j;
  int device_type;
  int button;
  int up = 0;
  int selected;
  int current;
  unsigned int device_id = GE_GetDeviceId(e);

  switch( e->type )
  {
    case GE_JOYBUTTONUP:
      up = 1;
      device_type = E_DEVICE_TYPE_JOYSTICK;
      button = e->jbutton.button;
      break;
    case GE_JOYBUTTONDOWN:
      device_type = E_DEVICE_TYPE_JOYSTICK;
      button = e->jbutton.button;
      break;
    case GE_KEYUP:
      up = 1;
      device_type = E_DEVICE_TYPE_KEYBOARD;
      button = e->key.keysym;
      break;
    case GE_KEYDOWN:
      device_type = E_DEVICE_TYPE_KEYBOARD;
      button = e->key.keysym;
      break;
    case GE_MOUSEBUTTONUP:
      up = 1;
      device_type = E_DEVICE_TYPE_MOUSE;
      button = e->button.button;
      break;
    case GE_MOUSEBUTTONDOWN:
      device_type = E_DEVICE_TYPE_MOUSE;
      button = e->button.button;
      break;
    default:
      return;
  }

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    selected = -1;
    if(next_config[i] > -1)
    {
      current = next_config[i];
    }
    else
    {
      current = current_config[i];
    }
    for(j=0; j<MAX_CONFIGURATIONS; ++j)
    {
      if (triggers[i][j].device_type != device_type || device_id
          != triggers[i][j].device_id)
      {
        continue;
      }
      if (button == triggers[i][j].button)
      {
        if (!up)
        {
          if(current == j)
          {
            continue;
          }
          if(selected < 0)
          {
            selected = j;
          }
          if(selected < current && j > current)
          {
            selected = j;
          }
        }
        else if(triggers[i][j].switch_back)
        {
          if(next_config[i] == j)
          {
            /* cancel the switch */
            selected = current_config[i];
          }
          else
          {
            /* switch back */
            selected = previous_config[i];
          }
          break;
        }
      }
    }
    if(selected > -1)
    {
      next_config[i] = selected;
      if(!up)
      {
        config_delay[i] = triggers[i][selected].delay / (gimx_params.refresh_period / 1000);
      }
      else
      {
        config_delay[i] = 0;
      }
      break;
    }
  }
}

/*
 * Check if a config activation has to be performed.
 */
void cfg_config_activation()
{
  int i, j;
  struct timeval tv;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    if(next_config[i] > -1)
    {
      if(!config_delay[i])
      {
        if(next_config[i] != current_config[i])
        {
          if(gimx_params.status)
          {
            gettimeofday(&tv, NULL);

            gprintf(_("%d %ld.%06ld controller %d is switched from configuration %d to %d\n"), i, tv.tv_sec, tv.tv_usec, i, current_config[i], next_config[i]);
          }
          previous_config[i] = current_config[i];
          current_config[i] = next_config[i];
          for(j=0; j<AXIS_MAX; ++j)
          {
            update_stick(i, j);
          }
        }
        next_config[i] = -1;
      }
      else
      {
        config_delay[i]--;
      }
    }
  }
}

/*
 * Specific stuff to postpone some GE_MOUSEBUTTONUP events
 * that come too quickly after corresponding GE_MOUSEBUTTONDOWN events.
 * If we don't do that, the PS3 will miss events.
 * 
 * This function also postpones mouse button up events in case a delayed config toggle is triggered.
 */
static int postpone_event(unsigned int device, GE_Event* event)
{
  int i;
  int ret = 0;
  s_mouse_control* mc = mouse_control + device;
  if (event->button.button == GE_BTN_WHEELUP
   || event->button.button == GE_BTN_WHEELDOWN
   || event->button.button == GE_BTN_WHEELRIGHT
   || event->button.button == GE_BTN_WHEELLEFT)
  {
    if (mc->postpone[event->button.button] + 1 < gimx_params.postpone_count)
    {
      GE_PushEvent(event);
      mc->postpone[event->button.button]++;
      ret = 1;
    }
    else
    {
      mc->postpone[event->button.button] = 0;
    }
  }

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    if(next_config[i] > -1)
    {
      if(triggers[i][next_config[i]].device_type == E_DEVICE_TYPE_MOUSE
          && triggers[i][next_config[i]].device_id == device
          && triggers[i][next_config[i]].button == event->button.button)
      {
        /* do not postpone the event if it has to trigger a switch back */
        if(!triggers[i][next_config[i]].switch_back)
        {
          GE_PushEvent(event);
          ret = 1;
          break;
        }
      }
    }
  }

  return ret;
}

static double mouse2axis(int device, s_adapter* controller, int which, double x, double y, s_axis_props* axis_props, double exp, double multiplier, int dead_zone, e_shape shape, e_mouse_mode mode)
{
  double z = 0;
  double dz = dead_zone;
  double motion_residue = 0;
  double ztrunk = 0;
  double val = 0;
  int min_axis, max_axis;
  int new_state;
  int axis = axis_props->axis;

  max_axis = controller_get_max_signed(controller->type, axis);
  if(axis_props->props == AXIS_PROP_CENTERED)
  {
    min_axis = -max_axis;
  }
  else
  {
    min_axis = 0;
  }

  multiplier *= controller_get_axis_scale(controller->type, axis);
  dz *= controller_get_axis_scale(controller->type, axis);

  if(which == AXIS_X)
  {
    val = x * gimx_params.frequency_scale;
    if(x && y && shape == E_SHAPE_CIRCLE)
    {
      dz = dz*cos(atan(fabs(y/x)));
    }
    if(device == current_mouse && (current_cal == DZX || current_cal == DZS))
    {
      if(val > 0)
      {
        controller->axis[axis] = dz;
      }
      else
      {
        controller->axis[axis] = -dz;
      }
      return 0;
    }
  }
  else if(which == AXIS_Y)
  {
    val = y * gimx_params.frequency_scale;
    if(x && y && shape == E_SHAPE_CIRCLE)
    {
      dz = dz*sin(atan(fabs(y/x)));
    }
    if(device == current_mouse && (current_cal == DZY || current_cal == DZS))
    {
      if(val > 0)
      {
        controller->axis[axis] = dz;
      }
      else
      {
        controller->axis[axis] = -dz;
      }
      return 0;
    }
  }

  if(val != 0)
  {
    z = multiplier * (val/fabs(val)) * pow(fabs(val), exp);
    /*
     * Subtract the first position to the dead zone (useful for high multipliers).
     */
    dz = dz - multiplier;// * pow(1, exp);
  }
  
  if(mode == E_MOUSE_MODE_AIMING)
  {
    if(z > 0)
    {
      controller->axis[axis] = dz + z;
      /*
       * max axis position => no residue
       */
      if(controller->axis[axis] < max_axis)
      {
        ztrunk = controller->axis[axis] - dz;
      }
    }
    else if(z < 0)
    {
      controller->axis[axis] = z - dz;
      /*
       * max axis position => no residue
       */
      if(controller->axis[axis] > min_axis)
      {
        ztrunk = controller->axis[axis] + dz;
      }
    }
    else controller->axis[axis] = 0;
  }
  else //E_MOUSE_MODE_DRIVING
  {
    new_state = controller->axis[axis] + z;
    if(new_state > 0 && new_state < dz)
    {
      new_state -= (2*dz);
    }
    if(new_state < 0 && new_state > -dz)
    {
      new_state += (2*dz);
    }
    controller->axis[axis] = clamp(min_axis, new_state, max_axis);
  }

  if(val != 0 && ztrunk != 0)
  {
    //printf("ztrunk: %.4f\n", ztrunk);
    /*
     * Compute the motion that wasn't applied due to the double to integer conversion.
     */
    motion_residue = (val/fabs(val)) * ( fabs(val) - pow(fabs(ztrunk)/multiplier, 1/exp) );
    if(fabs(motion_residue) < 0.0039)//allow 256 subpositions
    {
      motion_residue = 0;
    }
    //printf("motion_residue: %.4f\n", motion_residue);
  }

  return motion_residue;
}

void update_dbutton_axis(s_mapper* mapper, int c_id, int axis)
{
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];
  int value = intensity->value;
  if(mapper->axis_props.props & AXIS_PROP_NEGATIVE)
  {
    /*
     * Button to zero-centered axis.
     */
    adapter_get(c_id)->axis[axis] = - value;
  }
  else if(mapper->axis_props.props & AXIS_PROP_POSITIVE)
  {
    /*
     * Button to zero-centered axis.
     */
    adapter_get(c_id)->axis[axis] = value;
  }
  else
  {
    /*
     * Button to non-centered axis.
     */
    adapter_get(c_id)->axis[axis] = value;
  }
  update_stick(c_id, axis);
  /*
   * Specific code for issue 15.
   */
  if(axis >= rel_axis_0 && axis <= rel_axis_max)
  {
    if(mapper->axis_props.props & AXIS_PROP_POSITIVE)
    {
      adapter_get(c_id)->ts_axis[axis][0] = 1;
    }
    else if(mapper->axis_props.props & AXIS_PROP_NEGATIVE)
    {
      adapter_get(c_id)->ts_axis[axis][1] = 1;
    }
  }
}

void update_ubutton_axis(s_mapper* mapper, int c_id, int axis)
{
  int direction, opposite;
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];
  int value = intensity->value;
  adapter_get(c_id)->axis[axis] = 0;
  if(mapper->axis_props.props)
  {
    update_stick(c_id, axis);
  }
  if(axis >= rel_axis_0 && axis <= rel_axis_max)
  {
    if(mapper->axis_props.props & AXIS_PROP_POSITIVE)
    {
      direction = 0;
      opposite = 1;
    }
    else
    {
      direction = 1;
      opposite = 0;
    }
    adapter_get(c_id)->ts_axis[axis][direction] = 0;
    if(adapter_get(c_id)->ts_axis[axis][opposite] == 1)
    {
      if(mapper->axis_props.props & AXIS_PROP_NEGATIVE)
      {
        adapter_get(c_id)->axis[axis] = value;
      }
      else
      {
        adapter_get(c_id)->axis[axis] = -value;
      }
    }
  }
}

/*
 * Updates the state table.
 * Too long function, but not hard to understand.
 */
void cfg_process_event(GE_Event* event)
{
  s_mapper* mapper;
  int axis;
  unsigned int config;
  unsigned int c_id;
  unsigned int control;
  int threshold;
  double multiplier;
  double exp;
  double dead_zone;
  e_shape shape;
  int value = 0;
  double fvalue = 0;
  unsigned int nb_controls = 0;
  double mx;
  double my;
  double residue;
  s_mouse_control* mc;
  int min_axis, max_axis;
  e_mouse_mode mode;
  s_adapter* controller;

  unsigned int device = GE_GetDeviceId(event);

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    controller = adapter_get(c_id);
    config = current_config[c_id];

    nb_controls = 0;

    switch(event->type)
    {
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
      if(joystick_buttons[device][c_id][config])
      {
        nb_controls = joystick_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case GE_JOYAXISMOTION:
      if(joystick_axes[device][c_id][config])
      {
        nb_controls = joystick_axes[device][c_id][config]->nb_mappers;
      }
      break;
      case GE_KEYDOWN:
      case GE_KEYUP:
      if(keyboard_buttons[device][c_id][config])
      {
        nb_controls = keyboard_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
      if(mouse_buttons[device][c_id][config])
      {
        nb_controls = mouse_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case GE_MOUSEMOTION:
      if(mouse_axes[device][c_id][config])
      {
        nb_controls = mouse_axes[device][c_id][config]->nb_mappers;
      }
      break;
    }

    for(control=0; control<nb_controls; ++control)
    {
      switch(event->type)
      {
        case GE_JOYBUTTONDOWN:
          mapper = joystick_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_JOYBUTTONUP:
          mapper = joystick_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_JOYAXISMOTION:
          mapper = joystick_axes[device][c_id][config]+control;
          /*
           * Check that it's the right axis.
           */
          if(mapper->axis != event->jaxis.axis)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            multiplier = mapper->multiplier * controller_get_axis_scale(controller->type, axis);
            exp = mapper->exponent;
            dead_zone = mapper->dead_zone * controller_get_axis_scale(controller->type, axis);
            value = event->jaxis.value;
            max_axis = controller_get_max_signed(controller->type, axis);
            if(mapper->axis_props.props == AXIS_PROP_CENTERED)
            {
              min_axis = -max_axis;
            }
            else
            {
              min_axis = 0;
            }
            if(multiplier)
            {
              /*
               * Axis to axis.
               */
              if(value)
              {
                value = value/abs(value)*multiplier*pow(abs(value), exp);
              }
              if(value > 0)
              {
                value += dead_zone;
              }
              else if(value < 0)
              {
                value -= dead_zone;
              }
              controller->axis[axis] = clamp(min_axis, value, max_axis);
            }
            else
            {
              /*
               * Axis to button.
               */
              threshold = mapper->threshold;
              if(threshold > 0 && value > threshold)
              {
                controller->axis[axis] = max_axis;
              }
              else if(threshold < 0 && value < threshold)
              {
                controller->axis[axis] = max_axis;
              }
              else
              {
                controller->axis[axis] = min_axis;
              }
            }
          }
          break;
        case GE_KEYDOWN:
          mapper = keyboard_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_KEYUP:
          mapper = keyboard_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_MOUSEMOTION:
          mapper = mouse_axes[device][c_id][config]+control;
          mc = mouse_control + device;
          if(mc->change)
          {
            mx = mc->x;
            my = mc->y;
          }
          else
          {
            mx = 0;
            my = 0;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            multiplier = mapper->multiplier;
            if(multiplier)
            {
              /*
               * Axis to axis.
               */
              exp = mapper->exponent;
              dead_zone = mapper->dead_zone;
              shape = mapper->shape;
              mode = cal_get_mouse(device, config)->mode;
              residue = mouse2axis(device, controller, mapper->axis, mx, my, &mapper->axis_props, exp, multiplier, dead_zone, shape, mode);
              if(mapper->axis == AXIS_X)
              {
                mc->residue_x = residue;
              }
              else if(mapper->axis == AXIS_Y)
              {
                mc->residue_y = residue;
              }
            }
            else
            {
              if (mapper->axis == AXIS_X)
              {
                fvalue = mx;
              }
              else
              {
                fvalue = my;
              }
              /*
               * Axis to button.
               */
              max_axis = controller_get_max_signed(controller->type, axis);
              threshold = mapper->threshold;
              if(threshold > 0 && fvalue > threshold)
              {
                controller->axis[axis] = max_axis;
              }
              else if(threshold < 0 && fvalue < threshold)
              {
                controller->axis[axis] = max_axis;
              }
              else
              {
                controller->axis[axis] = 0;
              }
            }
          }
          break;
        case GE_MOUSEBUTTONDOWN:
          mapper = mouse_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->button.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_MOUSEBUTTONUP:
          mapper = mouse_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->button.button)
          {
            continue;
          }
          /*
           * Check if this event needs to be postponed.
           */
          if(postpone_event(device, event))
          {
            return; //no need to do something more
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
      }
    }
  }
}
