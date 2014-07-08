/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, unsigned short psm, int options);

int l2cap_send(int, const unsigned char*, int, int);
int acl_send_data (const char *bdaddr_dst, unsigned short cid, unsigned char *data, unsigned short plen);

int l2cap_recv(int, unsigned char*, int len);

int l2cap_set_flush_timeout(bdaddr_t *ba, int timeout_ms);

int l2cap_listen(unsigned short psm, int options);
int l2cap_accept(int s, bdaddr_t* src, unsigned short* psm, unsigned short* cid);

int l2cap_is_connected(int fd);
