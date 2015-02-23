/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#ifndef BT_UTILS_H_
#define BT_UTILS_H_

#include <stdint.h>

#ifdef WIN32
#include <btstack/hci_cmds.h>
#include <hci.h>
#else
#include <bluetooth/bluetooth.h>
#endif

#ifdef WIN32
#define ACL_MTU 1024
#define L2CAP_MTU 1024

/* BD Address */
typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

int bachk(const char *str);
void baswap(bdaddr_t *dst, bdaddr_t *src);
int ba2str(const bdaddr_t *ba, char *str);
int str2ba(const char *str, bdaddr_t *ba);

typedef struct {
    uint16_t type;
    uint16_t channel;
    uint16_t length;
} packet_header_t;

typedef struct {
  uint8_t buffer[sizeof(packet_header_t)+HCI_ACL_BUFFER_SIZE];
  uint16_t read;
  uint16_t remaining;
} recv_data_t;

int bt_connect();
int bt_send_cmd(int fd, const hci_cmd_t *cmd, ...);
int bt_recv_packet(int fd, recv_data_t* recv_data);

int bt_get_device_bdaddr(int fd, int device_number, bdaddr_t* bdaddr);
int bt_write_device_class(int fd, int device_number, uint32_t devclass);
int bt_disconnect(int fd, unsigned short handle);
#else
int bt_get_device_bdaddr(int device_number, bdaddr_t* bdaddr);
int bt_write_device_class(int device_number, uint32_t devclass);
int bt_disconnect(char bdaddr[18]);
#endif

#endif /* BT_UTILS_H_ */
