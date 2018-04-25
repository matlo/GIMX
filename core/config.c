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
#include <gimxinput/include/ginput.h>
#include "calibration.h"
#include "gimx.h"
#include "macros.h"
#include <controller.h>
#include "haptic/haptic_core.h"

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
  s_profile profiles[MAX_PROFILES];
} cfg_controllers[MAX_CONTROLLERS];

/*
 * This lists controller stick intensity modifiers.
 */
static s_intensity axis_intensity[MAX_CONTROLLERS][MAX_PROFILES][AXIS_MAX];

/*
 * This lists controls of each controller profile for all keyboards.
 */
static s_mapper_table keyboard_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_PROFILES];

/*
 * This lists controls of each controller profile for all mice.
 */
static s_mapper_table mouse_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_PROFILES];

static s_mapper_table mouse_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_PROFILES];

/*
 * Used to tweak mouse controls.
 */
static s_mouse_control mouse_control[MAX_DEVICES] = {};

/*
 * This lists controls of each controller profile for all joysticks.
 */
static s_mapper_table joystick_buttons[MAX_DEVICES][MAX_CONTROLLERS][MAX_PROFILES];
static s_mapper_table joystick_axes[MAX_DEVICES][MAX_CONTROLLERS][MAX_PROFILES];

/*
 * FFB tweaks, for each controller and each profile.
 */
static s_haptic_core_tweaks ffb_tweaks[MAX_CONTROLLERS][MAX_PROFILES];

void cfg_set_ffb_tweaks(const s_config_entry * entry)
{
  ffb_tweaks[entry->controller_id][entry->profile_id].invert = entry->params.ffb_tweaks.invert;
  ffb_tweaks[entry->controller_id][entry->profile_id].gain.rumble = entry->params.ffb_tweaks.gain.rumble;
  ffb_tweaks[entry->controller_id][entry->profile_id].gain.constant = entry->params.ffb_tweaks.gain.constant;
  ffb_tweaks[entry->controller_id][entry->profile_id].gain.spring = entry->params.ffb_tweaks.gain.spring;
  ffb_tweaks[entry->controller_id][entry->profile_id].gain.damper = entry->params.ffb_tweaks.gain.damper;
}

const s_haptic_core_tweaks * cfg_get_ffb_tweaks(int controller)
{
  return ffb_tweaks[controller] + cfg_controllers[controller].current->index;
}

void cfg_init_ffb_tweaks()
{
  unsigned int i, j;
  for (i = 0; i < MAX_CONTROLLERS; ++i)
  {
    for (j = 0; j < MAX_PROFILES; ++j)
    {
      ffb_tweaks[i][j].invert = 0;
      ffb_tweaks[i][j].gain.rumble = 100;
      ffb_tweaks[i][j].gain.constant = 100;
      ffb_tweaks[i][j].gain.spring = 100;
      ffb_tweaks[i][j].gain.damper = 100;
    }
  }
}

static struct
{
  unsigned int nb;
  s_js_corr * corr;
} js_corr[MAX_DEVICES] = {};

int cfg_add_js_corr(int joystick, s_js_corr * corr)
{
  void * ptr = realloc(js_corr[joystick].corr, (js_corr[joystick].nb + 1) * sizeof(*(js_corr->corr)));
  if(ptr == NULL)
  {
    gerror("%s:%d %s: realloc failed\n", __FILE__, __LINE__, __func__);
    return -1;
  }
  js_corr[joystick].corr = ptr;
  js_corr[joystick].corr[js_corr[joystick].nb].axis = corr->axis;
  memcpy(js_corr[joystick].corr[js_corr[joystick].nb].coef, corr->coef, sizeof(corr->coef));
  ++(js_corr[joystick].nb);
  return 0;
}

static s_js_corr * get_js_corr(int joystick, int axis)
{
  unsigned int i;
  for (i = 0; i < js_corr[joystick].nb; ++i)
  {
    if(js_corr[joystick].corr[i].axis == axis)
    {
      return js_corr[joystick].corr + i;
    }
  }
  return NULL;
}

s_mapper_table* cfg_get_joystick_axes(int device, int controller, int profile)
{
  return &(joystick_axes[device][controller][profile]);
}

s_mapper_table* cfg_get_joystick_buttons(int device, int controller, int profile)
{
  return &(joystick_buttons[device][controller][profile]);
}

s_mapper_table* cfg_get_mouse_axes(int device, int controller, int profile)
{
  return &(mouse_axes[device][controller][profile]);
}

s_mapper_table* cfg_get_mouse_buttons(int device, int controller, int profile)
{
  return &(mouse_buttons[device][controller][profile]);
}

s_mapper_table* cfg_get_keyboard_buttons(int device, int controller, int profile)
{
  return &(keyboard_buttons[device][controller][profile]);
}

void cfg_set_trigger(s_config_entry* entry)
{
  cfg_controllers[entry->controller_id].profiles[entry->profile_id].trigger.event.button = entry->event.id;
  cfg_controllers[entry->controller_id].profiles[entry->profile_id].trigger.event.device_id = entry->device.id;
  cfg_controllers[entry->controller_id].profiles[entry->profile_id].trigger.event.device_type = entry->device.type;
  cfg_controllers[entry->controller_id].profiles[entry->profile_id].trigger.switch_back = entry->params.trigger.switch_back;
  cfg_controllers[entry->controller_id].profiles[entry->profile_id].trigger.delay = entry->params.trigger.delay;
}

void cfg_set_controller_dpi(int controller, unsigned int dpi)
{
  controller_dpi[controller] = dpi;
}

void cfg_set_axis_intensity(s_config_entry* entry, int axis, s_intensity* intensity)
{
  s_intensity * target = axis_intensity[entry->controller_id][entry->profile_id] + axis;

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
    for (j = 0; j < MAX_PROFILES; ++j)
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
      table = cfg_get_keyboard_buttons(entry->device.id, entry->controller_id, entry->profile_id);
      break;
    case E_DEVICE_TYPE_MOUSE:
      switch(entry->event.type)
      {
        case E_EVENT_TYPE_BUTTON:
          table = cfg_get_mouse_buttons(entry->device.id, entry->controller_id, entry->profile_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          table = cfg_get_mouse_axes(entry->device.id, entry->controller_id, entry->profile_id);
          break;
        default:
          break;
      }
      break;
    case E_DEVICE_TYPE_JOYSTICK:
      switch(entry->event.type)
      {
        case E_EVENT_TYPE_BUTTON:
          table = cfg_get_joystick_buttons(entry->device.id, entry->controller_id, entry->profile_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          table = cfg_get_joystick_axes(entry->device.id, entry->controller_id, entry->profile_id);
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
    gerror("failed to allocate mapper\n");
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
    for(k=0; k<MAX_PROFILES && !used; ++k)
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
        mc->merge_x[mc->index] += mc->residue.x;
        mc->merge_y[mc->index] += mc->residue.y;
        /*
         * If no motion was received this iteration, the residual motion vector from the last iteration is reset.
         */
        if (!mc->change)
        {
          mc->residue.x = 0;
          mc->residue.y = 0;
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
        gstatus(_("controller %d profile %d axis %s intensity: %.0f\n"), c_id, cfg_controllers[c_id].current->index, controller_get_axis_name(adapter_get(c_id)->ctype, a_id), axis_intensity[c_id][cfg_controllers[c_id].current->index][a_id].value);
      }
    }
  }
}

/*
 * Initialize the cfg_controllers table.
 */
void cfg_trigger_init()
{
  int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    cfg_controllers[i].current = cfg_controllers[i].profiles;
    cfg_controllers[i].next = NULL;
    cfg_controllers[i].delay = 0;
    for(j=0; j<MAX_PROFILES; ++j)
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
 * Check if current profile of controllers need to be updated.
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

    s_profile * root = next;
    while (root->state.previous != NULL && root->state.previous != next) {
        root = root->state.previous;
    }

    for(j=0; j<MAX_PROFILES; ++j)
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
        if((selected->index < next->index && j > next->index) || (selected->index < root->index && j > root->index))
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
        else if (cfg_controllers[i].current == profile)
        {
          /* switch back */
          selected = profile->state.previous;
          break;
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
        }
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
 * Check if a profile activation has to be performed.
 */
void cfg_profile_activation()
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

            gstatus(_("%d %ld.%06ld controller %d is switched from profile %d to %d\n"), i, tv.tv_sec, tv.tv_usec, i, current->index, next->index);
          }

          if(current->state.previous != next && next->trigger.switch_back)
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

          const s_haptic_core_tweaks * tweaks = cfg_get_ffb_tweaks(i);
          
          adapter_set_haptic_tweaks(i, tweaks);

          unsigned int k;
          for (k = 0; k < sizeof(mouse_control) / sizeof(*mouse_control); ++k)
          {
            mouse_control[k].residue.x = mouse_control[k].residue.y = 0;
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
 * This function also postpones mouse button up events in case a delayed profile toggle is triggered.
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

static void mouse2axis1d(int device, s_adapter* controller, const s_mapper * mapper, double mx, double my, e_mouse_mode mode, s_mouse_control * mc)
{
  double z = 0;
  double * motion_residue = NULL;
  double ztrunk = 0;
  double val = 0;
  int min_axis, max_axis;
  int new_state;

  int which = mapper->axis;
  int axis = mapper->axis_props.axis;
  char props = mapper->axis_props.props;
  double exp = mapper->exponent;
  double multiplier = mapper->multiplier;
  double dz = mapper->dead_zone;

  max_axis = controller_get_max_signed(controller->ctype, axis);
  min_axis = (props == AXIS_PROP_CENTERED) ? -max_axis : 0;

  multiplier *= controller_get_axis_scale(controller->ctype, axis);
  dz *= controller_get_axis_scale(controller->ctype, axis);

  if(which == AXIS_X)
  {
    val = mx;
    if(device == current_mouse && current_cal == DZX)
    {
      controller->axis[axis] = copysign(dz, val);
      mc->residue.x = 0;
      return;
    }
    motion_residue = &mc->residue.x;
  }
  else if(which == AXIS_Y)
  {
    val = my;
    if(device == current_mouse && current_cal == DZY)
    {
      controller->axis[axis] = copysign(dz, val);
      mc->residue.y = 0;
      return;
    }
    motion_residue = &mc->residue.y;
  }

  val *= gimx_params.frequency_scale;

  if(val != 0)
  {
    z = copysign(multiplier * pow(fabs(val), exp), val);
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
      new_state -= (2 * dz);
    }
    if(new_state < 0 && new_state > -dz)
    {
      new_state += (2 * dz);
    }
    controller->axis[axis] = clamp(min_axis, new_state, max_axis);
  }

  if(val != 0 && ztrunk != 0)
  {
    //printf("ztrunk: %.4f\n", ztrunk);
    /*
     * Compute the motion that wasn't applied due to the double to integer conversion.
     */
    *motion_residue = copysign(fabs(val) - pow(fabs(ztrunk)/multiplier, 1/exp), val);
    if(fabs(*motion_residue) < 0.0039)//allow 256 subpositions
    {
      *motion_residue = 0;
    }
    //printf("motion_residue: %.4f\n", motion_residue);
  }
}

static double update_axis(int * axis, double dead_zone, double z, double max_axis, double min_axis)
{
  double residue = 0;

  if (fabs(z) >= 1)
  {
    z += dead_zone;
    *axis = z;
    if (*axis > min_axis && *axis < max_axis)
    {
      residue = z - *axis;
    }
  }
  else
  {
    *axis = 0;
    residue = z;
  }

  if (gimx_params.debug.config)
  {
    ginfo("z: %.2f dz: %.2f r: %.2f a: %d maa: %.2f mia: %.2f\n", z, dead_zone, residue, *axis, max_axis, min_axis);
  }

  return residue;
}

/*void update_residue(double z_x, double z_y, double multiplier_x, double multiplier_y, double exponent, s_mouse_control * mc)
{
  if (z_x == 0 && z_y == 0)
  {
    mc->residue.x = 0;
    mc->residue.y = 0;
  }
  else
  {
    double azx = fabs(z_x);
    double azy = fabs(z_y);
    double hypotenuse = hypot(azx, azy);
    double angle_cos = azx / hypotenuse;
    double angle_sin = azy / hypotenuse;
    if (gimx_params.debug.config)
    {
      ginfo("hypotenuse: %.2f angle_cos: %.2f angle_sin: %.2f\n", hypotenuse, angle_cos, angle_sin);
    }
    if (z_x == 0)
    {
      mc->residue.x = 0;
      mc->residue.y = copysign(pow(azy / (multiplier_y * pow(gimx_params.frequency_scale, exponent)), 1 / exponent), z_y);
    }
    else if (z_y == 0)
    {
      mc->residue.x = copysign(pow(azx / (multiplier_x * pow(gimx_params.frequency_scale, exponent)), 1 / exponent), z_x);
      mc->residue.y = 0;
    }
    else
    {
      double A2 = (multiplier_x * azy * azy) / (multiplier_y * azx * azx);
      mc->residue.x = copysign(pow(azx / (multiplier_x * pow(gimx_params.frequency_scale, exponent) * pow(1 + A2, exponent / 2) * angle_cos), 1 / exponent), z_x);
      mc->residue.y = copysign(pow(azy / (multiplier_y * pow(gimx_params.frequency_scale, exponent) * pow(1 + 1 / A2, exponent / 2) * angle_sin), 1 / exponent), z_y);
    }
  }

  if (gimx_params.debug.config)
  {
    ginfo("z_x: %.2f z_y: %.2f r_x: %.4f r_y: %.4f m_x: %.2f, m_y: %.2f\n", z_x, z_y, mc->residue.x, mc->residue.y, multiplier_x, multiplier_y);
  }
}*/

static int calibrate_dead_zone(int device, int * axis_x, int * axis_y, double dead_zone_x, double dead_zone_y, s_mouse_control * mc)
{
  if(device == current_mouse)
  {
    if (current_cal == DZX)
    {
      *axis_x = dead_zone_x;
      mc->residue.x = 0;
      *axis_y = 0;
      mc->residue.y = 0;
      return 1;
    }
    else if (current_cal == DZY)
    {
      *axis_y = dead_zone_y;
      mc->residue.y = 0;
      *axis_x = 0;
      mc->residue.x = 0;
      return 1;
    }
    else if(current_cal == DZS)
    {
      *axis_x = dead_zone_x;
      mc->residue.x = 0;
      *axis_y = dead_zone_y;
      mc->residue.y = 0;
      return 1;
    }
  }
  return 0;
}

static void mouse2axis2d(int device, s_adapter* controller, const s_mapper * mapper_x, double x, double y, s_mouse_control * mc)
{
  s_mapper * mapper_y = mapper_x->other;

  if (gimx_params.debug.config)
  {
    ginfo("mouse input: %.2f %.2f\n", x, y);
  }

  if (x == 0 && y == 0)
  {
    controller->axis[mapper_x->axis_props.axis] = controller->axis[mapper_y->axis_props.axis] = 0;
    mc->residue.x = mc->residue.y = 0;
    return;
  }

  int max_axis = controller_get_max_signed(controller->ctype, mapper_x->axis);
  int min_axis = (mapper_x->axis_props.props == AXIS_PROP_CENTERED) ? -max_axis : 0;

  double axis_scale = controller_get_axis_scale(controller->ctype, mapper_x->axis);

  double multiplier_x = mapper_x->multiplier * axis_scale;
  double exponent = mapper_x->exponent;
  double multiplier_y = mapper_y->multiplier * axis_scale;

  double dead_zone_x = copysign(mapper_x->dead_zone * axis_scale, x);
  double dead_zone_y = copysign(mapper_y->dead_zone * axis_scale, y);
  e_shape shape = mapper_x->shape;

  double hypotenuse = hypot(x, y);
  double angle_cos = fabs(x) / hypotenuse;
  double angle_sin = fabs(y) / hypotenuse;

  if (gimx_params.debug.config)
  {
    ginfo("hyp: %.4f cos: %.4f sin: %.4f\n", hypotenuse, angle_cos, angle_sin);
  }

  if(x && y && shape == E_SHAPE_CIRCLE)
  {
    dead_zone_x *= angle_cos;
    dead_zone_y *= angle_sin;
  }

  int * axis_x = controller->axis + mapper_x->axis_props.axis;
  int * axis_y = controller->axis + mapper_y->axis_props.axis;

  if (calibrate_dead_zone(device, axis_x, axis_y, dead_zone_x, dead_zone_y, mc) != 0)
  {
    return;
  }

  double norm = hypotenuse * gimx_params.frequency_scale;

  double z = pow(norm, exponent);

  double z_x = copysign(multiplier_x * z * angle_cos, x);
  double z_y = copysign(multiplier_y * z * angle_sin, y);

  /*double r_x =*/ update_axis(axis_x, dead_zone_x, z_x, max_axis, min_axis);
  /*double r_y =*/ update_axis(axis_y, dead_zone_y, z_y, max_axis, min_axis);

  /*if (gimx_params.subpositions && mc->change)
  {
    update_residue(r_x, r_y, multiplier_x, multiplier_y, exponent, mc);
  }*/
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
  unsigned int profile;
  unsigned int c_id;
  unsigned int control;
  int threshold;
  double multiplier;
  double exp;
  double dead_zone;
  int value = 0;
  double fvalue = 0;
  unsigned int nb_controls = 0;
  double mx;
  double my;
  s_mouse_control* mc;
  int min_axis, max_axis;
  e_mouse_mode mode;
  s_adapter* controller;

  unsigned int device = ginput_get_device_id(event);

  for(c_id=0; c_id<MAX_CONTROLLERS; ++c_id)
  {
    controller = adapter_get(c_id);
    profile = cfg_controllers[c_id].current->index;

    nb_controls = 0;

    switch(event->type)
    {
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
      nb_controls = joystick_buttons[device][c_id][profile].nb_mappers;
      break;
      case GE_JOYAXISMOTION:
      nb_controls = joystick_axes[device][c_id][profile].nb_mappers;
      break;
      case GE_KEYDOWN:
      case GE_KEYUP:
      nb_controls = keyboard_buttons[device][c_id][profile].nb_mappers;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
      nb_controls = mouse_buttons[device][c_id][profile].nb_mappers;
      break;
      case GE_MOUSEMOTION:
      nb_controls = mouse_axes[device][c_id][profile].nb_mappers;
      break;
    }

    for(control=0; control<nb_controls; ++control)
    {
      switch(event->type)
      {
        case GE_JOYBUTTONDOWN:
          mapper = joystick_buttons[device][c_id][profile].mappers+control;
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
          mapper = joystick_buttons[device][c_id][profile].mappers+control;
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
          mapper = joystick_axes[device][c_id][profile].mappers+control;
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
          mapper = keyboard_buttons[device][c_id][profile].mappers+control;
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
          mapper = keyboard_buttons[device][c_id][profile].mappers+control;
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
          mapper = mouse_axes[device][c_id][profile].mappers+control;
          if (mapper->axis == AXIS_Y && mapper->other != NULL)
          {
            continue; // processing is done when handling AXIS_X
          }
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
              mode = cal_get_mouse(device, profile)->options.mode;
              if (mapper->other && mode == E_MOUSE_MODE_AIMING)
              {
                if (mapper->axis == AXIS_X)
                {
                  mouse2axis2d(device, controller, mapper, mx, my, mc);
                }
                else
                {
                  continue;
                }
              }
              else
              {
                mouse2axis1d(device, controller, mapper, mx, my, mode, mc);
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
          mapper = mouse_buttons[device][c_id][profile].mappers+control;
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
          mapper = mouse_buttons[device][c_id][profile].mappers+control;
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
      for(k=0; k<MAX_PROFILES; ++k)
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
      for(k=0; k<MAX_PROFILES; ++k)
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

void cfg_pair_mouse_mappers()
{
  int i, j, k;
  for(i = 0; i < MAX_DEVICES; ++i)
  {
    for(j = 0; j < MAX_CONTROLLERS; ++j)
    {
      for(k = 0; k < MAX_PROFILES; ++k)
      {
        s_mapper_table* table = cfg_get_mouse_axes(i, j, k);
        int l, m;
        for(l = 0; l < table->nb_mappers; ++l)
        {
          for(m = l + 1; m < table->nb_mappers; ++m)
          {
            s_mapper * lmapper = table->mappers + l;
            s_mapper * mmapper = table->mappers + m;
            if ((lmapper->axis == AXIS_X && mmapper->axis == AXIS_Y)
                    || (lmapper->axis == AXIS_Y && mmapper->axis == AXIS_X))
            {
              if ((lmapper->axis_props.axis == rel_axis_lstick_x && mmapper->axis_props.axis == rel_axis_lstick_y)
                  || (lmapper->axis_props.axis == rel_axis_lstick_y && mmapper->axis_props.axis == rel_axis_lstick_x)
                  || (lmapper->axis_props.axis == rel_axis_rstick_y && mmapper->axis_props.axis == rel_axis_rstick_x)
                  || (lmapper->axis_props.axis == rel_axis_rstick_y && mmapper->axis_props.axis == rel_axis_rstick_x))
              {
                // two mouse axes (x and y) are mapped to a stick => pair the mappers
                lmapper->other = mmapper;
                mmapper->other = lmapper;
              }
            }
          }
        }
      }
    }
  }
}

