/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef BTSTACK_COMMON_H_
#define BTSTACK_COMMON_H_

#include <hci.h>

#define ACL_MTU 1024
#define L2CAP_MTU 1024

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

int btstack_common_init();
int btstack_common_getfd();
int btstack_common_recv(recv_data_t* recv_data);
int btstack_common_recv_packet(recv_data_t* recv_data);
int btstack_common_send_packet(uint16_t type, uint16_t cid, const unsigned char * buf, uint32_t len);
int btstack_common_send_cmd(const hci_cmd_t *cmd, ...);
int btstack_common_disconnect(uint16_t handle);

#endif /* BTSTACK_COMMON_H_ */
