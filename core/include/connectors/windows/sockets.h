/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

int wsa_init();
void wsa_count(int error);
void wsa_clean();
void psockerror(const char* msg);

#endif /* SOCKETS_H_ */
