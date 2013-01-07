
#ifndef EVENTS_H_
#define EVENTS_H_

#include <GE.h>

#define MAX_DEVICES 256

#define DEVTYPE_KEYBOARD 0x01
#define DEVTYPE_MOUSE    0x02
#define DEVTYPE_JOYSTICK 0x04
#define DEVTYPE_NB       3

#define MAX_KEYNAMES 249
#define MOUSE_BUTTONS_MAX 12

extern char* keynames[MAX_KEYNAMES];

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
