
/* Sixaxis emulator

   Copyright (c) 2010 Mathieu Laurendeau

   License: GPLv3
*/

#include <pthread.h>
#include <SDL/SDL.h>
#include <limits.h>
#include "macros.h"
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include "conversion.h"

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

typedef struct {
  int keyboard_id;
  s_macro_event_delay* macro;
} s_macro_arg;

s_macro_arg macro_arg;

extern char* homedir;

/*
 * This table is used to store all the macros that are read from script files at the initialization of the process.
 */
s_macro_event_delay* macro_table[SDLK_LAST];

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

    for (pt = macro_table; pt < macro_table + SDLK_LAST; pt++) {
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
 * Returns the macro index in macro_table.
 * If line is a macro definition and the macro is already defined, returns SDLK_UNKNOWN.
 * If macro is SDLK_UNKNOWN and if line is not a macro definition, nothing is done.
 */
SDLKey process_line(const char* line, SDLKey macro) {
    char command[LINE_MAX];
    char argument[LINE_MAX];
    SDLKey key;
    int delay;
    s_macro_event_delay** pt = NULL;
    int ret;

    if (macro != SDLK_UNKNOWN) {
        pt = macro_table + macro;
    }

    ret = sscanf(line, "%s %s", command, argument);

    if(ret < 2)
    {
        /* invalid line */
        return macro;
    }

    if (!strncmp(command, "MACRO", strlen("MACRO"))) {
        macro = get_key_from_buffer(argument);
        pt = macro_table + macro;

        if (!*pt) {
            (*pt) = calloc(1, sizeof(s_macro_event_delay));
            (*pt)->size = 1;
            strncpy((*pt)->macro, argument, MAX_NAME_LENGTH);
        } else {
            printf("Macro %s defined twice!\n", argument);
            macro = SDLK_UNKNOWN;
        }
    } else if (!strncmp(command, "KEYDOWN", strlen("KEYDOWN"))) {
        if ((*pt)) {
            key = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.state = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = key;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "KEYUP", strlen("KEYUP"))) {
        if ((*pt)) {
            key = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.state = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = key;
            (*pt)[(*pt)->size - 1].delay = 0;
        }
    } else if (!strncmp(command, "KEY", strlen("KEY"))) {
        if ((*pt)) {
            key = get_key_from_buffer(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.state = SDL_KEYDOWN;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = key;
            (*pt)[(*pt)->size - 1].delay = DEFAULT_DELAY;

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].event.type = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.state = SDL_KEYUP;
            (*pt)[(*pt)->size - 1].event.key.keysym.sym = key;
        }
    } else if (!strncmp(command, "DELAY", strlen("DELAY"))) {
        if ((*pt)) {
            delay = atoi(argument);

            allocate_element(pt);

            (*pt)[(*pt)->size - 1].delay = delay;
        }
    }

    return macro;
}

/*
 * Displays macro_table.
 */
void dump_scripts() {
    s_macro_event_delay** p_table;
    s_macro_event_delay* p_element;

    for (p_table = macro_table; p_table < macro_table + SDLK_LAST; p_table++) {
        if (*p_table) {
            printf("MACRO %s\n\n", (*p_table)->macro);
            for (p_element = *p_table; p_element && p_element < *p_table
                    + (*p_table)->size; p_element++) {
                if (p_element->event.type == SDL_KEYDOWN) {
                    printf("KEYDOWN %s\n", get_chars_from_key(
                            p_element->event.key.keysym.sym));
                } else if (p_element->event.type == SDL_KEYUP) {
                    printf("KEYUP %s\n", get_chars_from_key(
                            p_element->event.key.keysym.sym));
                }
                if (p_element->delay) {
                    printf("DELAY %d\n", p_element->delay);
                }
            }
            printf("\n");
        }
    }
}

/*
 * Reads macros from script files.
 */
void read_macros() {

    SDLKey macro = SDLK_UNKNOWN;
    char line[LINE_MAX];
    DIR *dirp;
    FILE* fp;
    char dir_path[PATH_MAX];
    char file_path[PATH_MAX];
    struct dirent *d;
    unsigned int i;
    unsigned int nb_filenames = 0;
    char** filenames = NULL;
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
        nb_filenames++;
        filenames = realloc(filenames, nb_filenames*sizeof(char*));
        filenames[nb_filenames-1] = calloc(strlen(d->d_name)+1, sizeof(char));
        strncpy(filenames[nb_filenames-1], d->d_name, strlen(d->d_name));
      }
#else
      snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, d->d_name);
      if(stat(file_path, &buf) == 0)
      {
        if(S_ISREG(buf.st_mode))
        {
          nb_filenames++;
          filenames = realloc(filenames, nb_filenames*sizeof(char*));
          filenames[nb_filenames-1] = calloc(strlen(d->d_name)+1, sizeof(char));
          strncpy(filenames[nb_filenames-1], d->d_name, strlen(d->d_name));
        }
      }
#endif
    }

    closedir(dirp);

    for(i=0; i<nb_filenames; ++i)
    {
      snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, filenames[i]);
      fp = fopen(file_path, "r");
      if (!fp) {
          fprintf(stderr, "Can not find '%s'\n", file_path);
      } else {
          while (fgets(line, LINE_MAX, fp)) {
              if (line[0] != '#') {
                  macro = process_line(line, macro);
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
    initialize_macro_table();
    read_macros();
}

/*
 * This code is run by each thread that executes a macro.
 */
void macro(s_macro_arg* p_macro_arg) {
    SDL_Event event;
    s_macro_event_delay* p_element;
    s_macro_event_delay* p_table = p_macro_arg->macro;

    for (p_element = p_table; p_element < p_table + p_table->size; p_element++) {
        if (p_element->event.type != SDL_NOEVENT) {
            event = p_element->event;
            event.key.which = p_macro_arg->keyboard_id;
            SDL_PushEvent(&event);
        }
        if (p_element->delay > 0) {
            usleep(p_element->delay*1000);
        }
    }
    pthread_exit (0);
}

/*
 * Launches a thread if there is a macro for that key.
 */
void macro_lookup(int keyboard_id, SDLKey key)
{
    pthread_t thread;
    pthread_attr_t thread_attr;

    if(macro_table[key]) {
        macro_arg.keyboard_id = keyboard_id;
        macro_arg.macro = macro_table[key];
        pthread_attr_init(&thread_attr);
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
        pthread_create( &thread, &thread_attr, (void*)macro, (void*) &macro_arg);
    }
}
