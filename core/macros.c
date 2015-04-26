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
#include <math.h>
#include "gimx.h"
#include "config.h"
#include <adapter.h>
#include <GE.h>
#include "../directories.h"

#ifdef WIN32
#include <sys/stat.h>
#define LINE_MAX 1024
#endif

#define MACRO_CONFIGS_FILE "configs.txt"

static unsigned char debug = 0;

/* This is the default delay with KEY/JBUTTON/MBUTTON commands. */
#define DEFAULT_DELAY 50

typedef struct
{
  GE_Event event;
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

typedef struct
{
  short min;
  short max;
} s_axis_range;

typedef struct
{
  GE_Event event; // the event that starts the macro
  s_axis_range range; // for axis events, the axis range
} s_macro_id;

typedef struct {
  s_macro_id id;
  GE_Event trigger; // the event that enables/disables the macro
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
}

/*
 * Allocates an element and initializes it to 0.
 */
int allocate_event(s_macro * pt) {
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
    fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
    return -1;
  }
}

int compare_events(GE_Event* e1, GE_Event* e2)
{
  int ret = 1;
  if(e1->type == e2->type)
  {
    switch(e1->type)
    {
      case GE_KEYDOWN:
      case GE_KEYUP:
        if(e1->key.keysym == e2->key.keysym)
        {
          ret = 0;
        }
        break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
        if(e1->button.button == e2->button.button)
        {
          ret = 0;
        }
        break;
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
        if(e1->jbutton.button == e2->jbutton.button)
        {
          ret = 0;
        }
        break;
      case GE_MOUSEMOTION:
        ret = 0;
        break;
      case GE_JOYAXISMOTION:
        if(e1->jaxis.axis == e2->jaxis.axis)
        {
          ret = 0;
        }
        break;
      case GE_NOEVENT:
        ret = 0;
        break;
      default:
        break;
    }
  }
  return ret;
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
      fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
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

static int compare_macro_ids(GE_Event * event, s_macro_id * id)
{
  if(compare_events(event, &id->event))
  {
    return 1;
  }
  GE_Event * last = get_last_event(event);
  if(last)
  {
    switch(event->type)
    {
      case GE_MOUSEMOTION:
        if(id->event.motion.xrel && is_rising_edge(event->motion.xrel, last->motion.xrel, &id->range))
        {
          return 0;
        }
        else if(id->event.motion.yrel && is_rising_edge(event->motion.yrel, last->motion.yrel, &id->range))
        {
          return 0;
        }
        break;
      case GE_JOYAXISMOTION:
        if(is_rising_edge(event->jaxis.value, last->jaxis.value, &id->range))
        {
          return 0;
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
 * Try to parse the line as a macro definition.
 * If the line matches a macro definition:
 *  - a new macro is allocated
 *  - a pointer to this macro is returned
 * Else:
 *  - NULL is returned
 */
static s_macro * get_macro(const char* line)
{
  int etype = -1;
  int rbutton, raxis;
  s_axis_range range;
  char argument[4][LINE_MAX];
  s_macro * pt;
  int ret = sscanf(line, "%s %s %s %s", argument[0], argument[1], argument[2], argument[3]);
  
  if(ret < 2) {
    /* invalid line */
    return NULL;
  }
  
  if (!strncmp(argument[0], "MACRO", strlen("MACRO")))
  {
    if(ret > 2)
    {
      if(!strncmp(argument[1], "KEYDOWN", strlen("KEYDOWN")))
      {
        if((rbutton = GE_KeyId(argument[2])))
        {
          etype = GE_KEYDOWN;
        }
      }
      else if(!strncmp(argument[1], "KEYUP", strlen("KEYUP")))
      {
        if((rbutton = GE_KeyId(argument[2])))
        {
          etype = GE_KEYUP;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
      {
        if((rbutton = GE_MouseButtonId(argument[2])) >= 0)
        {
          etype = GE_MOUSEBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONUP", strlen("MBUTTONUP")))
      {
        if((rbutton = GE_MouseButtonId(argument[2])) >= 0)
        {
          etype = GE_MOUSEBUTTONUP;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = GE_JOYBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONUP", strlen("JBUTTONUP")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = GE_JOYBUTTONUP;
        }
      }
      else if(!strncmp(argument[1], "MAXIS", strlen("MAXIS")))
      {
        if(ret > 3)
        {
          if((raxis = atoi(argument[2])) >= 0)
          {
            if(sscanf(argument[3], "[%hd,%hd]", &range.min, &range.max) == 2 && range.min <= range.max)
            {
              etype = GE_MOUSEMOTION;
            }
          }
        }
      }
      else if(!strncmp(argument[1], "JAXIS", strlen("JAXIS")))
      {
        if(ret > 3)
        {
          if((raxis = atoi(argument[2])) >= 0)
          {
            if(sscanf(argument[3], "[%hd,%hd]", &range.min, &range.max) == 2 && range.min <= range.max)
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
      if((rbutton = GE_KeyId(argument[1])))
      {
        etype = GE_KEYDOWN;
      }
    }
    
    if(etype < 0)
    {
      return NULL;
    }
    
    void * ptr = realloc(macros, (macros_nb + 1) * sizeof(s_macro));
    if(ptr)
    {
      macros = ptr;
      pt = macros + macros_nb;
      memset(pt, 0x00, sizeof(*pt));
      pt->active = ACTIVE_ON;
      pt->id.event.type = etype;
      switch(etype)
      {
        case GE_KEYDOWN:
        case GE_KEYUP:
          pt->id.event.key.keysym = rbutton;
        break;
        case GE_MOUSEBUTTONDOWN:
        case GE_MOUSEBUTTONUP:
          pt->id.event.button.button = rbutton;
        break;
        case GE_JOYBUTTONDOWN:
        case GE_JOYBUTTONUP:
          pt->id.event.jbutton.button = rbutton;
        break;
        case GE_JOYAXISMOTION:
          pt->id.event.jaxis.axis = raxis;
          pt->id.range = range;
        break;
        case GE_MOUSEMOTION:
          if(raxis == AXIS_X)
          {
            pt->id.event.motion.xrel = 1;
          }
          else if(raxis == AXIS_Y)
          {
            pt->id.event.motion.yrel = 1;
          }
          pt->id.range = range;
        break;
      }
      ++macros_nb;
      return pt;
    }
    else
    {
      fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
    }
  }
  return NULL;
}

/*
 * Try to parse the line as an event definition.
 * If the line matches an event definition:
 *  - events are added to the current macro table (pcurrent)
 * Else:
 *  - nothing is done
 */
static void get_event(const char* line)
{
  char argument[3][LINE_MAX];
  int rbutton;
  int raxis;
  int rvalue;
  int delay_nb;
  int i;
  
  if(!pcurrent)
  {
    return;
  }
  
  int ret = sscanf(line, "%s %s %s", argument[0], argument[1], argument[2]);
  
  if(ret < 2) {
    /* invalid line */
    return;
  }
  
  if (!strncmp(argument[0], "KEYDOWN", strlen("KEYDOWN")))
  {
    rbutton = GE_KeyId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYDOWN;
      pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
    }
  }
  else if (!strncmp(argument[0], "KEYUP", strlen("KEYUP")))
  {
    rbutton = GE_KeyId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYUP;
      pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
    }
  }
  else if (!strncmp(argument[0], "KEY", strlen("KEY")))
  {
    rbutton = GE_KeyId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYDOWN;
      pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
    }

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_event(pcurrent);
    }

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_KEYUP;
      pcurrent->events[pcurrent->nb_events - 1].key.keysym = rbutton;
    }
  }
  else if (!strncmp(argument[0], "MBUTTONDOWN", strlen("MBUTTONDOWN")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONDOWN;
      pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "MBUTTONUP", strlen("MBUTTONUP")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONUP;
      pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "MBUTTON", strlen("MBUTTON")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONDOWN;
      pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
    }

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_event(pcurrent);
    }

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_MOUSEBUTTONUP;
      pcurrent->events[pcurrent->nb_events - 1].button.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
  {
    rbutton = atoi(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONDOWN;
      pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "JBUTTONUP", strlen("JBUTTONUP")))
  {
    rbutton = atoi(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONUP;
      pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "JBUTTON", strlen("JBUTTON"))) {
    rbutton = atoi(argument[1]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONDOWN;
      pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
    }

    delay_nb = ceil((double)DEFAULT_DELAY / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_event(pcurrent);
    }

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYBUTTONUP;
      pcurrent->events[pcurrent->nb_events - 1].jbutton.button = rbutton;
    }
  }
  else if (!strncmp(argument[0], "DELAY", strlen("DELAY")))
  {
    delay_nb = ceil((double)atoi(argument[1]) / (gimx_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_event(pcurrent);
    }
  }
  else if (!strncmp(argument[0], "JAXIS", strlen("JAXIS")))
  {
    if(ret < 3)
    {
      /* invalid line */
      return;
    }

    raxis = atoi(argument[1]);
    rvalue = atoi(argument[2]);

    if(allocate_event(pcurrent) != -1)
    {
      pcurrent->events[pcurrent->nb_events - 1].type = GE_JOYAXISMOTION;
      pcurrent->events[pcurrent->nb_events - 1].jaxis.axis = raxis;
      pcurrent->events[pcurrent->nb_events - 1].jaxis.value = rvalue;
    }
  }
  else if (!strncmp(argument[0], "MAXIS", strlen("MAXIS")))
  {
    if(ret < 3)
    {
      /* invalid line */
      return;
    }

    raxis = atoi(argument[1]);
    rvalue = atoi(argument[2]);

    if(allocate_event(pcurrent) != -1)
    {
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
  }
}

void get_trigger(const char* line)
{
  int etype = -1;
  int rbutton;
  char argument[3][LINE_MAX];
  
  if(!pcurrent)
  {
    return;
  }
  
  int ret = sscanf(line, "%s %s %s", argument[0], argument[1], argument[2]);
  
  if(ret < 2) {
    /* invalid line */
    return;
  }
  
  if (!strncmp(argument[0], "TRIGGER", strlen("TRIGGER")))
  {
    if(ret > 2)
    {
      if(!strncmp(argument[1], "KEYDOWN", strlen("KEYDOWN")))
      {
        if((rbutton = GE_KeyId(argument[2])))
        {
          etype = GE_KEYDOWN;
        }
      }
      else if(!strncmp(argument[1], "KEYUP", strlen("KEYUP")))
      {
        if((rbutton = GE_KeyId(argument[2])))
        {
          etype = GE_KEYUP;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
      {
        if((rbutton = GE_MouseButtonId(argument[2])) >= 0)
        {
          etype = GE_MOUSEBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONUP", strlen("MBUTTONUP")))
      {
        if((rbutton = GE_MouseButtonId(argument[2])) >= 0)
        {
          etype = GE_MOUSEBUTTONUP;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = GE_JOYBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONUP", strlen("JBUTTONUP")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
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
      if((rbutton = GE_KeyId(argument[1])))
      {
        etype = GE_KEYDOWN;
      }
    }
    
    if(etype < 0)
    {
      return;
    }
    
    pcurrent->trigger.type = etype;
    switch(etype)
    {
      case GE_KEYDOWN:
      case GE_KEYUP:
        pcurrent->trigger.key.keysym = rbutton;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
        pcurrent->trigger.button.button = rbutton;
      break;
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
        pcurrent->trigger.jbutton.button = rbutton;
      break;
    }

    //macros with a trigger are default off
    pcurrent->active = ACTIVE_OFF;
  }
}

void get_toggle(const char* line)
{
  char argument[2][LINE_MAX];

  if(!pcurrent)
  {
    return;
  }

  int ret = sscanf(line, "%s %s", argument[0], argument[1]);

  if(ret < 2) {
    /* invalid line */
    return;
  }

  if (!strncmp(argument[0], "TOGGLE", strlen("TOGGLE")))
  {
    if(!strncmp(argument[1], "YES", strlen("YES")))
    {
      pcurrent->toggle = TOGGLE_YES;
    }
  }
}

void get_init(const char* line)
{
  char argument[2][LINE_MAX];

  if(!pcurrent)
  {
    return;
  }

  int ret = sscanf(line, "%s %s", argument[0], argument[1]);

  if(ret < 2) {
    /* invalid line */
    return;
  }

  if (!strncmp(argument[0], "INIT", strlen("INIT")))
  {
    if(!strncmp(argument[1], "ON", strlen("ON")))
    {
      pcurrent->active = ACTIVE_ON;
    }
    else if(!strncmp(argument[1], "OFF", strlen("OFF")))
    {
      pcurrent->active = ACTIVE_OFF;
    }
  }
}

/*
 * Processes a line of a script file (macro definition or command).
 */
void process_line(const char* line)
{
    s_macro * pt = get_macro(line);

    if(pt)
    {
      pcurrent = pt;
    }
    else if(pcurrent)
    {
      get_trigger(line);
      get_toggle(line);
      get_init(line);
      get_event(line);
    }

    return;
}

void dump_event(GE_Event* event, int newline, int axisvalue)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      gprintf("KEYDOWN %s", GE_KeyName(event->key.keysym));
      break;
    case GE_KEYUP:
      gprintf("KEYUP %s", GE_KeyName(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      gprintf("MBUTTONDOWN %s", GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      gprintf("MBUTTONUP %s", GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        gprintf("MAXIS 0");
        if(axisvalue)
        {
          gprintf(" %d", event->motion.xrel);
        }
      }
      else if(event->motion.yrel)
      {
        gprintf("MAXIS 1");
        if(axisvalue)
        {
          gprintf(" %d", event->motion.yrel);
        }
      }
      break;
    case GE_JOYBUTTONDOWN:
      gprintf("JBUTTONDOWN %d", event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      gprintf("JBUTTONUP %d", event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      gprintf("JAXIS %d", event->jaxis.axis);
      if(axisvalue)
      {
        gprintf(" %d", event->motion.yrel);
      }
      break;
    default:
      newline = 0;
      break;
  }
  if(newline)
  {
    gprintf("\n");
  }
}

/*
 * Displays macro_table.
 */
void dump_scripts() {
  s_macro * macro;
  GE_Event * event;
  int delay_nb;

  for (macro = macros; macro < macros + macros_nb; ++macro) {
    gprintf("MACRO ");
    dump_event(&macro->id.event, 0, 0);
    if(macro->id.event.type == GE_MOUSEMOTION || macro->id.event.type == GE_JOYAXISMOTION)
    {
      gprintf(" [%hd,%hd]", macro->id.range.min, macro->id.range.max);
    }
    gprintf("\n");
    if(macro->trigger.type)
    {
      gprintf("TRIGGER ");
      dump_event(&macro->trigger, 1, 0);
    }
    if(macro->active == ACTIVE_ON)
    {
      gprintf("INIT ON\n");
    }
    else
    {
      gprintf("INIT OFF\n");
    }
    if(macro->toggle == TOGGLE_YES)
    {
      gprintf("TOGGLE YES\n");
    }
    else
    {
      gprintf("TOGGLE NO\n");
    }
    delay_nb = 0;
    for (event = macro->events; event && event < macro->events + macro->nb_events; ++event) {
      if (!event->type) {
        delay_nb++;
      }
      else if(delay_nb) {
        gprintf("DELAY %d\n", delay_nb*(gimx_params.refresh_period/1000));
        delay_nb = 0;
      }
      dump_event(event, 1, 0);
    }
    gprintf("\n");
  }
}

static char** macro_configs = NULL;
static unsigned int nb_macro_configs = 0;
static unsigned char configs_txt_present = 0;

static void read_configs_txt(const char* dir_path)
{
  char line[LINE_MAX];
  char config[LINE_MAX];
  char macro[LINE_MAX];
  char extra[LINE_MAX];
  char file_path[PATH_MAX];
  FILE* fp;
  int ret;

  snprintf(file_path, sizeof(file_path), "%s%s", dir_path, MACRO_CONFIGS_FILE);
  fp = fopen(file_path, "r");
  if (fp)
  {
    configs_txt_present = 1;
    while (fgets(line, LINE_MAX, fp)) {
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
            fprintf(stderr, "configs.txt: invalid line: %s", line);
            continue;
          }
        }

        if(!strcmp(config, gimx_params.config_file))
        {
          char ** ptr = realloc(macro_configs, (nb_macro_configs+1)*sizeof(char*));
          if(ptr)
          {
            macro_configs = ptr;
            macro_configs[nb_macro_configs] = strdup(macro);
            if(macro_configs[nb_macro_configs])
            {
              nb_macro_configs++;
            }
            else
            {
              fprintf(stderr, "%s:%d strdup failed\n", __FILE__, __LINE__);
            }
          }
          else
          {
            fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
          }
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
    DIR *dirp;
    FILE* fp;
    char dir_path[PATH_MAX];
    char file_path[PATH_MAX];
    struct dirent *d;
    unsigned int i, j;
    unsigned int nb_filenames = 0;
    char** filenames = NULL;
#ifdef WIN32
    struct stat buf;
#endif

    snprintf(dir_path, sizeof(dir_path), "%s/%s/%s", gimx_params.homedir, GIMX_DIR, MACRO_DIR);

    dirp = opendir(dir_path);
    if (dirp == NULL)
    {
      gprintf("Warning: can't open macro directory %s\n", dir_path);
      return;
    }

    while ((d = readdir(dirp)))
    {
      if(!strcmp(d->d_name, MACRO_CONFIGS_FILE))
      {
        continue;
      }
#ifndef WIN32
      if (d->d_type == DT_REG)
      {
        char ** ptr = realloc(filenames, (nb_filenames+1)*sizeof(char*));
        if(ptr)
        {
          filenames = ptr;
          filenames[nb_filenames] = strdup(d->d_name);
          if(filenames[nb_filenames])
          {
            ++nb_filenames;
          }
          else
          {
            fprintf(stderr, "%s:%d strdup failed\n", __FILE__, __LINE__);
          }
        }
        else
        {
          fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
        }
      }
#else
      snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, d->d_name);
      if(stat(file_path, &buf) == 0)
      {
        if(S_ISREG(buf.st_mode))
        {
          char ** ptr = realloc(filenames, (nb_filenames+1)*sizeof(char*));
          if(ptr)
          {
            filenames = ptr;
            filenames[nb_filenames] = strdup(d->d_name);
            if(filenames[nb_filenames])
            {
              ++nb_filenames;
            }
            else
            {
              fprintf(stderr, "%s:%d strdup failed\n", __FILE__, __LINE__);
            }
          }
          else
          {
            fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
          }
        }
      }
#endif
    }

    closedir(dirp);

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
      fp = fopen(file_path, "r");
      if (!fp) {
        fprintf(stderr, "Can not find '%s'\n", file_path);
      } else {
        while (fgets(line, LINE_MAX, fp)) {
          if (line[0] != '#') {
            process_line(line);
          }
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

static void macros_read()
{
  read_macros();
  if(debug)
  {
    dump_scripts();
  }
}

/*
 * Initializes macro_table and reads macros from macro files.
 */
void macros_init() {
  macros_read();
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
    fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
  }
}

/*
 * Unregister a macro.
 */
static int macro_delete(GE_Event* event)
{
  int i;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(!compare_events(&running_macro[i].event, event))
    {
      if(GE_GetDeviceId(&running_macro[i].event) == GE_GetDeviceId(event))
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
    running_macro[running_macro_nb].event = *event;
    running_macro[running_macro_nb].macro_index = macro;
    running_macro[running_macro_nb].event_index = 0;
    running_macro_nb++;
  }
  else
  {
    fprintf(stderr, "%s:%d realloc failed\n", __FILE__, __LINE__);
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
    if(macros[i].trigger.type != GE_NOEVENT)
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
            macros[i].active = ACTIVE_ON;
            /*
             * Disable macros that have a different activation trigger.
             */
            for(j=0; j<macros_nb; ++j)
            {
              if(macros[j].trigger.type == GE_NOEVENT)
              {
                continue;
              }
              if(compare_events(event, &macros[j].trigger)
                 && macros[j].active == ACTIVE_ON)
              {
                macros[j].active = ACTIVE_OFF;
              }
            }
          }
        }
        else
        {
          if(macros[i].active == ACTIVE_OFF)
          {
            gprintf("enable macro: ");
            dump_event(&macros[i].id.event, 1, 0);
            macros[i].active = ACTIVE_ON;
          }
          else
          {
            gprintf("disable macro: ");
            dump_event(&macros[i].id.event, 1, 0);
            macros[i].active = ACTIVE_OFF;
          }
        }
      }
    }
    if(!compare_macro_ids(event, &macros[i].id))
    {
      if(macros[i].active == ACTIVE_ON)
      {
        /*
         * Start or stop a macro.
         */
        if(!macro_delete(event))
        {
          gprintf("start macro: ");
          dump_event(&macros[i].id.event, 1, 0);
          macro_add(event, i);
        }
        else
        {
          gprintf("stop macro: ");
          dump_event(&macros[i].id.event, 1, 0);
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
  int i, j;
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
      dtype2 = get_event_device_type(&running_macro[i].event);
      did = GE_GetDeviceId(&running_macro[i].event);
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
        GE_PushEvent(&event);
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

