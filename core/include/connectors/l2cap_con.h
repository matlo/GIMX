/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef WIN32
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#else
#include <connectors/bt_utils.h>
//TODO MLA
#define L2CAP_LM_MASTER 0
#define L2CAP_LM_AUTH 0
#define L2CAP_LM_ENCRYPT 0
#endif

#ifndef WIN32
int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, unsigned short psm, int options);
int l2cap_send(int fd, const unsigned char* buf, int len, int blocking);
#else
int l2cap_connect(int fd, const char *bdaddr_dest, unsigned short psm);
int l2cap_send(int fd, unsigned short channel, const unsigned char* buf, int len);
#endif

int acl_send_data (const char *bdaddr_dst, unsigned short cid, unsigned char *data, unsigned short plen);

int l2cap_recv(int, unsigned char*, int len);

int l2cap_set_flush_timeout(bdaddr_t *ba, int timeout_ms);

int l2cap_listen(unsigned short psm, int options);
int l2cap_accept(int s, bdaddr_t* src, unsigned short* psm, unsigned short* cid);

int l2cap_is_connected(int fd);
