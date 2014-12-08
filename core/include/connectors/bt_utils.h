/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_UTILS_H_
#define BT_UTILS_H_

#include <stdint.h>
#include <btstack/hci_cmds.h>

#ifdef WIN32
/* BD Address */
typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

int bachk(const char *str);
int ba2str(const bdaddr_t *ba, char *str);
int str2ba(const char *str, bdaddr_t *ba);

int bt_connect();
int bt_send_cmd(const hci_cmd_t *cmd, ...);
#endif

int bt_get_device_bdaddr(int device_number, char bdaddr[18]);
int bt_write_device_class(int device_number, uint32_t devclass);
int bt_disconnect(char bdaddr[18]);

#endif /* BT_UTILS_H_ */
