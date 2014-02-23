
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

#ifndef WIN32
int ev_joystick_has_ff_rumble(int joystick);
int ev_joystick_set_ff_rumble(int joystick, unsigned short weak_timeout, unsigned short weak, unsigned short strong_timeout, unsigned short strong);
#endif

void ev_grab_input(int);
void ev_set_callback(int (*)(GE_Event*));
void ev_pump_events();
int ev_push_event(GE_Event*);

int ev_peep_events(GE_Event*, int);

void ev_register_source(int fd, int id, int (*fp_read)(int), int (*fp_write)(int), int (*fd_cleanup)(int));
void ev_remove_source(int fd);
inline void ev_set_next_event(GE_Event* event);

#endif /* EVENTS_H_ */
