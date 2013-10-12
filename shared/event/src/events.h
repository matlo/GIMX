
#ifndef EVENTS_H_
#define EVENTS_H_

#include <GE.h>

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

#endif /* EVENTS_H_ */
