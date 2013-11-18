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
#include "emuclient.h"
#include "config.h"
#include <GE.h>
#include "../directories.h"

#ifdef WIN32
#include <sys/stat.h>
#define LINE_MAX 1024
#endif

/* This is the minimum amount of time between KEYDOWN and KEYUP. */
#define DEFAULT_DELAY 50

typedef struct
{
  GE_Event event;
  int macro;
  int index;
} s_running_macro;

/*
 * This table contains pending macros.
 * Dynamically allocated.
 */
static s_running_macro* running_macro = { NULL };
static unsigned int running_macro_nb;

/*
 * These tables are used to retrieve a device id for an event with a different device type than the trigger.
 */
static int controller_device[3][MAX_CONTROLLERS];
static int device_controller[3][MAX_DEVICES];

typedef struct {
  GE_Event event;
    /*
     * Below elements are only significant for the first entry.
     */
    unsigned char active; //1 = active, 0 = inactive
    int size; //The size of the table.
    int macro; //The index of the macro in running_macro.
} s_macro_event;

/*
 * This table is used to store all the macros that are read from script files at the initialization of the process.
 */
static s_macro_event** macro_table = NULL;
static int macro_table_nb = 0;
unsigned char triggered = 0;

/*
 * Cleans macro_table.
 * Frees all allocated blocks pointed by macro_table.
 * Frees running_macro table.
 */
void free_macros() {    
  free(running_macro);
	int i;
	for(i = 0; i < macro_table_nb; ++i)
	{
    free(macro_table[i]);
	}
  free(macro_table);
}

/*
 * Allocates an element and initializes it to 0.
 */
void allocate_element(s_macro_event** pt) {
    (*pt) = realloc((*pt), sizeof(s_macro_event) * (++((*pt)->size)));
    memset((*pt) + (*pt)->size - 1, 0x00, sizeof(s_macro_event));
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
      case GE_NOEVENT:
        ret = 0;
        break;
      default:
        break;
    }
  }
  return ret;
}

static s_macro_event** pcurrent = NULL;

/*
 * Try to parse the line as a macro definition.
 * If the line matches a macro definition:
 *  - a new macro table is allocated
 *  - a pointer to this table is returned
 * Else:
 *  - NULL is returned
 */
static s_macro_event** get_macro(const char* line)
{
  int etype = -1;
  int rbutton;
  char argument[3][LINE_MAX];
  s_macro_event** pt;
  int ret = sscanf(line, "%s %s %s", argument[0], argument[1], argument[2]);
  
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
    
    macro_table_nb++;
    macro_table = realloc(macro_table, macro_table_nb*sizeof(s_macro_event*));
    macro_table[macro_table_nb-1] = NULL;
    pt = &(macro_table[macro_table_nb-1]);

    (*pt) = calloc(2, sizeof(s_macro_event));
    (*pt)[0].size = 2;
    
    (*pt)[1].event.type = etype;
    switch(etype)
    {
      case GE_KEYDOWN:
      case GE_KEYUP:
        (*pt)[1].event.key.keysym = rbutton;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
        (*pt)[1].event.button.button = rbutton;
      break;
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
        (*pt)[1].event.jbutton.button = rbutton;
      break;
    }
    return pt;
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
  
  if(!pcurrent || !*pcurrent)
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

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_KEYDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym = rbutton;
  }
  else if (!strncmp(argument[0], "KEYUP", strlen("KEYUP")))
  {
    rbutton = GE_KeyId(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_KEYUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym = rbutton;
  }
  else if (!strncmp(argument[0], "KEY", strlen("KEY")))
  {
    rbutton = GE_KeyId(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_KEYDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (emuclient_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_KEYUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTONDOWN", strlen("MBUTTONDOWN")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_MOUSEBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTONUP", strlen("MBUTTONUP")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_MOUSEBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTON", strlen("MBUTTON")))
  {
    rbutton = GE_MouseButtonId(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_MOUSEBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (emuclient_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_MOUSEBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
  {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_JOYBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTONUP", strlen("JBUTTONUP")))
  {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_JOYBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTON", strlen("JBUTTON"))) {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_JOYBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (emuclient_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_JOYBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "DELAY", strlen("DELAY")))
  {
    delay_nb = ceil((double)atoi(argument[1]) / (emuclient_params.refresh_period/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
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

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_JOYAXISMOTION;
    (*pcurrent)[(*pcurrent)->size - 1].event.jaxis.axis = raxis;
    (*pcurrent)[(*pcurrent)->size - 1].event.jaxis.value = rvalue;
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

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = GE_MOUSEMOTION;
    if(raxis == AXIS_X)
    {
      (*pcurrent)[(*pcurrent)->size - 1].event.motion.xrel = rvalue;
    }
    else if(raxis == AXIS_Y)
    {
      (*pcurrent)[(*pcurrent)->size - 1].event.motion.yrel = rvalue;
    }
  }
}

void get_trigger(const char* line)
{
  int etype = -1;
  int rbutton;
  char argument[3][LINE_MAX];
  
  if(pcurrent && !*pcurrent)
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
    
    (*pcurrent)[0].event.type = etype;
    switch(etype)
    {
      case GE_KEYDOWN:
      case GE_KEYUP:
        (*pcurrent)[0].event.key.keysym = rbutton;
      break;
      case GE_MOUSEBUTTONDOWN:
      case GE_MOUSEBUTTONUP:
        (*pcurrent)[0].event.button.button = rbutton;
      break;
      case GE_JOYBUTTONDOWN:
      case GE_JOYBUTTONUP:
        (*pcurrent)[0].event.jbutton.button = rbutton;
      break;
    }
  }
}

/*
 * Processes a line of a script file (macro definition or command).
 */
void process_line(const char* line)
{
    s_macro_event** pt = get_macro(line);

    if(pt)
    {
      pcurrent = pt;
    }
    else if(pcurrent)
    {
      get_trigger(line);
      get_event(line);
    }

    return;
}

void dump_event(GE_Event* event)
{
  switch(event->type)
  {
    case GE_KEYDOWN:
      printf("KEYDOWN %s\n", GE_KeyName(event->key.keysym));
      break;
    case GE_KEYUP:
      printf("KEYUP %s\n", GE_KeyName(event->key.keysym));
      break;
    case GE_MOUSEBUTTONDOWN:
      printf("MBUTTONDOWN %s\n", GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEBUTTONUP:
      printf("MBUTTONUP %s\n", GE_MouseButtonName(event->button.button));
      break;
    case GE_MOUSEMOTION:
      if(event->motion.xrel)
      {
        printf("MAXIS 0 %d\n", event->motion.xrel);
      }
      else if(event->motion.yrel)
      {
        printf("MAXIS 1 %d\n", event->motion.yrel);
      }
      break;
    case GE_JOYBUTTONDOWN:
      printf("JBUTTONDOWN %d\n", event->jbutton.button);
      break;
    case GE_JOYBUTTONUP:
      printf("JBUTTONUP %d\n", event->jbutton.button);
      break;
    case GE_JOYAXISMOTION:
      printf("JAXIS %d %d\n", event->jaxis.axis, event->jaxis.value);
      break;
  }
}

/*
 * Displays macro_table.
 */
void dump_scripts() {
    s_macro_event** p_table;
    s_macro_event* p_element;
    int delay_nb;

    for (p_table = macro_table; p_table < macro_table + macro_table_nb; p_table++) {
        if (*p_table) {
            delay_nb = 0;
            for (p_element = *p_table+1; p_element && p_element < *p_table + (*p_table)->size; p_element++) {
                if(p_element == *p_table+1)
                {
                  printf("MACRO ");
                  dump_event(&p_element->event);
                  if((*p_table)->event.type)
                  {
                    printf("TRIGGER ");
                    dump_event(&(*p_table)->event);
                  }
                }
                else
                {
                  if (!p_element->event.type) {
                      delay_nb++;
                  }
                  else if(delay_nb)
                  {
                      printf("DELAY %d\n", delay_nb*(emuclient_params.refresh_period/1000));
                      delay_nb = 0;
                  }
                  dump_event(&p_element->event);
                }
            }
            printf("\n");
        }
    }
}

static char** macros = NULL;
static unsigned int nb_macros = 0;
static unsigned char configs_txt_present = 0;

static void read_configs_txt(const char* dir_path)
{
  char** macros_realloc = NULL;
  char line[LINE_MAX];
  char config[PATH_MAX];
  char macro[PATH_MAX];
  char file_path[PATH_MAX];
  FILE* fp;
  int ret;

  snprintf(file_path, sizeof(file_path), "%s%s", dir_path, "configs.txt");
  fp = fopen(file_path, "r");
  if (fp)
  {
    configs_txt_present = 1;
    while (fgets(line, LINE_MAX, fp)) {
      if (line[0] != '#') {
        ret = sscanf(line, "%s %s", config, macro);

        if(ret < 2) {
          /* invalid line */
          return;
        }

        if(!strcmp(config, emuclient_params.config_file))
        {
          macros_realloc = realloc(macros, (nb_macros+1)*sizeof(char*));
          if(macros_realloc)
          {
            macros = macros_realloc;
            macros[nb_macros] = strdup(macro);
            nb_macros++;
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
    char** filenames_realloc;
#ifdef WIN32
    struct stat buf;
#endif

    snprintf(dir_path, sizeof(dir_path), "%s/%s/%s", emuclient_params.homedir, APP_DIR, MACRO_DIR);

    dirp = opendir(dir_path);
    if (dirp == NULL)
    {
      gprintf("Warning: can't open macro directory %s\n", dir_path);
      return;
    }

    while ((d = readdir(dirp)))
    {
#ifndef WIN32
      if (d->d_type == DT_REG)
      {
        filenames_realloc = realloc(filenames, (nb_filenames+1)*sizeof(char*));
        if(filenames_realloc)
        {
          filenames = filenames_realloc;
          nb_filenames++;
          filenames[nb_filenames-1] = strdup(d->d_name);
        }
      }
#else
      snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, d->d_name);
      if(stat(file_path, &buf) == 0)
      {
        if(S_ISREG(buf.st_mode))
        {
          filenames_realloc = realloc(filenames, (nb_filenames+1)*sizeof(char*));
          if(filenames_realloc)
          {
            filenames = filenames_realloc;
            nb_filenames++;
            filenames[nb_filenames-1] = strdup(d->d_name);
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
        for(j=0; j<nb_macros; ++j)
        {
          if(!strcmp(macros[j], filenames[i]))
          {
            break;
          }
        }
        if(j == nb_macros)
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

    for(i=0; i<nb_macros; ++i)
    {
      free(macros[i]);
    }
    free(macros);
}

/*
 * Initializes macro_table and reads macros from macro files.
 */
void macros_init() {
  int i, j;
  for(j=0; j<3; ++j)
  {
    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
      controller_device[j][i] = -1;
    }
    for(i=0; i<MAX_DEVICES; ++i)
    {
      device_controller[j][i] = -1;
    }
  }
}

static void active_triggered_init()
{
  int i;
  GE_Event e = {.type = GE_NOEVENT};
  for(i=0; i<macro_table_nb; ++i)
  {
    if(macro_table[i][0].event.type == GE_NOEVENT)
    {
      /*
       * No trigger => macro is always active.
       */
      macro_table[i][0].active = 1;
    }
    else if(compare_events(&e, &macro_table[i][0].event))
    {
      if(e.type == GE_NOEVENT)
      {
        /*
         * First triggered macro => active at startup.
         */
        macro_table[i][0].active = 1;
        e = macro_table[i][0].event;
      }
    }
    else
    {
      /*
       * Trigger is the same as first triggered macro => active at startup.
       */
      macro_table[i][0].active = 1;
    }
  }
}

void macros_read()
{
  read_macros();
  active_triggered_init();
  //dump_scripts();
}

static void macro_unalloc(int index)
{
  s_running_macro* running_macro_realloc;
  memcpy(running_macro+index, running_macro+index+1, (running_macro_nb-index-1)*sizeof(s_running_macro));
  running_macro_realloc = realloc(running_macro, (running_macro_nb-1)*sizeof(s_running_macro));
  if(running_macro_realloc || !(running_macro_nb-1))
  {
    running_macro = running_macro_realloc;
    running_macro_nb--;
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
  s_running_macro* running_macro_realloc = realloc(running_macro, (running_macro_nb+1)*sizeof(s_running_macro));
  if(running_macro_realloc)
  {
    running_macro = running_macro_realloc;
    running_macro[running_macro_nb].event = *event;
    running_macro[running_macro_nb].macro = macro;
    running_macro[running_macro_nb].index = 1;
    running_macro_nb++;
  }
}

/*
 * Start or stop a macro.
 */
void macro_lookup(GE_Event* event)
{
  int i, j;
  for(i=0; i<macro_table_nb; ++i)
  {
    if(macro_table[i][0].event.type != GE_NOEVENT)
    {
      /*
       * Check if macro has to be activated.
       */
      if(!compare_events(event, &macro_table[i][0].event) 
         && !macro_table[i][0].active)
      {
        macro_table[i][0].active = 1;
        /*
         * Disable macros that have a different activation trigger.
         */
        for(j=0; j<macro_table_nb; ++j)
        {
          if(macro_table[j][0].event.type == GE_NOEVENT)
          {
            continue;
          }
          if(compare_events(event, &macro_table[j][0].event)
             && macro_table[j][0].active)
          {
            macro_table[j][0].active = 0;
          }
        }
      }
    }
    if(!compare_events(event, &macro_table[i][1].event))
    {
      if(macro_table[i][0].active)
      {
        /*
         * Start or stop a macro.
         */
        if(!macro_delete(event))
        {
          macro_add(event, i);
        }
      }
    }
  }
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
  s_macro_event* p_macro_table;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(running_macro[i].index < macro_table[running_macro[i].macro]->size)
    {
      running_macro[i].index++;
    
      while(running_macro[i].index < macro_table[running_macro[i].macro]->size)
      {
        p_macro_table = macro_table[running_macro[i].macro] + running_macro[i].index;
        event = p_macro_table->event;
        dtype1 = get_event_device_type(&event);
        dtype2 = get_event_device_type(&running_macro[i].event);
        did = GE_GetDeviceId(&running_macro[i].event);
        if(dtype1 != E_DEVICE_TYPE_UNKNOWN && dtype2 != E_DEVICE_TYPE_UNKNOWN && did >= 0)
        {
          int controller = device_controller[dtype2-1][did];
          if(controller < 0)
          {
            controller = 0;
            for(j=0; j<MAX_CONTROLLERS; ++j)
            {
              if(controller_device[dtype1-1][j] >= 0)
              {
                controller = j;
                break;
              }
            }
          }
          did = controller_device[dtype1-1][controller];
          if(did < 0)
          {
            did = 0;
          }
          event.key.which = did;
          GE_PushEvent(&event);
        }
        if(event.type != GE_NOEVENT)
        {
          running_macro[i].index++;
        }
        else
        {
          break;
        }
      }
    }
    if(running_macro[i].index >= macro_table[running_macro[i].macro]->size)
    {
      macro_unalloc(i);
      i--;
    }
  }
  return running_macro_nb;
}

void macro_set_controller_device(int controller, int device_type, int device_id)
{
  if(controller < 0 || controller >= MAX_CONTROLLERS
  || device_type < 0 || device_type >= 3
  || device_id < 0 || device_type > MAX_DEVICES)
  {
    fprintf(stderr, "set_controller_device error\n");
    return;
  }
  if(controller_device[device_type][controller] < 0)
  {
    controller_device[device_type][controller] = device_id;
    device_controller[device_type][device_id] = controller;
  }
  else if(controller_device[device_type][controller] != device_id)
  {
    gprintf(_("macros are not not available for: "));
    if(device_type == 0)
    {
      gprintf(_("keyboard %s (%d)\n"), GE_KeyboardName(device_id), GE_KeyboardVirtualId(device_id));
    }
    else if(device_type == 1)
    {
      gprintf(_("mouse %s (%d)\n"), GE_MouseName(device_id), GE_MouseVirtualId(device_id));
    }
    else if(device_type == 2)
    {
      gprintf(_("joystick %s (%d)\n"), GE_JoystickName(device_id), GE_JoystickVirtualId(device_id));
    }
  }
}
