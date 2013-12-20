
#ifndef EVENTS_H_
#define EVENTS_H_

#include <GE.h>

#define MAX_EVENTS 256

int ev_init();
void ev_quit();

void ev_set_caption (const char*, const char*);

void ev_joystick_close(int);
const char* ev_joystick_name(int);
const char* ev_mouse_name(int);
const char* ev_keyboard_name(int);

void ev_grab_input(int);
void ev_set_callback(int (*)(GE_Event*));
void ev_pump_events();
int ev_push_event(GE_Event*);

int ev_peep_events(GE_Event*, int);

void ev_register_source(int fd, int id, int (*fd_read)(int), void (*fd_cleanup)(int));
void ev_remove_source(int fd);
inline void ev_set_next_event(GE_Event* event);

#endif /* EVENTS_H_ */
