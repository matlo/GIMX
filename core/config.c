#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include <stdlib.h>
#include "sixaxis.h"
#include <math.h>
#include "conversion.h"
#include <unistd.h>
#include <sys/time.h>
#include "sdl_tools.h"
#include "calibration.h"
#include "emuclient.h"

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
static s_intensity axis_intensity[MAX_CONTROLLERS][MAX_CONFIGURATIONS][SA_MAX];

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

void cfg_process_motion_event(SDL_Event* event)
{
  s_mouse_control* mc = cfg_get_mouse_control(sdl_get_device_id(event));
  if(mc)
  {
    mc->merge_x[mc->index] += event->motion.xrel;
    mc->merge_y[mc->index] += event->motion.yrel;
    mc->change = 1;
  }
}

#define MOTION_NB 1
#define RATIO 1
int weight;
int divider;

void cfg_process_motion()
{
  int i, j, k;
  s_mouse_control* mc;
  s_mouse_cal* mcal;
  SDL_Event mouse_evt = { };
  /*
   * Process a single (merged) motion event for each mouse.
   */
  for (i = 0; i < MAX_DEVICES; ++i)
  {
    mc = cfg_get_mouse_control(i);
    mcal = cal_get_mouse(i, current_config[cal_get_controller(i)]);
    if (mc->changed || mc->change)
    {
      if (subpos)
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

      if(mcal->bsx)
      {
        mc->x = 0;
        weight = 1;
        divider = 0;
        for(j=0; j<*mcal->bsx; ++j)
        {
          k = mc->index - j;
          if (k < 0)
          {
            k += MAX_BUFFERSIZE;
          }
          mc->x += (mc->merge_x[k]*weight);
          divider += weight;
          weight *= *mcal->fix;
        }
        mc->x /= divider;
      }

      if(mcal->bsy)
      {
        mc->y = 0;
        weight = 1;
        divider = 0;
        for(j=0; j<*mcal->bsy; ++j)
        {
          k = mc->index - j;
          if (k < 0)
          {
            k += MAX_BUFFERSIZE;
          }
          mc->y += (mc->merge_y[k]*weight);
          divider += weight;
          weight *= *mcal->fiy;
        }
        mc->y /= divider;
      }

      mouse_evt.motion.which = i;
      mouse_evt.type = SDL_MOUSEMOTION;
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
 * It only does something for axes in {sa_lstick_x, sa_lstick_y, sa_rstick_x, sa_rstick_y}
 */
static void update_stick(int c_id, int axis)
{
  if(axis > sa_rstick_y)
  {
    return;
  }
  
  if(axis & 1)
  {
    axis = axis - 1;
  }
  
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];
  int value = intensity->value;

  if(intensity->down_button == -1 && intensity->up_button == -1)
  {
    return;
  }

  if (intensity->shape == E_SHAPE_CIRCLE)
  {
    if (state[c_id].user.axis[axis] && state[c_id].user.axis[axis+1])
    {
      value = sqrt(value * value / 2);
    }
  }

  if (state[c_id].user.axis[axis] > 0)
  {
    state[c_id].user.axis[axis] = round(value);
    controller[c_id].send_command = 1;
  }
  else if (state[c_id].user.axis[axis] < 0)
  {
    state[c_id].user.axis[axis] = -round(value);
    controller[c_id].send_command = 1;
  }
  if (state[c_id].user.axis[axis+1] > 0)
  {
    state[c_id].user.axis[axis+1] = round(value);
    controller[c_id].send_command = 1;
  }
  else if (state[c_id].user.axis[axis+1] < 0)
  {
    state[c_id].user.axis[axis+1] = -round(value);
    controller[c_id].send_command = 1;
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
    if (intensity->value > mean_axis_value)
    {
      if (intensity->down_button != -1)
      {
        intensity->value = mean_axis_value;
      }
      else
      {
        intensity->value = (double) intensity->dead_zone
            + intensity->step;
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
        intensity->value = mean_axis_value;
      }
    }
    ret = 1;
  }

  return ret;
}

/*
 * Check if axis intensities need to be updated.
 */
void cfg_intensity_lookup(SDL_Event* e)
{
  int c_id, a_id;
  int device_type;
  int button_id;
  unsigned int device_id = sdl_get_device_id(e);

  switch( e->type )
  {
    case SDL_JOYBUTTONDOWN:
      device_type = E_DEVICE_TYPE_JOYSTICK;
      button_id = e->jbutton.button;
      break;
    case SDL_KEYDOWN:
      device_type = E_DEVICE_TYPE_KEYBOARD;
      button_id = e->key.keysym.sym;
      break;
    case SDL_MOUSEBUTTONDOWN:
      device_type = E_DEVICE_TYPE_MOUSE;
      button_id = e->button.button;
      break;
    default:
      return;
  }

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    for(a_id=0; a_id<SA_MAX; ++a_id)
    {
      if(update_intensity(device_type, device_id, button_id, c_id, a_id))
      {
        update_stick(c_id, a_id);
        gprintf("controller %d configuration %d axis %s intensity: %.0f\n", c_id, current_config[c_id], get_axis_name(a_id), axis_intensity[c_id][current_config[c_id]][a_id].value);
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
void cfg_trigger_lookup(SDL_Event* e)
{
  int i, j;
  int device_type;
  int button;
  int up = 0;
  int selected;
  int current;
  unsigned int device_id = sdl_get_device_id(e);

  switch( e->type )
  {
    case SDL_JOYBUTTONUP:
      up = 1;
    case SDL_JOYBUTTONDOWN:
      device_type = E_DEVICE_TYPE_JOYSTICK;
      button = e->jbutton.button;
      break;
    case SDL_KEYUP:
      up = 1;
    case SDL_KEYDOWN:
      device_type = E_DEVICE_TYPE_KEYBOARD;
      button = e->key.keysym.sym;
      break;
    case SDL_MOUSEBUTTONUP:
      up = 1;
    case SDL_MOUSEBUTTONDOWN:
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
        config_delay[i] = triggers[i][selected].delay / (refresh_rate / 1000);
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
          if(display)
          {
            gettimeofday(&tv, NULL);

            gprintf("%d %ld.%06ld controller %d is switched from configuration %d to %d\n", i, tv.tv_sec, tv.tv_usec, i, current_config[i], next_config[i]);
          }
          previous_config[i] = current_config[i];
          current_config[i] = next_config[i];
          for(j=0; j<SA_MAX; ++j)
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
 * Specific stuff to postpone some SDL_MOUSEBUTTONUP events
 * that come too quickly after corresponding SDL_MOUSEBUTTONDOWN events.
 * If we don't do that, the PS3 will miss events.
 * 
 * This function also postpones mouse button up events in case a delayed config toggle is triggered.
 */
static int postpone_event(unsigned int device, SDL_Event* event)
{
  int i;
  int ret = 0;
  s_mouse_control* mc = mouse_control + device;
  if (event->button.button == SDL_BUTTON_WHEELUP)
  {
    if (mc->postpone_wheel_up < postpone_count)
    {
      SDL_PushEvent(event);
      mc->postpone_wheel_up++;
      ret = 1;
    }
    else
    {
      mc->postpone_wheel_up = 0;
    }
  }
  else if (event->button.button == SDL_BUTTON_WHEELDOWN)
  {
    if (mc->postpone_wheel_down < postpone_count)
    {
      SDL_PushEvent(event);
      mc->postpone_wheel_down++;
      ret = 1;
    }
    else
    {
      mc->postpone_wheel_down = 0;
    }
  }
  else if (event->button.button == SDL_BUTTON_X1)
  {
    if (mc->postpone_button_x1 < postpone_count)
    {
      SDL_PushEvent(event);
      mc->postpone_button_x1++;
      ret = 1;
    }
    else
    {
      mc->postpone_button_x1 = 0;
    }
  }
  else if (event->button.button == SDL_BUTTON_X2)
  {
    if (mc->postpone_button_x2 < postpone_count)
    {
      SDL_PushEvent(event);
      mc->postpone_button_x2++;
      ret = 1;
    }
    else
    {
      mc->postpone_button_x2 = 0;
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
          SDL_PushEvent(event);
          ret = 1;
          break;
        }
      }
    }
  }

  return ret;
}

static double mouse2axis(int device, struct sixaxis_state* state, int which, double x, double y, int axis, double exp, double multiplier, int dead_zone, e_shape shape)
{
  double z = 0;
  double dz = dead_zone;
  double motion_residue = 0;
  double ztrunk = 0;
  double val = 0;

  multiplier *= axis_scale;
  dz *= axis_scale;

  if(which == AXIS_X)
  {
    val = x * frequency_scale;
    if(x && y && shape == E_SHAPE_CIRCLE)
    {
      dz = dz*cos(atan(fabs(y/x)));
    }
    if(device == current_mouse && (current_cal == DZX || current_cal == DZS))
    {
      if(val > 0)
      {
        state->user.axis[axis] = dz;
      }
      else
      {
        state->user.axis[axis] = -dz;
      }
      return 0;
    }
  }
  else if(which == AXIS_Y)
  {
    val = y * frequency_scale;
    if(x && y && shape == E_SHAPE_CIRCLE)
    {
      dz = dz*sin(atan(fabs(y/x)));
    }
    if(device == current_mouse && (current_cal == DZY || current_cal == DZS))
    {
      if(val > 0)
      {
        state->user.axis[axis] = dz;
      }
      else
      {
        state->user.axis[axis] = -dz;
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

  if(z > 0)
  {
    state->user.axis[axis] = dz + z;
    /*
     * max axis position => no residue
     */
    if(state->user.axis[axis] < mean_axis_value)
    {
      ztrunk = state->user.axis[axis] - dz;
    }
  }
  else if(z < 0)
  {
    state->user.axis[axis] = z - dz;
    /*
     * max axis position => no residue
     */
    if(state->user.axis[axis] > -mean_axis_value)
    {
      ztrunk = state->user.axis[axis] + dz;
    }
  }
  else state->user.axis[axis] = 0;

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
  if(mapper->controller_axis_value < 0)
  {
    /*
     * Button to zero-centered axis.
     */
    state[c_id].user.axis[axis] = - value;
  }
  else if(mapper->controller_axis_value > 0)
  {
    /*
     * Button to zero-centered axis.
     */
    state[c_id].user.axis[axis] = value;
  }
  else
  {
    /*
     * Button to non-centered axis.
     */
    state[c_id].user.axis[axis] = value;
  }
  update_stick(c_id, axis);
  /*
   * Specific code for issue 15.
   */
  if(axis >= sa_lstick_x && axis <= sa_gyro)
  {
    if(mapper->controller_axis_value > 0)
    {
      controller[c_id].ts_axis[axis][0] = 1;
    }
    else if(mapper->controller_axis_value < 0)
    {
      controller[c_id].ts_axis[axis][1] = 1;
    }
  }
}

void update_ubutton_axis(s_mapper* mapper, int c_id, int axis)
{
  int dir, odir;
  s_intensity* intensity = &axis_intensity[c_id][current_config[c_id]][axis];
  int value = intensity->value;
  state[c_id].user.axis[axis] = 0;
  if(mapper->controller_axis_value)
  {
    update_stick(c_id, axis);
  }
  if(axis >= sa_lstick_x && axis <= sa_gyro)
  {
    if(mapper->controller_axis_value > 0)
    {
      dir = 0;
      odir = 1;
    }
    else
    {
      dir = 1;
      odir = 0;
    }
    controller[c_id].ts_axis[axis][dir] = 0;
    if(controller[c_id].ts_axis[axis][odir] == 1)
    {
      if(mapper->controller_axis_value < 0)
      {
        state[c_id].user.axis[axis] = value;
      }
      else
      {
        state[c_id].user.axis[axis] = -value;
      }
    }
  }
}

/*
 * Updates the state table.
 * Too long function, but not hard to understand.
 */
void cfg_process_event(SDL_Event* event)
{
  s_mapper* mapper;
  unsigned int axis;
  unsigned int config;
  unsigned int c_id;
  unsigned int control;
  int threshold;
  double multiplier;
  double exp;
  int dead_zone;
  e_shape shape;
  int value = 0;
  unsigned int nb_controls = 0;
  double mx;
  double my;
  double residue;
  s_mouse_control* mc;
  int nb_buttons;
  unsigned char hat;

  /*
   * 'which' should always be at that place
   * There is no need to check the value, since it's stored as an uint8_t, and MAX_DEVICES is 256.
   */
  unsigned int device = sdl_get_device_id(event);

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    config = current_config[c_id];

    nb_controls = 0;

    switch(event->type)
    {
      case SDL_JOYHATMOTION:
      event->jbutton.which = event->jhat.which;
      nb_buttons = sdl_get_joystick_buttons(event->jhat.which);
      hat = event->jhat.hat;
      if(event->jhat.value & SDL_HAT_UP)
      {
        event->jbutton.type = SDL_JOYBUTTONDOWN;
        event->jbutton.button=nb_buttons+4*hat;
        cfg_process_event(event);
      }
      else
      {
        event->jbutton.type = SDL_JOYBUTTONUP;
        event->jbutton.button=nb_buttons+4*hat;
        cfg_process_event(event);
      }
      if(event->jhat.value & SDL_HAT_RIGHT)
      {
        event->jbutton.type = SDL_JOYBUTTONDOWN;
        event->jbutton.button=nb_buttons+4*hat+1;
        cfg_process_event(event);
      }
      else
      {
        event->jbutton.type = SDL_JOYBUTTONUP;
        event->jbutton.button=nb_buttons+4*hat+1;
        cfg_process_event(event);
      }
      if(event->jhat.value & SDL_HAT_DOWN)
      {
        event->jbutton.type = SDL_JOYBUTTONDOWN;
        event->jbutton.button=nb_buttons+4*hat+2;
        cfg_process_event(event);
      }
      else
      {
        event->jbutton.type = SDL_JOYBUTTONUP;
        event->jbutton.button=nb_buttons+4*hat+2;
        cfg_process_event(event);
      }
      if(event->jhat.value & SDL_HAT_LEFT)
      {
        event->jbutton.type = SDL_JOYBUTTONDOWN;
        event->jbutton.button=nb_buttons+4*hat+3;
        cfg_process_event(event);
      }
      else
      {
        event->jbutton.type = SDL_JOYBUTTONUP;
        event->jbutton.button=nb_buttons+4*hat+3;
        cfg_process_event(event);
      }
      return;
      break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
      if(joystick_buttons[device][c_id][config])
      {
        nb_controls = joystick_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case SDL_JOYAXISMOTION:
      if(sdl_is_sixaxis(device) && event->jaxis.axis > 3)
      {
        event->jaxis.value = (event->jaxis.value + 32767) / 2;
      }
      if(joystick_axes[device][c_id][config])
      {
        nb_controls = joystick_axes[device][c_id][config]->nb_mappers;
      }
      break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
      if(keyboard_buttons[device][c_id][config])
      {
        nb_controls = keyboard_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      if(mouse_buttons[device][c_id][config])
      {
        nb_controls = mouse_buttons[device][c_id][config]->nb_mappers;
      }
      break;
      case SDL_MOUSEMOTION:
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
        case SDL_JOYBUTTONDOWN:
          mapper = joystick_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case SDL_JOYBUTTONUP:
          mapper = joystick_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case SDL_JOYAXISMOTION:
          mapper = joystick_axes[device][c_id][config]+control;
          /*
           * Check that it's the right axis.
           */
          if(mapper->axis != event->jaxis.axis)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          multiplier = mapper->multiplier;
          exp = mapper->exponent;
          dead_zone = mapper->dead_zone;
          if(axis >= 0)
          {
            if(mapper->controller_axis_value)
            {
              multiplier *= axis_scale;
              value = event->jaxis.value;
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
              state[c_id].user.axis[axis] = clamp(-mean_axis_value, value , mean_axis_value);
            }
            else
            {
              value = event->jaxis.value;
              if(value)
              {
                value = value/abs(value)*multiplier*pow(abs(value), exp);
              }
              if(value > 0)
              {
                value += dead_zone;
                state[c_id].user.axis[axis] = clamp(0, value , 255);
              }
              else
              {
                state[c_id].user.axis[axis] = 0;
              }
            }
          }
          break;
        case SDL_KEYDOWN:
          mapper = keyboard_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym.sym)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case SDL_KEYUP:
          mapper = keyboard_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym.sym)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case SDL_MOUSEMOTION:
          mapper = mouse_axes[device][c_id][config]+control;
          /*
           * Check the mouse axis.
           */
          if (mapper->axis == AXIS_X)
          {
            value = event->motion.xrel;
          }
          else if(mapper->axis == AXIS_Y)
          {
            value = event->motion.yrel;
          }
          else
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            multiplier = mapper->multiplier;
            if(mapper->controller_axis_value || multiplier)
            {
              /*
               * Axis to zero-centered axis.
               */
              exp = mapper->exponent;
              dead_zone = mapper->dead_zone;
              shape = mapper->shape;
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
              residue = mouse2axis(device, state+c_id, mapper->axis, mx, my, axis, exp, multiplier, dead_zone, shape);
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
              /*
               * Axis to non-centered axis.
               */
              threshold = mapper->threshold;
              if(threshold > 0 && value > threshold)
              {
                state[c_id].user.axis[axis] = 255;
              }
              else if(threshold < 0 && value < threshold)
              {
                state[c_id].user.axis[axis] = 255;
              }
              else
              {
                state[c_id].user.axis[axis] = 0;
              }
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          mapper = mouse_buttons[device][c_id][config]+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->button.button)
          {
            continue;
          }
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case SDL_MOUSEBUTTONUP:
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
          controller[c_id].send_command = 1;
          axis = mapper->controller_axis;
          if(axis >= 0)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
      }
    }
  }
}
