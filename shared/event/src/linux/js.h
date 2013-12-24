/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef JS_H_
#define JS_H_

#include <poll.h>

int js_init();
int js_get_nfds();
int js_fill_fds(nfds_t max, struct pollfd fds[]);
int js_close(int id);
void js_quit();
const char* js_get_name(int index);
void js_set_callback(int (*fp)(GE_Event*));

#endif /* JS_H_ */
