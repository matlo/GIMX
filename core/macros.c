/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <SDL/SDL.h>
#include <limits.h>
#include "macros.h"
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "conversion.h"
#include <math.h>
#include "emuclient.h"
#include "sdl_tools.h"

#ifdef WIN32
#include <sys/stat.h>
#define LINE_MAX 1024
#endif

/* This is the minimum amount of time between KEYDOWN and KEYUP. */
#define DEFAULT_DELAY 50

/* This is the name of the file that contains all macro files. */
#ifndef WIN32
#define MACRO_DIR ".emuclient/macros/"
#else
#define MACRO_DIR "macros/"
#endif

typedef struct
{
  SDL_Event event;
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
    SDL_Event event;
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
    free(macro_table);
}

/*
 * Allocates an element and initializes it to 0.
 */
void allocate_element(s_macro_event** pt) {
    (*pt) = realloc((*pt), sizeof(s_macro_event) * (++((*pt)->size)));
    memset((*pt) + (*pt)->size - 1, 0x00, sizeof(s_macro_event));
}

int compare_events(SDL_Event* e1, SDL_Event* e2)
{
  int ret = 1;
  if(e1->type == e2->type)
  {
    switch(e1->type)
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if(e1->key.keysym.sym == e2->key.keysym.sym)
        {
          ret = 0;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if(e1->button.button == e2->button.button)
        {
          ret = 0;
        }
        break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        if(e1->jbutton.button == e2->jbutton.button)
        {
          ret = 0;
        }
        break;
      case SDL_NOEVENT:
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
        if((rbutton = get_key_from_buffer(argument[2])))
        {
          etype = SDL_KEYDOWN;
        }
      }
      else if(!strncmp(argument[1], "KEYUP", strlen("KEYUP")))
      {
        if((rbutton = get_key_from_buffer(argument[2])))
        {
          etype = SDL_KEYUP;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
      {
        if((rbutton = get_mouse_event_id_from_buffer(argument[2])))
        {
          etype = SDL_MOUSEBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONUP", strlen("MBUTTONUP")))
      {
        if((rbutton = get_mouse_event_id_from_buffer(argument[2])))
        {
          etype = SDL_MOUSEBUTTONUP;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = SDL_JOYBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONUP", strlen("JBUTTONUP")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = SDL_JOYBUTTONUP;
        }
      }
    }
    else
    {
      /*
       * For compatibility with previous macro files.
       */
      if((rbutton = get_key_from_buffer(argument[1])))
      {
        etype = SDL_KEYDOWN;
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
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        (*pt)[1].event.key.keysym.sym = rbutton;
      break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        (*pt)[1].event.button.button = rbutton;
      break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
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
  char argument[2][LINE_MAX];
  int rbutton;
  int delay_nb;
  int i;
  
  if(!*pcurrent)
  {
    return;
  }
  
  int ret = sscanf(line, "%s %s", argument[0], argument[1]);
  
  if(ret < 2) {
    /* invalid line */
    return;
  }
  
  if (!strncmp(argument[0], "KEYDOWN", strlen("KEYDOWN")))
  {
    rbutton = get_key_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_KEYDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym.sym = rbutton;
  }
  else if (!strncmp(argument[0], "KEYUP", strlen("KEYUP")))
  {
    rbutton = get_key_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_KEYUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym.sym = rbutton;
  }
  else if (!strncmp(argument[0], "KEY", strlen("KEY")))
  {
    rbutton = get_key_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_KEYDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym.sym = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_KEYUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.key.keysym.sym = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTONDOWN", strlen("MBUTTONDOWN")))
  {
    rbutton = get_mouse_event_id_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_MOUSEBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTONUP", strlen("MBUTTONUP")))
  {
    rbutton = get_mouse_event_id_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_MOUSEBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "MBUTTON", strlen("MBUTTON")))
  {
    rbutton = get_mouse_event_id_from_buffer(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_MOUSEBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_MOUSEBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.button.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
  {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_JOYBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTONUP", strlen("JBUTTONUP")))
  {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_JOYBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "JBUTTON", strlen("JBUTTON"))) {
    rbutton = atoi(argument[1]);

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_JOYBUTTONDOWN;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;

    delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }

    allocate_element(pcurrent);

    (*pcurrent)[(*pcurrent)->size - 1].event.type = SDL_JOYBUTTONUP;
    (*pcurrent)[(*pcurrent)->size - 1].event.jbutton.button = rbutton;
  }
  else if (!strncmp(argument[0], "DELAY", strlen("DELAY")))
  {
    delay_nb = ceil((double)atoi(argument[1]) / (refresh_rate/1000));
    for(i=0; i<delay_nb; ++i)
    {
      allocate_element(pcurrent);
    }
  }
}

void get_trigger(const char* line)
{
  int etype = -1;
  int rbutton;
  char argument[3][LINE_MAX];
  
  if(!*pcurrent)
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
        if((rbutton = get_key_from_buffer(argument[2])))
        {
          etype = SDL_KEYDOWN;
        }
      }
      else if(!strncmp(argument[1], "KEYUP", strlen("KEYUP")))
      {
        if((rbutton = get_key_from_buffer(argument[2])))
        {
          etype = SDL_KEYUP;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONDOWN", strlen ("MBUTTONDOWN")))
      {
        if((rbutton = get_mouse_event_id_from_buffer(argument[2])))
        {
          etype = SDL_MOUSEBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "MBUTTONUP", strlen("MBUTTONUP")))
      {
        if((rbutton = get_mouse_event_id_from_buffer(argument[2])))
        {
          etype = SDL_MOUSEBUTTONUP;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONDOWN", strlen("JBUTTONDOWN")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = SDL_JOYBUTTONDOWN;
        }
      }
      else if(!strncmp(argument[1], "JBUTTONUP", strlen("JBUTTONUP")))
      {
        if((rbutton = atoi(argument[2])) >= 0)
        {
          etype = SDL_JOYBUTTONUP;
        }
      }
    }
    else
    {
      /*
       * For compatibility with previous macro files.
       */
      if((rbutton = get_key_from_buffer(argument[1])))
      {
        etype = SDL_KEYDOWN;
      }
    }
    
    if(etype < 0)
    {
      return;
    }
    
    (*pcurrent)[0].event.type = etype;
    switch(etype)
    {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        (*pcurrent)[0].event.key.keysym.sym = rbutton;
      break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        (*pcurrent)[0].event.button.button = rbutton;
      break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
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
    else
    {
      get_trigger(line);
      get_event(line);
    }

    return;
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
            for (p_element = *p_table; p_element && p_element < *p_table + (*p_table)->size; p_element++) {
                if(p_element == *p_table && p_element->event.type)
                {
                  printf("%d TRIGGER ", p_element->active);
                }
                else if(p_element == *p_table+1)
                {
                  printf("MACRO ");
                }
                if (!p_element->event.type) {
                    delay_nb++;
                }
                else if(delay_nb)
                {
                    printf("DELAY %d\n", delay_nb*(refresh_rate/1000));
                    delay_nb = 0;
                }
                switch(p_element->event.type)
                {
                  case SDL_KEYDOWN:
                    printf("KEYDOWN %s\n", get_chars_from_key(p_element->event.key.keysym.sym));
                    break;
                  case SDL_KEYUP:
                    printf("KEYUP %s\n", get_chars_from_key(p_element->event.key.keysym.sym));
                    break;
                  case SDL_MOUSEBUTTONDOWN:
                    printf("MBUTTONDOWN %s\n", get_chars_from_button(p_element->event.button.button));
                    break;
                  case SDL_MOUSEBUTTONUP:
                    printf("MBUTTONUP %s\n", get_chars_from_button(p_element->event.button.button));
                    break;
                  case SDL_JOYBUTTONDOWN:
                    printf("JBUTTONDOWN %d\n", p_element->event.jbutton.button);
                    break;
                  case SDL_JOYBUTTONUP:
                    printf("JBUTTONUP %d\n", p_element->event.jbutton.button);
                    break;
                }
            }
            printf("\n");
        }
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
    unsigned int i;
    unsigned int nb_filenames = 0;
    char** filenames = NULL;
    char** filenames_realloc;
#ifdef WIN32
    struct stat buf;
#endif

#ifndef WIN32
    snprintf(dir_path, sizeof(dir_path), "%s/%s", homedir, MACRO_DIR);
#else
    snprintf(dir_path, sizeof(dir_path), "%s", MACRO_DIR);
#endif
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

    for(i=0; i<nb_filenames; ++i)
    {
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
  SDL_Event e = {.type = SDL_NOEVENT};
  for(i=0; i<macro_table_nb; ++i)
  {
    if(macro_table[i][0].event.type == SDL_NOEVENT)
    {
      /*
       * No trigger => macro is always active.
       */
      macro_table[i][0].active = 1;
    }
    else if(compare_events(&e, &macro_table[i][0].event))
    {
      if(e.type == SDL_NOEVENT)
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
  dump_scripts();
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
static int macro_delete(SDL_Event* event)
{
  int i;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(!compare_events(&running_macro[i].event, event))
    {
      if(sdl_get_device_id(&running_macro[i].event) == sdl_get_device_id(event))
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
static void macro_add(SDL_Event* event, int macro)
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
void macro_lookup(SDL_Event* event)
{
  int i, j;
  for(i=0; i<macro_table_nb; ++i)
  {
    if(macro_table[i][0].event.type != SDL_NOEVENT)
    {
      if(!compare_events(event, &macro_table[i][0].event) 
         && !macro_table[i][0].active)
      {
        macro_table[i][0].active = 1;
        for(j=0; j<macro_table_nb; ++j)
        {
          if(macro_table[j][0].event.type == SDL_NOEVENT)
          {
            continue;
          }
          if(compare_events(event, &macro_table[j][0].event)
             && macro_table[j][0].active)
          {
            macro_table[j][0].active = 0;
          }
        }
        break;
      }
    }
    if(!compare_events(event, &macro_table[i][1].event))
    {
      if(macro_table[i][0].active)
      {
        if(!macro_delete(event))
        {
          macro_add(event, i);
        }
      }
    }
  }
}

int get_event_device_type(SDL_Event* ev)
{
  switch(ev->type)
  {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      return E_DEVICE_TYPE_KEYBOARD;
    break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      return E_DEVICE_TYPE_MOUSE;
    break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      return E_DEVICE_TYPE_JOYSTICK;
    break;
  }
  return E_DEVICE_TYPE_UNKNOWN;
}

/*
 * Generate events for pending macros.
 */
void macro_process()
{
  int i;
  int dtype1, dtype2, did;
  SDL_Event event;
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
        did = sdl_get_device_id(&running_macro[i].event);
        if(dtype1 != E_DEVICE_TYPE_UNKNOWN && dtype2 != E_DEVICE_TYPE_UNKNOWN && did >= 0)
        {
          int controller = device_controller[dtype2-1][did];
          if(controller < 0)
          {
            controller = 0;
          }
          did = controller_device[dtype1-1][controller];
          if(did < 0)
          {
            did = 0;
          }
          event.key.which = did;
          SDL_PushEvent(&event);
        }
        if(event.type != SDL_NOEVENT)
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
      gprintf(_("keyboard %s (%d)\n"), sdl_get_keyboard_name(device_id), sdl_get_keyboard_virtual_id(device_id));
    }
    else if(device_type == 1)
    {
      gprintf(_("mouse %s (%d)\n"), sdl_get_mouse_name(device_id), sdl_get_mouse_virtual_id(device_id));    
    }
    else if(device_type == 2)
    {
      gprintf(_("joystick %s (%d)\n"), sdl_get_joystick_name(device_id), sdl_get_joystick_virtual_id(device_id));    
    }
  }
}
