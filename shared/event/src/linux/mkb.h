/*
 Copyright (c) 2013 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef MKB_H_
#define MKB_H_

#include <poll.h>

int mkb_init();
int mkb_close_device(int id);
void mkb_quit();
void mkb_grab(int mode);
int mkb_get_nfds();
int mkb_fill_fds(nfds_t max, struct pollfd fds[]);
char* mkb_get_k_name(int index);
char* mkb_get_m_name(int index);
void mkb_set_callback(int (*fp)(GE_Event*));

#endif /* MKB_H_ */
