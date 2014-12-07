/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_UTILS_H_
#define BT_UTILS_H_

#include <stdint.h>

#ifdef WIN32
#define OPCODE(ogf, ocf) (ocf | ogf << 10)

#define OGF_INFORMATIONAL_PARAMETERS 0x04

typedef struct {
  uint16_t    opcode;
  const char *format;
} hci_cmd_t;

const hci_cmd_t hci_read_bd_addr = {
  .opcode = OPCODE(OGF_INFORMATIONAL_PARAMETERS, 0x09),
  .format = ""
};

/* BD Address */
typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

int bachk(const char *str);
int ba2str(const bdaddr_t *ba, char *str);
int str2ba(const char *str, bdaddr_t *ba);

int bt_connect();
int bt_send_cmd(int fd, const hci_cmd_t *cmd, ...);
#endif

int bt_get_device_bdaddr(int device_number, char bdaddr[18]);
int bt_write_device_class(int device_number, uint32_t devclass);
int bt_disconnect(char bdaddr[18]);

#endif /* BT_UTILS_H_ */
