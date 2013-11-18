/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef TCP_CON_H_
#define TCP_CON_H_

int tcp_connect(char* ip, unsigned short port);
void tcp_close(int fd);
int tcp_send(int fd, const unsigned char* buf, int length);

#endif /* TCP_CON_H_ */
