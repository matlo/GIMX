/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TCP_CON_H_
#define TCP_CON_H_

#include <arpa/inet.h>

#ifdef WIN32
#include <connectors/windows/sockets.h>
#else
#define psockerror(msg) perror(msg)
#endif

int tcp_connect(unsigned int ip, unsigned short port);
int tcp_close(int fd);
int tcp_send(int fd, const unsigned char* buf, int length);
int tcp_recv(int fd, unsigned char* buf, int len);

#endif /* TCP_CON_H_ */
