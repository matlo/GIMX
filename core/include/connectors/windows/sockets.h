/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <winsock2.h>

#define MSG_DONTWAIT 0
#define MSG_NOSIGNAL 0

int wsa_init();
void wsa_count(int error);
void wsa_clean();
void psockerror(const char* msg);

#endif /* SOCKETS_H_ */
