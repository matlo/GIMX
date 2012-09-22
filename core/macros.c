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

typedef struct {
    SDL_Event event;
    int delay;
    int size;
    char macro[MAX_NAME_LENGTH];
} s_macro_event_delay;

/*
 * These tables are used to retrieve a device id for an event with a different device type than the trigger.
 */
int controller_device[3][MAX_CONTROLLERS];
int device_controller[3][MAX_DEVICES];

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

/*
 * This table is used to store all the macros that are read from script files at the initialization of the process.
 * index 1: the device type, keyboard=0, mouse=1, joystick=2
 * index 2: the key or button id
 * index 3: key down=1, key up=0
 */
s_macro_event_delay* macro_table[3][SDLK_LAST][2];

/*
 * Initializes macro_table.
 */
void initialize_macro_table() {
    memset(macro_table, 0x00, sizeof(macro_table));
}

/*
 * Cleans macro_table.
 * Frees all allocated blocks pointed by macro_table.
 */
void free_macros() {
    s_macro_event_delay** pt;

    for (pt = &(macro_table[0][0][0]); pt < &(macro_table[0][0][0])+(sizeof(macro_table)/sizeof(s_macro_event_delay*)); pt++) {
        free(*pt);
    }
}

/*
 * Allocates an element and initializes it to 0.
 */
void allocate_element(s_macro_event_delay** pt) {
    (*pt) = realloc((*pt), sizeof(s_macro_event_delay) * (++((*pt)->size)));
    memset((*pt) + (*pt)->size - 1, 0x00, sizeof(s_macro_event_delay));
}

/*
 * Processes a line of a script file (macro definition or command).
 * If line is a valid macro definition, *dtype, *button and *down are set.
 * If line is a not a valid macro definition (already defined or wrong arguments), *dtype is set to -1.
 * If *dtype >= 0 and  if line is a valid command, macro_table[*dtype][*button][*down] is modified.
 * If *dtype == -1 and if line is not a macro definition, nothing is done.
 */
void process_line(const char* line, int* dtype, int* button, int* down) {
    char command[LINE_MAX];
    char argument[LINE_MAX];
    int rbutton;
    int delay_nb;
    s_macro_event_delay** pt = NULL;
    int ret;
    int i;

    if (*dtype >= 0) {
        pt = &macro_table[*dtype][*button][*down];
    }

    ret = sscanf(line, "%s %s", command, argument);

    if(ret < 2) {
        /* invalid line */
        return;
    }

    if (!strncmp(command, "MACRO", strlen("MACRO"))) {
        if(strstr(line, "KEYDOWN"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = get_key_from_buffer(argument)))
            {
              *dtype = 0;
              *button = rbutton;
              *down = 1;
            }
          }
        }
        else if(strstr(line, "KEYUP"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = get_key_from_buffer(argument)))
            {
              *dtype = 0;
              *button = rbutton;
              *down = 0;
            }
          }
        }
        else if(strstr(line, "MBUTTONDOWN"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = get_mouse_event_id_from_buffer(argument)))
            {
              *dtype = 1;
              *button = rbutton;
              *down = 1;
            }
          }
        }
        else if(strstr(line, "MBUTTONUP"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = get_mouse_event_id_from_buffer(argument)))
            {
              *dtype = 1;
              *button = rbutton;
              *down = 0;
            }
          }
        }
        else if(strstr(line, "JBUTTONDOWN"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = atoi(argument)) >= 0)
            {
              *dtype = 2;
              *button = rbutton;
              *down = 1;
            }
          }
        }
        else if(strstr(line, "JBUTTONUP"))
        {
          ret = sscanf(line, "%*s %*s %s", argument);
          if(ret == 1)
          {
            if((rbutton = atoi(argument)) >= 0)
            {
              *dtype = 2;
              *button = rbutton;
              *down = 0;
            }
          }
        }
        else
        {
          /*
           * For compatibility with previous macro files.
           */
          if((rbutton = get_key_from_buffer(argument)))
          {
            *dtype = 0;
            *button = rbutton;
            *down = 1;
          }
        }
        
        if(*dtype < 0)
        {
          return;
        }

        pt = &(macro_table[*dtype][*button][*down]);

        if (!*pt) {
            (*pt) = calloc(1, sizeof(s_macro_event_delay));
            (*pt)->size = 1;
            strncpy((*pt)->macro, argument, MAX_NAME_LENGTH);
        } else {
            gprintf(_("Macro %s defined twice!\n"), argument);
            *dtype = -1;
        }
        return;
    }
    
    if (!strncmp(command, "KEYDOWN", strlen("KEYDOWN"))) {
        if (*pt) {
            rbutton = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "KEYUP", strlen("KEYUP"))) {
        if (*pt) {
            rbutton = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "KEY", strlen("KEY"))) {
        if (*pt) {
            rbutton = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
      
            delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
            for(i=0; i<delay_nb; ++i)
            {
              allocate_element(pt);
              (*pt)[(*pt)->size - 1].delay = 1;
            }

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    }
    else if (!strncmp(command, "MBUTTONDOWN", strlen("MBUTTONDOWN"))) {
        if (*pt) {
            rbutton = get_mouse_event_id_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_MOUSEBUTTONDOWN;
            (*pt)[(*pt)->size - 1].event.button.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "MBUTTONUP", strlen("MBUTTONUP"))) {
        if (*pt) {
            rbutton = get_mouse_event_id_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_MOUSEBUTTONUP;
            (*pt)[(*pt)->size - 1].event.button.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "MBUTTON", strlen("MBUTTON"))) {
        if (*pt) {
            rbutton = get_mouse_event_id_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_MOUSEBUTTONDOWN;
            (*pt)[(*pt)->size - 1].event.button.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
      
            delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
            for(i=0; i<delay_nb; ++i)
            {
              allocate_element(pt);
              (*pt)[(*pt)->size - 1].delay = 1;
            }

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_MOUSEBUTTONUP;
            (*pt)[(*pt)->size - 1].event.button.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    }
    else if (!strncmp(command, "JBUTTONDOWN", strlen("JBUTTONDOWN"))) {
        if (*pt) {
            rbutton = atoi(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_JOYBUTTONDOWN;
            (*pt)[(*pt)->size - 1].event.jbutton.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "JBUTTONUP", strlen("JBUTTONUP"))) {
        if (*pt) {
            rbutton = atoi(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_JOYBUTTONUP;
            (*pt)[(*pt)->size - 1].event.jbutton.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "JBUTTON", strlen("JBUTTON"))) {
        if (*pt) {
            rbutton = atoi(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_JOYBUTTONDOWN;
            (*pt)[(*pt)->size - 1].event.jbutton.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;

            delay_nb = ceil((double)DEFAULT_DELAY / (refresh_rate/1000));
            for(i=0; i<delay_nb; ++i)
            {
              allocate_element(pt);
              (*pt)[(*pt)->size - 1].delay = 1;
            }

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_JOYBUTTONUP;
            (*pt)[(*pt)->size - 1].event.jbutton.button = rbutton;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    }
    else if (!strncmp(command, "DELAY", strlen("DELAY"))) {
        if (*pt) {
            delay_nb = ceil((double)atoi(argument) / (refresh_rate/1000));
            for(i=0; i<delay_nb; ++i)
            {
              allocate_element(pt);
              (*pt)[(*pt)->size - 1].delay = 1;
            }
        }
    }

    return;
}

/*
 * Displays macro_table.
 */
/*void dump_scripts() {
    s_macro_event_delay** p_table;
    s_macro_event_delay* p_element;
    int delay_nb;

    for (p_table = macro_table; p_table < macro_table + SDLK_LAST; p_table++) {
        if (*p_table) {
            delay_nb = 0;
            printf("MACRO %s\n\n", (*p_table)->macro);
            for (p_element = *p_table; p_element && p_element < *p_table
                    + (*p_table)->size; p_element++) {
                if (p_element->delay) {
                    delay_nb++;
                }
                else if(delay_nb)
                {
                    printf("DELAY %d\n", delay_nb*(refresh_rate/1000));
                    delay_nb = 0;
                }
                if (p_element->event.type == SDL_KEYDOWN) {
                    printf("KEYDOWN %s\n", get_chars_from_key(
                            p_element->event.key.keysym.sym));
                } else if (p_element->event.type == SDL_KEYUP) {
                    printf("KEYUP %s\n", get_chars_from_key(
                            p_element->event.key.keysym.sym));
                }
            }
            printf("\n");
        }
    }
}*/

/*
 * Reads macros from script files.
 */
void read_macros() {

    int dtype = -1, button, down;
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
      printf("Warning: can't open macro directory %s\n", dir_path);
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
                  process_line(line, &dtype, &button, &down);
                  if(dtype < 0)
                  {
                      break;
                  }
              }
          }
          fclose(fp);
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
void initialize_macros() {
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
    initialize_macro_table();
    read_macros();
}

typedef struct
{
  int dtype;
  int did;
  int button;
  int down;
  int index;
} s_running_macro;

/*
 * This table contains pending macros.
 * Dynamically allocated.
 */
s_running_macro* running_macro = { NULL };
unsigned int running_macro_nb;

/*
 * Unregister a macro.
 */
static int macro_delete(int dtype, int did, int button, int down)
{
  s_running_macro* running_macro_realloc;
  int i;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(running_macro[i].dtype == dtype
    && running_macro[i].did == did 
    && running_macro[i].button == button 
    && running_macro[i].down == down)
    {
      memcpy(running_macro+i, running_macro+i+1, (running_macro_nb-i-1)*sizeof(s_running_macro));
      running_macro_realloc = realloc(running_macro, (running_macro_nb-1)*sizeof(s_running_macro));
      if(running_macro_realloc || !(running_macro_nb-1))
      {
        running_macro = running_macro_realloc;
        running_macro_nb--;
      }
      return 1;
    }
  }
  return 0;
}

/*
 * Register a new macro.
 */
static void macro_add(int dtype, int did, int button, int down)
{
  s_running_macro* running_macro_realloc = realloc(running_macro, (running_macro_nb+1)*sizeof(s_running_macro));
  if(running_macro_realloc)
  {
    running_macro = running_macro_realloc;
    running_macro_nb++;
    running_macro[running_macro_nb-1].dtype = dtype;
    running_macro[running_macro_nb-1].did = did;
    running_macro[running_macro_nb-1].button = button;
    running_macro[running_macro_nb-1].down = down;
    running_macro[running_macro_nb-1].index = 0;
  }
}

/*
 * Start or stop a macro.
 */
void macro_lookup(int dtype, int did, int button, int down)
{
  did = merge_all_devices ? 0 : did;
  if(macro_table[dtype][button][down])
  {
    if(!macro_delete(dtype, did, button, down))
    {
      macro_add(dtype, did, button, down);
    }
  }
}

/*
 * Generate events for pending macros.
 */
void macro_process()
{
  int i;
  SDL_Event event;
  s_macro_event_delay* p_macro_table;
  for(i=0; i<running_macro_nb; ++i)
  {
    if(running_macro[i].index < macro_table[running_macro[i].dtype][running_macro[i].button][running_macro[i].down]->size)
    {
      running_macro[i].index++;
    
      while(running_macro[i].index < macro_table[running_macro[i].dtype][running_macro[i].button][running_macro[i].down]->size)
      {
        p_macro_table = macro_table[running_macro[i].dtype][running_macro[i].button][running_macro[i].down] + running_macro[i].index;
        event = p_macro_table->event;
        int dtype = E_DEVICE_TYPE_UNKNOWN;
        switch(event.type)
        {
          case SDL_KEYDOWN:
          case SDL_KEYUP:
            dtype = E_DEVICE_TYPE_KEYBOARD;
          break;
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
            dtype = E_DEVICE_TYPE_MOUSE;
          break;
          case SDL_JOYBUTTONDOWN:
          case SDL_JOYBUTTONUP:
            dtype = E_DEVICE_TYPE_JOYSTICK;
          break;
        }
        if(dtype != E_DEVICE_TYPE_UNKNOWN)
        {
          int controller = device_controller[running_macro[i].dtype][running_macro[i].did];
          if(controller >= 0)
          {
            event.key.which = controller_device[dtype-1][controller];
            SDL_PushEvent(&event);
          }
        }
        if(!p_macro_table->delay)
        {
          running_macro[i].index++;
        }
        else
        {
          break;
        }
      }
    }
    if(running_macro[i].index >= macro_table[running_macro[i].dtype][running_macro[i].button][running_macro[i].down]->size)
    {
      macro_delete(running_macro[i].dtype, running_macro[i].did, running_macro[i].button, running_macro[i].down);
      i--;
    }
  }
}
