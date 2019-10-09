/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <limits.h>
#include "macros.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include "gimx.h"
#include "config.h"
#include <controller.h>
#include <gimxinput/include/ginput.h>
#include "../directories.h"
#include <gimxfile/include/gfile.h>

#define MACRO_CONFIGS_FILE "configs.txt"

#define MAX_LINE_TOKENS 4

/* This is the default delay with KEY/JBUTTON/MBUTTON commands. */
#define DEFAULT_DELAY 50

typedef struct
{
  short min;
  short max;
} s_axis_range;

typedef struct
{
  GE_Event event; // the event that starts the macro
  s_axis_range range; // for axis events, the axis range
} s_event_id;

typedef struct
{
  s_event_id id;
  int macro_index;
  int event_index;
} s_running_macro;

/*
 * This table contains pending macros.
 * Dynamically allocated.
 */
static s_running_macro * running_macro = NULL;
static unsigned int running_macro_nb;

#define ACTIVE_OFF 0
#define ACTIVE_ON  1

#define TOGGLE_NO  0
#define TOGGLE_YES 1

typedef struct {
  s_event_id id;
  s_event_id trigger; // the event that enables/disables the macro
  unsigned char active; // tells if the macro is enabled or not
  unsigned char toggle; // TOGGLE_YES: the trigger enables/disables only this macro
                        // TOGGLE_NO: the trigger also disables the macros that have toggle set to TOGGLE_NO
  GE_Event * events;
  int nb_events; //The size of the table.
} s_macro;

static GE_Event * axis_values = NULL;
static int axis_values_nb = 0;

/*
 * This table is used to store all the macros that are read from script files at the initialization of the process.
 */
static s_macro * macros = NULL;
static int macros_nb = 0;

/*
 * Cleans macro_table.
 * Frees all allocated blocks pointed by macro_table.
 * Frees running_macro table.
 */
void macros_clean() {
  free(running_macro);
  running_macro = NULL;
  int i;
  for(i = 0; i < macros_nb; ++i)
  {
    free(macros[i].events);
    macros[i].events = NULL;
  }
  free(macros);
  macros = NULL;
  free(axis_values);
  axis_values = NULL;
  axis_values_nb = 0;
}

/*
 * Allocates an element and initializes it to 0.
 */
static int allocate_event(s_macro * pt) {
  void * ptr = realloc(pt->events, sizeof(GE_Event) * (pt->nb_events + 1));
  if(ptr)
  {
    pt->events = ptr;
    memset(pt->events + pt->nb_events, 0x00, sizeof(GE_Event));
    pt->nb_events++;
    return 0;
  }
  else
  {
    gerror("%s:%d realloc failed\n", __FILE__, __LINE__);
    return -1;
  }
}

#define ALLOCATE_EVENT_OR_FAIL \
    if(allocate_event(pcurrent) == -1) \
    { \
      return -1; \
    }

static GE_Event * get_last_event(GE_Event * event)
{
  GE_Event * last = NULL;
  for(last = axis_values; last < axis_values + axis_values_nb; ++last)
  {
    if(event->type != last->type || event->which != last->which)
    {
      continue;
    }
    if(event->type == GE_MOUSEMOTION)
    {
      break;
    }
    if(event->type == GE_JOYAXISMOTION)
    {
      if(event->jaxis.axis == last->jaxis.axis)
      {
        break;
      }
    }
  }
  if(last == axis_values + axis_values_nb)
  {
    return NULL;
  }
  return last;
}

static void save_axis(GE_Event * event)
{
  if(event->type != GE_MOUSEMOTION && event->type != GE_JOYAXISMOTION)
  {
    return;
  }
  GE_Event * last = get_last_event(event);
  if(last)
  {
    *last = *event;
  }
  else
  {
    void * ptr = realloc(axis_values, (axis_values_nb + 1) * sizeof(*axis_values));
    if(ptr)
    {
      axis_values = ptr;
      axis_values[axis_values_nb] = *event;
      ++axis_values_nb;
    }
    else
    {
      gwarn("%s:%d realloc failed\n", __FILE__, __LINE__);
    }
  }
}

int is_rising_edge(short current, short last, s_axis_range * range)
{
  if((current >= range->min && current <= range->max)
      && (last < range->min || last > range->max))
  {
    return 1;
  }
  return 0;
}

static int compare_events(GE_Event * event, s_event_id * id)
{
  if(event->type != id->event.type)
  {
    return 1;
  }

  switch(event->type)
  {
    case GE_KEYDOWN:
    case GE_KEYUP:
      if(event->key.keysym == id->event.key.keysym)
      {
        return 0;
      }
      else
      {
        return 1;
      }
    case GE_MOUSEBUTTONDOWN:
    case GE_MOUSEBUTTONUP:
      if(event->button.button == id->event.button.button)
      {
        return 0;
      }
      else
      {
        return 1;
      }
    case GE_JOYBUTTONDOWN:
    case GE_JOYBUTTONUP:
      if(event->jbutton.button == id->event.jbutton.button)
      {
        return 0;
      }
      else
      {
        return 1;
      }
    default:
      break;
  }

  GE_Event * last = get_last_event(event);
  if(last)
  {
    switch(event->type)
    {
      case GE_MOUSEMOTION:
        if(id->event.motion.xrel)
        {
          if(is_rising_edge(event->motion.xrel, last->motion.xrel, &id->range))
          {
            return 0;
          }
          else
          {
            return 1;
          }
        }
        else if(id->event.motion.yrel)
        {
          if(is_rising_edge(event->motion.yrel, last->motion.yrel, &id->range))
          {
            return 0;
          }
          else
          {
            return 1;
          }
        }
        break;
      case GE_JOYAXISMOTION:
        if(event->jaxis.axis == id->event.jaxis.axis)
        {
          if(is_rising_edge(event->jaxis.value, last->jaxis.value, &id->range))
          {
            return 0;
          }
          else
          {
            return 1;
          }
        }
        else
        {
          return 1;
        }
        break;
      default:
        break;
    }
  }
  return 1;
}

static s_macro * pcurrent = NULL;

/*
 * \brief Parse a macro definition, allocate a macro and update pcurrent.
 *
 * \brief tokens  the line tokens
 * \brief ntoks   the number of tokens
 *
 * \return 0 if in case of success, -1 in case of error (invalid line, no current macro, allocation error)
 */
static int get_macro(char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks)
{
  int etype = -1;
  int rbutton = 0;
  int raxis = 0;
  s_axis_range range;

  if(ntoks < 2) {
    return -1;
  }

  pcurrent = NULL;

  if(ntoks > 2)
  {
    if(!strncmp(tokens[1], "KEYDOWN", strlen("KEYDOWN")))
    {
      if((rbutton = ginput_key_id(tokens[2])))
      {
        etype = GE_KEYDOWN;
      }
    }
    else if(!strncmp(tokens[1], "KEYUP", strlen("KEYUP")))
    {
      if((rbutton = ginput_key_id(tokens[2])))
      {
        etype = GE_KEYUP;
      }
    }
    else if(!strncmp(tokens[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
    {
      if((rbutton = ginput_mouse_button_id(tokens[2])) >= 0)
      {
        etype = GE_MOUSEBUTTONDOWN;
      }
    }
    else if(!strncmp(tokens[1], "MBUTTONUP", strlen("MBUTTONUP")))
    {
      if((rbutton = ginput_mouse_button_id(tokens[2])) >= 0)
      {
        etype = GE_MOUSEBUTTONUP;
      }
    }
    else if(!strncmp(tokens[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
    {
      if((rbutton = atoi(tokens[2])) >= 0)
      {
        etype = GE_JOYBUTTONDOWN;
      }
    }
    else if(!strncmp(tokens[1], "JBUTTONUP", strlen("JBUTTONUP")))
    {
      if((rbutton = atoi(tokens[2])) >= 0)
      {
        etype = GE_JOYBUTTONUP;
      }
    }
    else if(!strncmp(tokens[1], "MAXIS", strlen("MAXIS")))
    {
      if(ntoks > 3)
      {
        if((raxis = atoi(tokens[2])) >= 0)
        {
          if(sscanf(tokens[3], "[%hd,%hd]", &range.min, &range.max) == 2 && range.min <= range.max)
          {
            etype = GE_MOUSEMOTION;
          }
        }
      }
    }
    else if(!strncmp(tokens[1], "JAXIS", strlen("JAXIS")))
    {
      if(ntoks > 3)
      {
        if((raxis = atoi(tokens[2])) >= 0)
        {
          if(sscanf(tokens[3], "[%hd,%hd]", &range.min, &range.max) == 2 && range.min <= range.max)
          {
            etype = GE_JOYAXISMOTION;
          }
        }
      }
    }
  }
  else
  {
    /*
     * For compatibility with previous macro files.
     */
    if((rbutton = ginput_key_id(tokens[1])))
    {
      etype = GE_KEYDOWN;
    }
  }

  if(etype < 0)
  {
    return -1;
  }

  int ret = 0;

  void * ptr = realloc(macros, (macros_nb + 1) * sizeof(s_macro));
  if(ptr)
  {
    macros = ptr;
    pcurrent = macros + macros_nb;
    memset(pcurrent, 0x00, sizeof(*pcurrent));
    pcurrent->active = ACTIVE_ON;
    pcurrent->id.event.type = etype;
    switch(etype)
    {
      case GE_KEYDOWN:
      case GE_KEYUP:
        pcurrent->id.event.key.keysym = rbutton;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
        pcurrent->id.event.button.button = rbutton;
      break;
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
        pcurrent->id.event.jbutton.button = rbutton;
      break;
      case GE_JOYAXISMOTION:
        pcurrent->id.event.jaxis.axis = raxis;
        pcurrent->id.range = range;
      break;
      case GE_MOUSEMOTION:
        if(raxis == AXIS_X)
        {
          pcurrent->id.event.motion.xrel = 1;
        }
        else if(raxis == AXIS_Y)
        {
          pcurrent->id.event.motion.yrel = 1;
        }
        pcurrent->id.range = range;
      break;
    }
    ++macros_nb;
  }
  else
  {
    gwarn("%s:%d realloc failed\n", __FILE__, __LINE__);
    ret = -1;
  }

  return ret;
}

/*
 * \brief Parse an event definition and add it to pcurrent.
 *
 * \brief tokens  the line tokens
 * \brief ntoks   the number of tokens
 *
 * \return 0 if in case of success, -1 in case of error (invalid line, no current macro, allocation error)
 */
static int get_event(char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks)
{
  int rbutton;
  int raxis;
  int rvalue;
  int delay_nb;
  int i;

  if(!pcurrent || ntoks < 2) {
    return -1;
  }

  int ret = 0;

  if (!strncmp(tokens[0], "KEYDOWN", strlen("KEYDOWN")))
  {
    rbutton = ginput_key_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYDOWN;
    pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
  }
  else if (!strncmp(tokens[0], "KEYUP", strlen("KEYUP")))
  {
    rbutton = ginput_key_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYUP;
    pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
  }
  else if (!strncmp(tokens[0], "KEY", strlen("KEY")))
  {
    rbutton = ginput_key_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYDOWN;
    pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      ALLOCATE_EVENT_OR_FAIL
    }

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYUP;
    pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
  }
  else if (!strncmp(tokens[0], "MBUTTONDOWN", strlen("MBUTTONDOWN")))
  {
    rbutton = ginput_mouse_button_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONDOWN;
    pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
  }
  else if (!strncmp(tokens[0], "MBUTTONUP", strlen("MBUTTONUP")))
  {
    rbutton = ginput_mouse_button_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONUP;
    pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
  }
  else if (!strncmp(tokens[0], "MBUTTON", strlen("MBUTTON")))
  {
    rbutton = ginput_mouse_button_id(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONDOWN;
    pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      ALLOCATE_EVENT_OR_FAIL
    }

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONUP;
    pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
  }
  else if (!strncmp(tokens[0], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
  {
    rbutton = atoi(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONDOWN;
    pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
  }
  else if (!strncmp(tokens[0], "JBUTTONUP", strlen("JBUTTONUP")))
  {
    rbutton = atoi(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONUP;
    pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
  }
  else if (!strncmp(tokens[0], "JBUTTON", strlen("JBUTTON"))) {
    rbutton = atoi(tokens[1]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONDOWN;
    pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      ALLOCATE_EVENT_OR_FAIL
    }

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONUP;
    pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
  }
  else if (!strncmp(tokens[0], "DELAY", strlen("DELAY")))
  {
    delay_nb = ceil((double)atoi(tokens[1]) / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      ALLOCATE_EVENT_OR_FAIL
    }
  }
  else if (!strncmp(tokens[0], "JAXIS", strlen("JAXIS")))
  {
    if(ntoks < 3)
    {
      return -1;
    }

    raxis = atoi(tokens[1]);
    rvalue = atoi(tokens[2]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYAXISMOTION;
    pcurrent->events[pcurrent->nb_events - 1].jaxis.axis = raxis;
    pcurrent->events[pcurrent->nb_events - 1].jaxis.value = rvalue;
  }
  else if (!strncmp(tokens[0], "MAXIS", strlen("MAXIS")))
  {
    if(ntoks < 3)
    {
      return -1;
    }

    raxis = atoi(tokens[1]);
    rvalue = atoi(tokens[2]);

    ALLOCATE_EVENT_OR_FAIL

    pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEMOTION;
    if(raxis == AXIS_X)
    {
      pcurrent->events[pcurrent->nb_events - 1].motion.xrel = rvalue;
    }
    else if(raxis == AXIS_Y)
    {
      pcurrent->events[pcurrent->nb_events - 1].motion.yrel = rvalue;
    }
  }
  else
  {
    ret = -1;
  }

  return ret;
}

/*
 * \brief Parse a trigger definition and add it to pcurrent.
 *
 * \brief tokens  the line tokens
 * \brief ntoks   the number of tokens
 *
 * \return 0 if in case of success, -1 in case of error (invalid line or no current macro)
 */
int get_trigger(char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks)
{
  int etype = -1;
  int rbutton;

  if(!pcurrent || ntoks < 2)
  {
    return -1;
  }

  if(ntoks > 2)
  {
    if(!strncmp(tokens[1], "KEYDOWN", strlen("KEYDOWN")))
    {
      if((rbutton = ginput_key_id(tokens[2])))
      {
        etype = GE_KEYDOWN;
      }
    }
    else if(!strncmp(tokens[1], "KEYUP", strlen("KEYUP")))
    {
      if((rbutton = ginput_key_id(tokens[2])))
      {
        etype = GE_KEYUP;
      }
    }
    else if(!strncmp(tokens[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
    {
      if((rbutton = ginput_mouse_button_id(tokens[2])) >= 0)
      {
        etype = GE_MOUSEBUTTONDOWN;
      }
    }
    else if(!strncmp(tokens[1], "MBUTTONUP", strlen("MBUTTONUP")))
    {
      if((rbutton = ginput_mouse_button_id(tokens[2])) >= 0)
      {
        etype = GE_MOUSEBUTTONUP;
      }
    }
    else if(!strncmp(tokens[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
    {
      if((rbutton = atoi(tokens[2])) >= 0)
      {
        etype = GE_JOYBUTTONDOWN;
      }
    }
    else if(!strncmp(tokens[1], "JBUTTONUP", strlen("JBUTTONUP")))
    {
      if((rbutton = atoi(tokens[2])) >= 0)
      {
        etype = GE_JOYBUTTONUP;
      }
    }
  }
  else
  {
    /*
     * For compatibility with previous macro files.
     */
    if((rbutton = ginput_key_id(tokens[1])))
    {
      etype = GE_KEYDOWN;
    }
  }

  if(etype < 0)
  {
    return -1;
  }

  pcurrent->trigger.event.type = etype;
  switch(etype)
  {
    case GE_KEYDOWN:
    case GE_KEYUP:
      pcurrent->trigger.event.key.keysym = rbutton;
    break;
    case GE_MOUSEBUTTONDOWN:
    case GE_MOUSEBUTTONUP:
      pcurrent->trigger.event.button.button = rbutton;
    break;
    case GE_JOYBUTTONDOWN:
    case GE_JOYBUTTONUP:
      pcurrent->trigger.event.jbutton.button = rbutton;
    break;
  }

  //macros with a trigger are default off
  pcurrent->active = ACTIVE_OFF;

  return 0;
}

/*
 * \brief Parse a toggle definition and add it to pcurrent.
 *
 * \brief tokens  the line tokens
 * \brief ntoks   the number of tokens
 *
 * \return 0 if in case of success, -1 in case of error (invalid line or no current macro)
 */
int get_toggle(char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks)
{
  if(!pcurrent || ntoks < 2)
  {
    return -1;
  }

  if(!strncmp(tokens[1], "YES", strlen("YES")))
  {
    pcurrent->toggle = TOGGLE_YES;
  }

  return 0;
}

/*
 * \brief Parse an init definition and add it to pcurrent.
 *
 * \brief tokens  the line tokens
 * \brief ntoks   the number of tokens
 *
 * \return 0 if in case of success, -1 in case of error (invalid line or no current macro)
 */
int get_init(char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks)
{
  if(!pcurrent || ntoks < 2)
  {
    return -1;
  }

  if(!strncmp(tokens[1], "ON", strlen("ON")))
  {
    pcurrent->active = ACTIVE_ON;
  }
  else if(!strncmp(tokens[1], "OFF", strlen("OFF")))
  {
    pcurrent->active = ACTIVE_OFF;
  }

  return 0;
}

static const struct
{
    const char * key;
    size_t key_len;
    int (*fp) (char tokens[MAX_LINE_TOKENS][LINE_MAX], int ntoks);
} line_process[] =
{
        { "MACRO",   sizeof("MACRO") - 1,   get_macro   },
        { "TRIGGER", sizeof("TRIGGER") - 1, get_trigger },
        { "TOGGLE",  sizeof("TOGGLE") - 1,  get_toggle  },
        { "INIT",    sizeof("INIT") - 1,    get_init    },
};

/*
 * Processes a line of a macro file.
 */
int macros_process_line(const char* line)
{
    int ret;

    char tokens[MAX_LINE_TOKENS][LINE_MAX];

    int ntoks = sscanf(line, "%s %s %s %s", tokens[0], tokens[1], tokens[2], tokens[3]);

    if (ntoks < 0)
    {
      gwarn("failed to scan line: %s", line);
      return -1;
    }

    unsigned int i;
    for (i = 0; i < sizeof(line_process) / sizeof(*line_process); ++i)
    {
        if (!strncmp(line, line_process[i].key, line_process[i].key_len))
        {
            int ret = line_process[i].fp(tokens, ntoks);
            if (ret < 0)
            {
              gwarn("invalid line: %s", line);
            }
            return ret;
        }
    }

    ret = get_event(tokens, ntoks);
    if (ret < 0)
    {
      gwarn("invalid line: %s", line);
    }

    return ret;
}

void dump_event(GE_Event* event, int newline, int axisvalue)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      ginfo("KEYDOWN %s", ginput_key_name(event->key.keysym));
      break;
    case GE_KEYUP:
      ginfo("KEYUP %s", ginput_key_name(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      ginfo("MBUTTONDOWN %s", ginput_mouse_button_name(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      ginfo("MBUTTONUP %s", ginput_mouse_button_name(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        ginfo("MAXIS 0");
        if(axisvalue)
        {
          ginfo(" %d", event->motion.xrel);
        }
      }
      else if(event->motion.yrel)
      {
        ginfo("MAXIS 1");
        if(axisvalue)
        {
          ginfo(" %d", event->motion.yrel);
        }
      }
      break;
    case GE_JOYBUTTONDOWN:
      ginfo("JBUTTONDOWN %d", event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      ginfo("JBUTTONUP %d", event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      ginfo("JAXIS %d", event->jaxis.axis);
      if(axisvalue)
      {
        ginfo(" %d", event->motion.yrel);
      }
      break;
    default:
      newline = 0;
      break;
  }
  if(newline)
  {
    ginfo("\n");
  }
}

static inline void dump_macro_id(s_macro * macro)
{
  dump_event(&macro->id.event, 0, 0);
  if(macro->id.event.type == GE_MOUSEMOTION || macro->id.event.type == GE_JOYAXISMOTION)
  {
    ginfo(" [%hd,%hd]", macro->id.range.min, macro->id.range.max);
  }
  ginfo("\n");
}

/*
 * Displays macro_table.
 */
void dump_scripts() {
  s_macro * macro;
  GE_Event * event;
  int delay_nb;

  for (macro = macros; macro < macros + macros_nb; ++macro) {
    ginfo("MACRO ");
    dump_macro_id(macro);
    if(macro->trigger.event.type)
    {
      ginfo("TRIGGER ");
      dump_event(&macro->trigger.event, 1, 0);
    }
    if(macro->active == ACTIVE_ON)
    {
      ginfo("INIT ON\n");
    }
    else
    {
      ginfo("INIT OFF\n");
    }
    if(macro->toggle == TOGGLE_YES)
    {
      ginfo("TOGGLE YES\n");
    }
    else
    {
      ginfo("TOGGLE NO\n");
    }
    delay_nb = 0;
    for (event = macro->events; event && event < macro->events + macro->nb_events; ++event) {
      if (!event->type) {
        delay_nb++;
      }
      else if(delay_nb) {
        ginfo("DELAY %d\n", delay_nb*(gimx_params.refresh_period/1000));
        delay_nb = 0;
      }
      dump_event(event, 1, 1);
    }
    if(delay_nb) {
      ginfo("DELAY %d\n", delay_nb*(gimx_params.refresh_period/1000));
    }
    ginfo("\n");
  }
}

static char** macro_configs = NULL;
static unsigned int nb_macro_configs = 0;
static unsigned char configs_txt_present = 0;

#define ADD_FILE(FILES, NB_FILES, FILE) \
        char ** ptr = realloc(FILES, (NB_FILES + 1) * sizeof(char*)); \
        if(ptr) { \
            FILES = ptr; \
            FILES[NB_FILES] = strdup(FILE); \
            if(FILES[NB_FILES]) { \
                ++NB_FILES; \
            } else { \
                gwarn("%s:%d strdup failed\n", __FILE__, __LINE__); \
            } \
        } else { \
            gwarn("%s:%d realloc failed\n", __FILE__, __LINE__); \
        }

static void read_configs_txt(const char* dir_path)
{
  char line[LINE_MAX];
  char config[LINE_MAX];
  char macro[LINE_MAX];
  char extra[LINE_MAX];
  char file_path[PATH_MAX];
  FILE* fp;
  int ret;

  /*
   * TODO this produces warning with gcc 9
   * error: ‘%s’ directive output may be truncated writing 11 bytes into a
   * region of size between 1 and 4096.
   * NOTE: PATH_MAX only reflects the maximum length a path can be on Windows.
   * On OSX and GNU Linux, it varies greatly. This link has some useful info:
   * https://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html.
   */
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wformat-truncation"
  snprintf(file_path, sizeof(file_path), "%s%s", dir_path, MACRO_CONFIGS_FILE);
  #pragma GCC diagnostic pop
  fp = gfile_fopen(file_path, "r");
  if (fp)
  {
    if (gimx_params.logfile != NULL) {
      printf("Dump of %s\n", file_path);
    }
    configs_txt_present = 1;
    while (fgets(line, LINE_MAX, fp)) {
      if (gimx_params.logfile != NULL) {
        printf("%s", line);
      }
      if (line[0] != '#') {
        // As file names may contain spaces, use '/' as a separator.
        // File names may not contain '/'.
        ret = sscanf(line, "%[^/]/%[^\n]", config, macro);

        if(ret < 2) {
          // Handle former syntax that used spaces as a separator.
          // This was bad as file names may contain spaces.
          // Lines with more than one space are invalid, because
          // there is no way to tell how to split them.
          ret = sscanf(line, "%s%s%s", config, macro, extra);

          if(ret != 2) {
            gwarn("configs.txt: invalid line: %s", line);
            continue;
          }
        }

        if(!strcmp(config, gimx_params.config_file))
        {
            ADD_FILE(macro_configs, nb_macro_configs, macro)
        }
      }
    }
    fclose(fp);
  }
}

/*
 * Reads macros from script files.
 */
static void read_macros() {
    char line[LINE_MAX];
    FILE* fp;
    char dir_path[PATH_MAX];
    char file_path[PATH_MAX];
    unsigned int i, j;
    unsigned int nb_filenames = 0;
    char** filenames = NULL;

    snprintf(dir_path, sizeof(dir_path), "%s/%s/%s", gimx_params.homedir, GIMX_DIR, MACRO_DIR);

    GFILE_DIR * dirp = gfile_opendir(dir_path);
    if (dirp == NULL)
    {
      if (errno == ENOENT)
      {
        gwarn("can't open macro directory %s\n", dir_path);
      }
      else
      {
        gerror("failed to open %s:", dir_path);
        perror("gfile_opendir");
      }
      return;
    }

    GFILE_DIRENT * d;
    while ((d = gfile_readdir(dirp)))
    {
      char * name = gfile_name(d);
      if (gfile_isfile(dir_path, name) && strcmp(name, MACRO_CONFIGS_FILE))
      {
          ADD_FILE(filenames, nb_filenames, name)
      }
      free(name);
    }

    gfile_closedir(dirp);

    read_configs_txt(dir_path);

    for(i=0; i<nb_filenames; ++i)
    {
      if(configs_txt_present)
      {
        for(j=0; j<nb_macro_configs; ++j)
        {
          if(!strcmp(macro_configs[j], filenames[i]))
          {
            break;
          }
        }
        if(j == nb_macro_configs)
        {
          continue; //skip this macro file.
        }
      }
      //else: no configs.txt => read all macros.

      snprintf(file_path, sizeof(file_path), "%s%s", dir_path, filenames[i]);
      fp = gfile_fopen(file_path, "r");
      if (!fp) {
        gwarn("failed to open %s\n", file_path);
      } else {
        if (gimx_params.logfile != NULL) {
          printf("Dump of %s\n", file_path);
        }
        int has_errors = 0;
        while (fgets(line, LINE_MAX, fp)) {
          if (gimx_params.logfile != NULL) {
            printf("%s", line);
          }
          if (line[0] != '#' && line[0] != '\n' && line[0] != '\r')
          {
            if (macros_process_line(line) < 0)
            {
              has_errors = 1;
            }
          }
        }
        if (has_errors == 1)
        {
          gwarn("failed to process file %s\n", file_path);
        }
        fclose(fp);
        pcurrent = NULL;
      }
    }

    for(i=0; i<nb_filenames; ++i)
    {
      free(filenames[i]);
    }
    free(filenames);

    for(i=0; i<nb_macro_configs; ++i)
    {
      free(macro_configs[i]);
    }
    free(macro_configs);
}

/*
 * Initializes macro_table and reads macros from macro files.
 */
void macros_init()
{
    read_macros();
    if(gimx_params.debug.macros)
    {
      dump_scripts();
    }
}

static void macro_unalloc(int index)
{
  memmove(running_macro+index, running_macro+index+1, (running_macro_nb-index-1)*sizeof(s_running_macro));
  --running_macro_nb;
  void * ptr = realloc(running_macro, running_macro_nb*sizeof(s_running_macro));
  if(ptr || !running_macro_nb)
  {
    running_macro = ptr;
  }
  else
  {
    gwarn("%s:%d realloc failed\n", __FILE__, __LINE__);
  }
}

/*
 * Unregister a macro.
 */
static int macro_delete(GE_Event* event)
{
  unsigned int i;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(!compare_events(event, &macros[running_macro[i].macro_index].id))
    {
      if(ginput_get_device_id(&running_macro[i].id.event) == ginput_get_device_id(event))
      {
        macro_unalloc(i);
        return 1;
      }
    }
  }
  return 0;
}

/*
 * Register a new macro.
 */
static void macro_add(GE_Event* event, int macro)
{
  void * ptr = realloc(running_macro, (running_macro_nb+1)*sizeof(s_running_macro));
  if(ptr)
  {
    running_macro = ptr;
    running_macro[running_macro_nb].id.event = *event;
    running_macro[running_macro_nb].id.range = macros[macro].id.range;
    running_macro[running_macro_nb].macro_index = macro;
    running_macro[running_macro_nb].event_index = 0;
    running_macro_nb++;
  }
  else
  {
    gwarn("%s:%d realloc failed\n", __FILE__, __LINE__);
  }
}

/*
 * Start or stop a macro.
 */
void macro_lookup(GE_Event* event)
{
  int i, j;
  for(i=0; i<macros_nb; ++i)
  {
    if(macros[i].trigger.event.type != GE_NOEVENT)
    {
      /*
       * Check if macro has to be activated.
       */
      if(!compare_events(event, &macros[i].trigger))
      {
        if(macros[i].toggle == TOGGLE_NO)
        {
          if(macros[i].active == ACTIVE_OFF)
          {
            if (gimx_params.debug.macros)
            {
              ginfo("enable macro: ");
              dump_event(&macros[i].id.event, 1, 0);
            }
            macros[i].active = ACTIVE_ON;
            /*
             * Disable macros that have a different activation trigger.
             */
            for(j=0; j<macros_nb; ++j)
            {
              if(macros[j].trigger.event.type == GE_NOEVENT)
              {
                continue;
              }
              if(compare_events(event, &macros[j].trigger)
                 && macros[j].toggle == TOGGLE_NO
                 && macros[j].active == ACTIVE_ON)
              {
                if (gimx_params.debug.macros)
                {
                  ginfo("disable macro: ");
                  dump_event(&macros[j].id.event, 1, 0);
                }
                macros[j].active = ACTIVE_OFF;
              }
            }
          }
        }
        else
        {
          if(macros[i].active == ACTIVE_OFF)
          {
            if (gimx_params.debug.macros)
            {
              ginfo("enable macro: ");
              dump_event(&macros[i].id.event, 1, 0);
            }
            macros[i].active = ACTIVE_ON;
          }
          else
          {
            if (gimx_params.debug.macros)
            {
              ginfo("disable macro: ");
              dump_event(&macros[i].id.event, 1, 0);
            }
            macros[i].active = ACTIVE_OFF;
          }
        }
      }
    }
    if(!compare_events(event, &macros[i].id))
    {
      if(macros[i].active == ACTIVE_ON)
      {
        /*
         * Start or stop a macro.
         */
        if(!macro_delete(event))
        {
          if (gimx_params.debug.macros)
          {
            ginfo("start macro: ");
            dump_macro_id(macros+i);
          }
          macro_add(event, i);
        }
        else
        {
          if (gimx_params.debug.macros)
          {
            ginfo("stop macro: ");
            dump_macro_id(macros+i);
          }
        }
      }
    }
  }
  save_axis(event);
}

int get_event_device_type(GE_Event* ev)
{
  switch(ev->type)
  {
    case GE_KEYDOWN:
    case GE_KEYUP:
      return E_DEVICE_TYPE_KEYBOARD;
    break;
    case GE_MOUSEBUTTONDOWN:
    case GE_MOUSEBUTTONUP:
    case GE_MOUSEMOTION:
      return E_DEVICE_TYPE_MOUSE;
    break;
    case GE_JOYBUTTONDOWN:
    case GE_JOYBUTTONUP:
    case GE_JOYAXISMOTION:
      return E_DEVICE_TYPE_JOYSTICK;
    break;
  }
  return E_DEVICE_TYPE_UNKNOWN;
}

/*
 * Generate events for pending macros and return the number of running macros.
 */
unsigned int macro_process()
{
  unsigned int i, j;
  int dtype1, dtype2, did;
  GE_Event event;
  for(i=0; i<running_macro_nb; ++i)
  {
    while(running_macro[i].event_index < macros[running_macro[i].macro_index].nb_events)
    {
      event = macros[running_macro[i].macro_index].events[running_macro[i].event_index];
      /*
       * Find out the device that will be the source of the generated event.
       */
      dtype1 = get_event_device_type(&event);
      dtype2 = get_event_device_type(&running_macro[i].id.event);
      did = ginput_get_device_id(&running_macro[i].id.event);
      if(dtype1 != E_DEVICE_TYPE_UNKNOWN && dtype2 != E_DEVICE_TYPE_UNKNOWN && did >= 0)
      {
        /*
         * Get the controller for the device that started the macro.
         */
        int controller = adapter_get_controller(dtype2, did);
        if(controller < 0)
        {
          /*
           * No controller found => find the first device of the same type.
           */
          controller = 0;
          for(j=0; j<MAX_CONTROLLERS; ++j)
          {
            if(adapter_get_device(dtype1, j) >= 0)
            {
              controller = j;
              break;
            }
          }
        }
        did = adapter_get_device(dtype1, controller);
        if(did < 0)
        {
          did = 0;
        }
        event.which = did;
        ginput_queue_push(&event);
      }

      // next event
      running_macro[i].event_index++;

      if(event.type == GE_NOEVENT)
      {
        // that's all for now
        break;
      }
    }
    if(running_macro[i].event_index == macros[running_macro[i].macro_index].nb_events)
    {
      macro_unalloc(i);
      i--;
    }
  }
  return running_macro_nb;
}
