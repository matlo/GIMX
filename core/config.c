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
#include <ginput.h>
#include "calibration.h"
#include "gimx.h"
#include "macros.h"
#include <adapter.h>

#define DEFAULT_RADIUS 512
#define DEFAULT_VELOCITY 1

static unsigned int controller_dpi[MAX_CONTROLLERS];

typedef struct
{
  int device_type;
  int device_id;
  int button;
} s_event;

typedef struct profile
{
  int index;
  struct
  {
    s_event event;
    int switch_back;
    int delay; // in ms
  } trigger;
  struct
  {
    struct profile * previous;
    struct profile * next;
  } state;
} s_profile;

static struct
{
  s_profile * current;
  s_profile * next;
  int delay; // in periods
  s_profile profiles[MAX_CONFIGURATIONS];
} cfg_controllers[MAX_CONTROLLERS];

/*
 * This lists controller stick intensity modifiers.
 */
static s_intensity axis_intensity[MAX_CONTROLLERS][MAX_CONFIGURATIONS][AXIS_MAX];

/*
 * This lists controls of each controller configuration for all keyboards.
 */
static s_mapper_table keyboard_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

/*
 * This lists controls of each controller configuration for all mice.
 */
static s_mapper_table mouse_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

static s_mapper_table mouse_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

/*
 * Used to tweak mouse controls.
 */
static s_mouse_control mouse_control[MAX_DEVICES] = {};

/*
 * This lists controls of each controller configuration for all joysticks.
 */
static s_mapper_table joystick_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];
static s_mapper_table joystick_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_CONFIGURATIONS];

static struct
{
  unsigned int nb;
  s_js_corr * corr;
} js_corr[MAX_DEVICES] = {};

void cfg_add_js_corr(uint8_t device, s_js_corr * corr)
{
  void * ptr = realloc(js_corr[device].corr, (js_corr[device].nb + 1) * sizeof(*(js_corr->corr)));
  if(ptr == NULL)
  {
    fprintf(stderr, "%s:%d %s: realloc failed\n", __FILE__, __LINE__, __func__);
    return;
  }
  js_corr[device].corr = ptr;
  js_corr[device].corr[js_corr[device].nb].axis = corr->axis;
  memcpy(js_corr[device].corr[js_corr[device].nb].coef, corr->coef, sizeof(corr->coef));
  ++(js_corr[device].nb);
}

static s_js_corr * get_js_corr(uint8_t device, uint8_t axis)
{
  unsigned int i;
  for (i = 0; i < js_corr[device].nb; ++i)
  {
    if(js_corr[device].corr[i].axis == axis)
    {
      return js_corr[device].corr + i;
    }
  }
  return NULL;
}

s_mapper_table* cfg_get_joystick_axes(int device, int controller, int config)
{
  return &(joystick_axes[device][controller][config]);
}

s_mapper_table* cfg_get_joystick_buttons(int device, int controller, int config)
{
  return &(joystick_buttons[device][controller][config]);
}

s_mapper_table* cfg_get_mouse_axes(int device, int controller, int config)
{
  return &(mouse_axes[device][controller][config]);
}

s_mapper_table* cfg_get_mouse_buttons(int device, int controller, int config)
{
  return &(mouse_buttons[device][controller][config]);
}

s_mapper_table* cfg_get_keyboard_buttons(int device, int controller, int config)
{
  return &(keyboard_buttons[device][controller][config]);
}

void cfg_set_trigger(s_config_entry* entry)
{
  cfg_controllers[entry->controller_id].profiles[entry->config_id].trigger.event.button = entry->event.id;
  cfg_controllers[entry->controller_id].profiles[entry->config_id].trigger.event.device_id = entry->device.id;
  cfg_controllers[entry->controller_id].profiles[entry->config_id].trigger.event.device_type = entry->device.type;
  cfg_controllers[entry->controller_id].profiles[entry->config_id].trigger.switch_back = entry->params.trigger.switch_back;
  cfg_controllers[entry->controller_id].profiles[entry->config_id].trigger.delay = entry->params.trigger.delay;
}

void cfg_set_controller_dpi(int controller, unsigned int dpi)
{
  controller_dpi[controller] = dpi;
}

void cfg_set_axis_intensity(s_config_entry* entry, int axis, s_intensity* intensity)
{
  s_intensity * target = axis_intensity[entry->controller_id][entry->config_id] + axis;

  if (intensity->down.button != -1)
  {
    target->down = intensity->down;
  }
  if (intensity->up.button != -1)
  {
    target->up = intensity->up;
  }
  target->params = intensity->params;
  target->dead_zone = (double) intensity->params.dead_zone * controller_get_axis_scale(adapter_get(entry->controller_id)->ctype, axis);
  target->step = (target->max_value - target->dead_zone) / intensity->params.steps;
}

void cfg_intensity_init()
{
  int i, j, k;
  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    if(adapter_get(i)->ctype == C_TYPE_NONE)
    {
      continue;
    }
    for (j = 0; j < MAX_CONFIGURATIONS; ++j)
    {
      for (k = 0; k < AXIS_MAX; ++k)
      {
        s_intensity* intensity = &(axis_intensity[i][j][k]);

        intensity->up.button = -1;
        intensity->down.button = -1;
        intensity->max_value = controller_get_max_signed(adapter_get(i)->ctype, k);
        intensity->value = intensity->max_value;
      }
    }
  }
}

static s_mapper_table* get_mapper_table(s_config_entry* entry)
{
  s_mapper_table* table = NULL;

  if(entry->device.id < 0) return NULL;

  switch(entry->device.type)
  {
    case E_DEVICE_TYPE_KEYBOARD:
      table = cfg_get_keyboard_buttons(entry->device.id, entry->controller_id, entry->config_id);
      break;
    case E_DEVICE_TYPE_MOUSE:
      switch(entry->event.type)
      {
        case E_EVENT_TYPE_BUTTON:
          table = cfg_get_mouse_buttons(entry->device.id, entry->controller_id, entry->config_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          table = cfg_get_mouse_axes(entry->device.id, entry->controller_id, entry->config_id);
          break;
        default:
          break;
      }
      break;
    case E_DEVICE_TYPE_JOYSTICK:
      switch(entry->event.type)
      {
        case E_EVENT_TYPE_BUTTON:
          table = cfg_get_joystick_buttons(entry->device.id, entry->controller_id, entry->config_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          table = cfg_get_joystick_axes(entry->device.id, entry->controller_id, entry->config_id);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return table;
}

static s_mapper* allocate_mapper(s_config_entry* entry)
{
  s_mapper* ret = NULL;

  s_mapper_table* table = get_mapper_table(entry);

  void* ptr = realloc(table->mappers, (table->nb_mappers+1)*sizeof(*table->mappers));

  if(ptr)
  {
    table->mappers = ptr;
    ret = table->mappers+table->nb_mappers;
    memset(ret, 0x00, sizeof(*ret));
    table->nb_mappers++;
  }
  else
  {
    fprintf(stderr, "can't allocate mapper\n");
  }

  return ret;
}

int cfg_add_binding(s_config_entry* entry)
{
  int ret = 0;

  s_mapper* p_mapper = allocate_mapper(entry);

  if(p_mapper)
  {
    *p_mapper = entry->params.mapper;
  }
  else
  {
    ret = -1;
  }

  return ret;
}

#define JOYSTICK_RUMBLE_REFRESH_PERIOD 20000

static struct
{
  unsigned short weak;
  unsigned short strong;
  unsigned char updated; //an update has been received
  unsigned char active; //the rumble is active
  int elapsed; //the time elapsed since the last GE_JoystickSetRumble() call
} joystick_rumble[MAX_DEVICES] = {};

void cfg_process_rumble_event(GE_Event* event)
{
  joystick_rumble[event->jrumble.which].weak = event->jrumble.weak;
  joystick_rumble[event->jrumble.which].strong = event->jrumble.strong;
  joystick_rumble[event->jrumble.which].updated = 1;
}

void cfg_process_rumble()
{
  int i;
  for (i = 0; i < MAX_DEVICES; ++i)
  {
    joystick_rumble[i].elapsed += gimx_params.refresh_period;

    if(joystick_rumble[i].updated && joystick_rumble[i].elapsed >= JOYSTICK_RUMBLE_REFRESH_PERIOD)
    {
      unsigned short weak = joystick_rumble[i].weak;
      unsigned short strong = joystick_rumble[i].strong;

      unsigned char active = weak || strong;

      if(joystick_rumble[i].active || active)
      {
        GE_Event haptic = { .jrumble = { .type = GE_HAPTIC_RUMBLE, .which = i, .weak = weak, .strong = strong } };
        ginput_joystick_set_haptic(&haptic);
      }

      joystick_rumble[i].active = active;

      joystick_rumble[i].weak = 0;
      joystick_rumble[i].strong = 0;
      joystick_rumble[i].updated = 0;
      joystick_rumble[i].elapsed = 0;
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
      if(joystick_buttons[id][j][k].nb_mappers || joystick_axes[id][j][k].nb_mappers)
      {
        used = 1;
      }
    }
  }
  return used;
}

s_mouse_control* cfg_get_mouse_control(int id)
{
  if(id >= 0)
  {
    return mouse_control + id;
  }
  return NULL;
}

void cfg_process_motion_event(GE_Event* event)
{
  s_mouse_control* mc = cfg_get_mouse_control(ginput_get_device_id(event));
  if(mc)
  {
    mc->merge_x[mc->index] += event->motion.xrel;
    mc->merge_y[mc->index] += event->motion.yrel;
    mc->change = 1;
  }
}

void cfg_process_motion()
{
  int i, k;
  unsigned int j;
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
    mcal = cal_get_mouse(i, cfg_controllers[cal_get_controller(i)].current->index);
    if(!mc->change && mcal->options.mode == E_MOUSE_MODE_DRIVING)
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
      for(j=0; j<mcal->options.buffer_size; ++j)
      {
        k = mc->index - j;
        if (k < 0)
        {
          k += MAX_BUFFERSIZE;
        }
        mc->x += (mc->merge_x[k]*weight);
        divider += weight;
        weight *= mcal->options.filter;
      }
      mc->x /= divider;

      mc->y = 0;
      weight = 1;
      divider = 0;
      for(j=0; j<mcal->options.buffer_size; ++j)
      {
        k = mc->index - j;
        if (k < 0)
        {
          k += MAX_BUFFERSIZE;
        }
        mc->y += (mc->merge_y[k]*weight);
        divider += weight;
        weight *= mcal->options.filter;
      }
      mc->y /= divider;

      mouse_evt.motion.which = i;
      mouse_evt.type = GE_MOUSEMOTION;
      cfg_process_event(&mouse_evt);

      mouse_evt.motion.xrel = mc->x;
      mouse_evt.motion.yrel = mc->y;
      macro_lookup(&mouse_evt);
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
  
  s_intensity* intensity = &axis_intensity[c_id][cfg_controllers[c_id].current->index][axis];
  double value = intensity->value;

  if(intensity->down.button == -1 && intensity->up.button == -1)
  {
    return;
  }

  if (axis <= rel_axis_rstick_y && intensity->params.shape == E_SHAPE_CIRCLE)
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
  
  s_intensity* intensity = &axis_intensity[c_id][cfg_controllers[c_id].current->index][axis];

  if (intensity->up.device.type == device_type && device_id == intensity->up.device.id && button == intensity->up.button)
  {
    intensity->value += intensity->step;
    if (intensity->value > intensity->max_value)
    {
      if (intensity->down.button != -1)
      {
        intensity->value = intensity->max_value;
      }
      else
      {
        intensity->value = intensity->dead_zone + intensity->step;
      }
    }
    ret = 1;
  }
  else if (intensity->down.device.type == device_type && device_id == intensity->down.device.id && button == intensity->down.button)
  {
    intensity->value -= intensity->step;
    if (intensity->value < intensity->dead_zone + intensity->step)
    {
      if (intensity->up.button != -1)
      {
        intensity->value = intensity->dead_zone + intensity->step;
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
  unsigned int device_id = ginput_get_device_id(e);

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
    if(adapter_get(c_id)->ctype == C_TYPE_NONE)
    {
      continue;
    }
    for(a_id=0; a_id<AXIS_MAX; ++a_id)
    {
      if(update_intensity(device_type, device_id, button_id, c_id, a_id))
      {
        update_stick(c_id, a_id);
        gprintf(_("controller %d configuration %d axis %s intensity: %.0f\n"), c_id, cfg_controllers[c_id].current->index, controller_get_axis_name(adapter_get(c_id)->ctype, a_id), axis_intensity[c_id][cfg_controllers[c_id].current->index][a_id].value);
      }
    }
  }
}

/*
 * Initialize next_config and prev_config tables.
 */
void cfg_trigger_init()
{
  int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    cfg_controllers[i].current = cfg_controllers[i].profiles;
    cfg_controllers[i].next = NULL;
    cfg_controllers[i].delay = 0;
    for(j=0; j<MAX_CONFIGURATIONS; ++j)
    {
      s_profile * profile = cfg_controllers[i].profiles + j;
      profile->index = j;
      profile->state.next = NULL;
      profile->state.previous = NULL;
    }
  }
}

static inline s_event get_event(GE_Event * event)
{
  s_event e = { 0, 0, 0 };

  e.device_id = ginput_get_device_id(event);

  switch( event->type )
  {
    case GE_JOYBUTTONUP:
      e.device_type = E_DEVICE_TYPE_JOYSTICK;
      e.button = event->jbutton.button;
      break;
    case GE_JOYBUTTONDOWN:
      e.device_type = E_DEVICE_TYPE_JOYSTICK;
      e.button = event->jbutton.button;
      break;
    case GE_KEYUP:
      e.device_type = E_DEVICE_TYPE_KEYBOARD;
      e.button = event->key.keysym;
      break;
    case GE_KEYDOWN:
      e.device_type = E_DEVICE_TYPE_KEYBOARD;
      e.button = event->key.keysym;
      break;
    case GE_MOUSEBUTTONUP:
      e.device_type = E_DEVICE_TYPE_MOUSE;
      e.button = event->button.button;
      break;
    case GE_MOUSEBUTTONDOWN:
      e.device_type = E_DEVICE_TYPE_MOUSE;
      e.button = event->button.button;
      break;
    default:
      break;
  }

  return e;
}

static inline int compare_trigger(s_profile * profile, s_event * event)
{
  if(event->device_type != profile->trigger.event.device_type
      || event->device_id != profile->trigger.event.device_id
      || event->button != profile->trigger.event.button)
  {
    return 1;
  }

  return 0;
}

/*
 * Check if current configurations of controllers need to be updated.
 */
void cfg_trigger_lookup(GE_Event* e)
{
  int i, j;
  int up = 0;
  s_profile * selected;

  switch( e->type )
  {
    case GE_JOYBUTTONUP:
    case GE_KEYUP:
    case GE_MOUSEBUTTONUP:
      up = 1;
      break;
    case GE_JOYBUTTONDOWN:
    case GE_KEYDOWN:
    case GE_MOUSEBUTTONDOWN:
      break;
    default:
      return;
  }

  s_event event = get_event(e);

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    selected = NULL;

    s_profile * next = cfg_controllers[i].current;

    if(cfg_controllers[i].next != NULL)
    {
      next = cfg_controllers[i].next;
    }

    for(j=0; j<MAX_CONFIGURATIONS; ++j)
    {
      s_profile * profile = cfg_controllers[i].profiles + j;

      if (compare_trigger(profile, &event))
      {
        continue;
      }

      if (!up)
      {
        if(next == profile)
        {
          continue;
        }
        if(selected == NULL)
        {
          selected = profile;
        }
        if(selected->index < next->index && j > next->index)
        {
          selected = profile;
        }
      }
      else if(profile->trigger.switch_back)
      {
        if(cfg_controllers[i].next == profile)
        {
          /* cancel the switch */
          cfg_controllers[i].next = NULL;
          cfg_controllers[i].delay = 0;
        }
        else
        {
          s_profile * profile_next = profile->state.next;
          if(profile_next)
          {
            /* remove profile from history */
            profile_next->state.previous = profile->state.previous;
            s_profile * profile_previous = profile->state.previous;
            if(profile_previous)
            {
              profile_previous->state.next = profile->state.next;
            }
            profile->state.next = NULL;
            profile->state.previous = NULL;
          }
          else
          {
            /* switch back */
            selected = profile->state.previous;
          }
        }
        break;
      }
    }
    if(selected != NULL)
    {
      cfg_controllers[i].next = selected;
      if(!up)
      {
        cfg_controllers[i].delay = selected->trigger.delay / (gimx_params.refresh_period / 1000);
      }
      else
      {
        cfg_controllers[i].delay = 0;
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
    s_profile * next = cfg_controllers[i].next;

    if(next != NULL)
    {
      if(!cfg_controllers[i].delay)
      {
        s_profile * current = cfg_controllers[i].current;

        if(next != current)
        {
          if(gimx_params.status)
          {
            gettimeofday(&tv, NULL);

            gprintf(_("%d %ld.%06ld controller %d is switched from configuration %d to %d\n"), i, tv.tv_sec, tv.tv_usec, i, current->index, next->index);
          }

          if(current->state.previous != next)
          {
            current->state.next = next;
            next->state.previous = current;
          }
          else
          {
            current->state.previous = NULL;
            next->state.next = NULL;
          }

          cfg_controllers[i].current = next;

          for(j=0; j<AXIS_MAX; ++j)
          {
            update_stick(i, j);
          }
        }

        cfg_controllers[i].next = NULL;
      }
      else
      {
        cfg_controllers[i].delay--;
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
    if (mc->postpone[event->button.button] < gimx_params.postpone_count)
    {
      ginput_queue_push(event);
      mc->postpone[event->button.button]++;
      ret = 1;
    }
    else
    {
      mc->postpone[event->button.button] = 0;
    }
  }

  s_event e = get_event(event);

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    s_profile * next = cfg_controllers[i].next;
    if(next != NULL)
    {
      if(!compare_trigger(next, &e))
      {
        /* do not postpone the event if it has to trigger a switch back */
        if(!next->trigger.switch_back)
        {
          ginput_queue_push(event);
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

  max_axis = controller_get_max_signed(controller->ctype, axis);
  if(axis_props->props == AXIS_PROP_CENTERED)
  {
    min_axis = -max_axis;
  }
  else
  {
    min_axis = 0;
  }

  multiplier *= controller_get_axis_scale(controller->ctype, axis);
  dz *= controller_get_axis_scale(controller->ctype, axis);

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
  s_intensity* intensity = &axis_intensity[c_id][cfg_controllers[c_id].current->index][axis];
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
  s_intensity* intensity = &axis_intensity[c_id][cfg_controllers[c_id].current->index][axis];
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

  unsigned int device = ginput_get_device_id(event);

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    controller = adapter_get(c_id);
    config = cfg_controllers[c_id].current->index;

    nb_controls = 0;

    switch(event->type)
    {
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
      nb_controls = joystick_buttons[device][c_id][config].nb_mappers;
      break;
      case GE_JOYAXISMOTION:
      nb_controls = joystick_axes[device][c_id][config].nb_mappers;
      break;
      case GE_KEYDOWN:
      case GE_KEYUP:
      nb_controls = keyboard_buttons[device][c_id][config].nb_mappers;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
      nb_controls = mouse_buttons[device][c_id][config].nb_mappers;
      break;
      case GE_MOUSEMOTION:
      nb_controls = mouse_axes[device][c_id][config].nb_mappers;
      break;
    }

    for(control=0; control<nb_controls; ++control)
    {
      switch(event->type)
      {
        case GE_JOYBUTTONDOWN:
          mapper = joystick_buttons[device][c_id][config].mappers+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_JOYBUTTONUP:
          mapper = joystick_buttons[device][c_id][config].mappers+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->jbutton.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_JOYAXISMOTION:
          mapper = joystick_axes[device][c_id][config].mappers+control;
          /*
           * Check that it's the right axis.
           */
          if(mapper->axis != event->jaxis.axis)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            multiplier = mapper->multiplier * controller_get_axis_scale(controller->ctype, axis);
            exp = mapper->exponent;
            dead_zone = mapper->dead_zone * controller_get_axis_scale(controller->ctype, axis);
            value = event->jaxis.value;
            max_axis = controller_get_max_signed(controller->ctype, axis);
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
              s_js_corr * corr = get_js_corr(event->jaxis.which, event->jaxis.axis);
              if(corr != NULL)
              {
                value = value > corr->coef[0] ? (value < corr->coef[1] ? 0 :
                        ((corr->coef[3] * (value - corr->coef[1])) >> 14)) :
                        ((corr->coef[2] * (value - corr->coef[0])) >> 14);
              }
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
          mapper = keyboard_buttons[device][c_id][config].mappers+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_KEYUP:
          mapper = keyboard_buttons[device][c_id][config].mappers+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->key.keysym)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_MOUSEMOTION:
          mapper = mouse_axes[device][c_id][config].mappers+control;
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
          if(axis >= 0 && axis < AXIS_MAX)
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
              mode = cal_get_mouse(device, config)->options.mode;
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
              max_axis = controller_get_max_signed(controller->ctype, axis);
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
          mapper = mouse_buttons[device][c_id][config].mappers+control;
          /*
           * Check that it's the right button.
           */
          if(mapper->button != event->button.button)
          {
            continue;
          }
          controller->send_command = 1;
          axis = mapper->axis_props.axis;
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_dbutton_axis(mapper, c_id, axis);
          }
          break;
        case GE_MOUSEBUTTONUP:
          mapper = mouse_buttons[device][c_id][config].mappers+control;
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
          if(axis >= 0 && axis < AXIS_MAX)
          {
            update_ubutton_axis(mapper, c_id, axis);
          }
          break;
      }
    }
  }
}

void cfg_clean()
{
  s_mapper_table* table;
  int i, j, k;
  for(i=0; i<MAX_DEVICES; ++i)
  {
    for(j=0; j<MAX_CONTROLLERS; ++j)
    {
      for(k=0; k<MAX_CONFIGURATIONS; ++k)
      {
        table = cfg_get_keyboard_buttons(i, j, k);
        free(table->mappers);
        table->mappers = NULL;
        table->nb_mappers = 0;
        table = cfg_get_mouse_buttons(i, j, k);
        free(table->mappers);
        table->mappers = NULL;
        table->nb_mappers = 0;
        table = cfg_get_mouse_axes(i, j, k);
        free(table->mappers);
        table->mappers = NULL;
        table->nb_mappers = 0;
        table = cfg_get_joystick_buttons(i, j, k);
        free(table->mappers);
        table->mappers = NULL;
        table->nb_mappers = 0;
        table = cfg_get_joystick_axes(i, j, k);
        free(table->mappers);
        table->mappers = NULL;
        table->nb_mappers = 0;
      }
    }
    free(js_corr[i].corr);
    js_corr[i].corr = NULL;
    js_corr[i].nb = 0;
  }
}

void cfg_read_calibration()
{
  int i, j, k;
  s_mapper_table* table;
  s_mapper* p_mapper;
  s_mouse_cal* mcal;
  int found;

  current_mouse = -1;

  for(i=0; i<MAX_DEVICES; ++i)
  {
    found = 0;
    for(j=0; j<MAX_CONTROLLERS && !found; ++j)
    {
      for(k=0; k<MAX_CONFIGURATIONS; ++k)
      {
        table = cfg_get_mouse_axes(i, j, k);
        mcal = cal_get_mouse(i, k);
        /*if(*pp_mapper)
        {
          printf("mouse %u - profile %u - mode %u - bs %u - f %.02f\n", i, k, mcal->mode, mcal->buffer_size, mcal->filter);
        }*/
        for(p_mapper = table->mappers; p_mapper && p_mapper<table->mappers+table->nb_mappers; p_mapper++)
        {
          if(current_mouse < 0)
          {
            current_mouse = i;
          }
          if(p_mapper->axis == 0)
          {
            found = 1;
            cal_set_controller(i, j);
            mcal->mx = &p_mapper->multiplier;
            mcal->ex = &p_mapper->exponent;
            mcal->dzx = &p_mapper->dead_zone;
            mcal->dzs = &p_mapper->shape;
            mcal->rd = DEFAULT_RADIUS;
            mcal->vel = DEFAULT_VELOCITY;
            mcal->dpi = controller_dpi[j];
          }
          else
          {
            found = 1;
            cal_set_controller(i, j);
            mcal->my = &p_mapper->multiplier;
            mcal->ey = &p_mapper->exponent;
            mcal->dzy = &p_mapper->dead_zone;
          }
        }
      }
    }
  }

  if(current_mouse < 0)
  {
    current_mouse = 0;
  }
}
