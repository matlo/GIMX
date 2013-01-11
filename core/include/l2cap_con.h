/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

int l2cap_connect(const char*, const char*, int);

int l2cap_send(int, const unsigned char*, int, int);

int l2cap_recv(int, unsigned char*, int len);

#ifdef WIN32
int bachk(const char *str);
#endif
