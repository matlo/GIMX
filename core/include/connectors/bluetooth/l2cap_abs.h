/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef L2CAP_ABS_H_
#define L2CAP_ABS_H_

#ifndef WIN32
#include <bluetooth/bluetooth.h>
#else
#include <connectors/windows/bt_bdaddr.h>
#endif

#define L2CAP_ABS_MAX_PEERS     7 // = MAX_CONTROLLERS
#define L2CAP_ABS_MAX_CHANNELS  3 // = PSM_SDP + PSM_HID_CONTROL + PSM_HID_INTERRUPT

#define PSM_SDP           0x01
#define PSM_HID_CONTROL   0x11
#define PSM_HID_INTERRUPT 0x13

typedef enum
{
  E_L2CAP_ABS_BTSTACK,
  E_L2CAP_ABS_BLUEZ,
  E_L2CAP_ABS_MAX,
} e_l2cap_abs;

#define L2CAP_ABS_LM_MASTER  0x01
#define L2CAP_ABS_LM_AUTH    0x02
#define L2CAP_ABS_LM_ENCRYPT 0x04

typedef int (* L2CAP_ABS_GENERIC_CALLBACK) (int user);
typedef L2CAP_ABS_GENERIC_CALLBACK L2CAP_ABS_CLOSE_CALLBACK;
typedef L2CAP_ABS_GENERIC_CALLBACK L2CAP_ABS_CONNECT_CALLBACK;
typedef L2CAP_ABS_GENERIC_CALLBACK L2CAP_ABS_READ_CALLBACK;
typedef int (* L2CAP_ABS_LISTEN_ACCEPT_CALLBACK) (int channel, bdaddr_t * src);

typedef int (* L2CAP_ABS_CONNECT) (const char *bdaddr_src, const char *bdaddr_dest, unsigned short psm, int options,
    int user, L2CAP_ABS_CONNECT_CALLBACK callback, L2CAP_ABS_CLOSE_CALLBACK close_callback);
typedef int (* L2CAP_ABS_LISTEN) (int user, unsigned short psm, int options,
    L2CAP_ABS_LISTEN_ACCEPT_CALLBACK read_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback);
typedef int (* L2CAP_ABS_SEND) (int channel, const unsigned char* buf, int len, int blocking);
typedef int (* L2CAP_ABS_RECV) (int channel, unsigned char* buf, int len);
typedef int (* L2CAP_ABS_CLOSE) (int channel);
typedef int (* L2CAP_ABS_PACKET_CALLBACK) (int user, int psm, const unsigned char *buf, int len);
typedef void (* L2CAP_ABS_ADD_SOURCE) (int channel, int user,
    L2CAP_ABS_READ_CALLBACK read_callback, L2CAP_ABS_PACKET_CALLBACK packet_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback);

typedef int (* L2CAP_ABS_DISCONNECT) (int channel);

typedef struct
{
  L2CAP_ABS_CONNECT connect;
  L2CAP_ABS_LISTEN listen;
  L2CAP_ABS_SEND send;
  L2CAP_ABS_RECV recv;
  L2CAP_ABS_CLOSE close;
  L2CAP_ABS_ADD_SOURCE add_source;
  L2CAP_ABS_DISCONNECT disconnect;
} s_l2cap_abs;

void l2cap_abs_register(e_l2cap_abs index, s_l2cap_abs * value);
s_l2cap_abs * l2cap_abs_get();

#endif /* L2CAP_ABS_H_ */
