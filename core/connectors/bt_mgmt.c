/*
 Copyright (c) 2014 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3

 This source code is highly inspired by the bluez source code.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <unistd.h>
#include <bluetooth/mgmt.h>
#include <sys/param.h>

#include <poll.h>

#include <connectors/bt_utils.h>
#include <connectors/bt_utils.h>
#include <emuclient.h>
#include "../../directories.h"

#define TRUE 1
#define FALSE 0

#define MGMT_BUF_SIZE 1024

#ifndef BDADDR_BREDR
#define BDADDR_BREDR 0x00
#endif

typedef enum
{
  E_INIT,
  E_SET_POWERED,
  E_SET_CONNECTABLE,
  E_SET_LOCAL_NAME,
  E_LOAD_LINK_KEYS,
  E_DONE
}e_step;

static e_step step = E_INIT;

int mgmt_create(void)
{
  struct sockaddr_hci addr;
  int sk;

  sk = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC | SOCK_NONBLOCK, BTPROTO_HCI);
  if (sk < 0)
    return -errno;

  memset(&addr, 0, sizeof(addr));
  addr.hci_family = AF_BLUETOOTH;
  addr.hci_dev = HCI_DEV_NONE;
  addr.hci_channel = HCI_CHANNEL_CONTROL;

  if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    int err = -errno;
    close(sk);
    return err;
  }

  return sk;
}

static int mgmt_set_mode(int sk, uint16_t index, uint16_t opcode, uint8_t val)
{
  char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_mode)];
  struct mgmt_hdr *hdr = (void *) buf;
  struct mgmt_mode *cp = (void *) &buf[sizeof(*hdr)];

  memset(buf, 0, sizeof(buf));
  hdr->opcode = htobs(opcode);
  hdr->index = htobs(index);
  hdr->len = htobs(sizeof(*cp));

  cp->val = val;

  if (write(sk, buf, sizeof(buf)) < 0)
    return -errno;

  return 0;
}

static int mgmt_set_local_name(int sk, uint16_t index)
{
  char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_cp_set_local_name)];
  struct mgmt_hdr *hdr = (void *) buf;
  struct mgmt_cp_set_local_name *cp = (void *) &buf[sizeof(*hdr)];

  memset(buf, 0, sizeof(buf));
  hdr->opcode = htobs(MGMT_OP_SET_LOCAL_NAME);
  hdr->index = htobs(index);
  hdr->len = htobs(sizeof(*cp));

  char name[] = "Wireless Controller";

  memcpy(cp->name, name, sizeof(name));

  if (write(sk, buf, sizeof(buf)) < 0)
    return -errno;

  return 0;
}

static int mgmt_cmd_complete(int sk, uint16_t index, void *buf, size_t len)
{
  struct mgmt_ev_cmd_complete *ev = buf;
  uint16_t opcode;
  int ret = 0;

  if (len < sizeof(*ev)) {
    fprintf(stderr, "Too small management command complete event packet\n");
    return -1;
  }

  opcode = btohs(bt_get_unaligned(&ev->opcode));

  len -= sizeof(*ev);

  switch (opcode) {
  case MGMT_OP_SET_POWERED:
    if(step != E_SET_POWERED) {
      fprintf(stderr, "unexpected set_powered complete\n");
    }
    else if(ev->status)
    {
      fprintf(stderr, "set_powered failed\n");
      ret = -1;
    }
    break;
  case MGMT_OP_SET_LOCAL_NAME:
    if(step != E_SET_LOCAL_NAME) {
      fprintf(stderr, "unexpected set_local_name complete\n");
    }
    else if(ev->status)
    {
      fprintf(stderr, "set_local_name failed\n");
      ret = -1;
    }
    break;
  case MGMT_OP_SET_CONNECTABLE:
    if(step != E_SET_CONNECTABLE) {
      fprintf(stderr, "unexpected set_connectable complete\n");
    }
    else if(ev->status)
    {
      fprintf(stderr, "set_powered failed\n");
      ret = -1;
    }
    break;
  /*case MGMT_OP_SET_DEV_CLASS:
    break;*/
  case MGMT_OP_LOAD_LINK_KEYS:
    if(step != E_LOAD_LINK_KEYS) {
      fprintf(stderr, "unexpected load_link_keys complete\n");
    }
    else if(ev->status)
    {
      fprintf(stderr, "load_link_keys failed\n");
      ret = -1;
    }
    break;
  default:
    fprintf(stderr, "Unhandled command complete for opcode %u\n", opcode);
    ret = -1;
    break;
  }

  return ret;
}

static int mgmt_read_evt(int sk, int eindex)
{
  int ret;
  unsigned char buf[MGMT_BUF_SIZE];
  struct mgmt_hdr *hdr = (void *) buf;

  struct pollfd pfd =
  {
    .fd = sk,
    .events = POLLIN
  };

  if((ret = poll(&pfd, 1, 5000)) <= 0)
  {
    fprintf(stderr, "Unable to poll management socket: %s (%d)\n", strerror(errno), errno);
    return -1;
  }

  if(pfd.revents != POLLIN)
  {
    fprintf(stderr, "Bad event from management socket\n");
    return -1;
  }

  if((ret = read(sk, buf, sizeof(buf))) < 0)
  {
    fprintf(stderr, "Unable to read from management socket: %s (%d)\n", strerror(errno), errno);
    return -1;
  }

  /*int i;
  for(i=0; i<ret; ++i)
  {
    if(!(i%16))
    {
      printf("\n");
    }
    printf("0x%02x ", buf[i]);
  }
  printf("\n");*/

  uint16_t opcode = btohs(bt_get_unaligned(&hdr->opcode));
  uint16_t len = btohs(bt_get_unaligned(&hdr->len));
  uint16_t index = btohs(bt_get_unaligned(&hdr->index));

  if (ret != MGMT_HDR_SIZE + len) {
    fprintf(stderr, "Packet length mismatch. ret %d len %u\n", ret, len);
    return -1;
  }

  switch (opcode) {
  case MGMT_EV_CMD_COMPLETE:
    return mgmt_cmd_complete(sk, index, buf + MGMT_HDR_SIZE, len);
  default:
    fprintf(stderr, "Unknown Management opcode %u (index %u)\n", opcode, index);
    break;
  }

  return 0;
}

static int mgmt_load_link_keys(int sk, uint16_t index, uint16_t nb_keys, bdaddr_t bdaddrs[nb_keys], unsigned char keys[nb_keys][16])
{
  char *buf;
  struct mgmt_hdr *hdr;
  struct mgmt_cp_load_link_keys *cp;
  struct mgmt_link_key_info *key;
  size_t cp_size;
  int err;

  cp_size = sizeof(*cp) + (nb_keys * sizeof(*key));

  buf = calloc(sizeof(*hdr) + cp_size, 1);
  if (buf == NULL)
    return -ENOMEM;

  hdr = (void *) buf;
  hdr->opcode = htobs(MGMT_OP_LOAD_LINK_KEYS);
  hdr->len = htobs(cp_size);
  hdr->index = htobs(index);

  cp = (void *) (buf + sizeof(*hdr));
  cp->debug_keys = 0x00;
  cp->key_count = htobs(nb_keys);

  unsigned int i;
  for (i=0, key=cp->keys; i<nb_keys; ++i, ++key) {

    bacpy(&key->addr.bdaddr, bdaddrs+i);
    key->addr.type = BDADDR_BREDR;
    key->type = HCI_LK_UNAUTH_COMBINATION;
    memcpy(key->val, keys[i], 16);
    key->pin_len = 0x00;
  }

  if (write(sk, buf, sizeof(*hdr) + cp_size) < 0)
    err = -errno;
  else
    err = 0;

  free(buf);

  return err;
}

static int str2buf(const char *str, uint8_t *buf, size_t blen)
{
  int i, dlen;

  if (str == NULL)
    return -EINVAL;

  memset(buf, 0, blen);

  dlen = MIN((strlen(str) / 2), blen);

  for (i = 0; i < dlen; i++)
    sscanf(str + (i * 2), "%02hhX", &buf[i]);

  return 0;
}

static int read_link_keys(uint16_t index, uint16_t nb_keys, bdaddr_t bdaddrs[nb_keys], unsigned char keys[nb_keys][16])
{
  char file_path[PATH_MAX];

  char dongle_bdaddr[18];

  if(bt_get_device_bdaddr(index, dongle_bdaddr) < 0)
  {
    fprintf(stderr, "can't read device bdaddr\n");
    return -1;
  }

  snprintf(file_path, sizeof(file_path), "%s%s%s%s/%s", emuclient_params.homedir, GIMX_DIR, BT_DIR, dongle_bdaddr, "linkkeys");

  char line[LINE_MAX];

  FILE* file = fopen(file_path, "r");
  int i = 0;

  if (!file)
  {
    fprintf(stderr, "Can't open '%s'.\n", file_path);
    return -1;
  }
  else
  {
    while (fgets(line, LINE_MAX, file) && i < nb_keys)
    {
      line[17] = '\0';
      str2ba(line, bdaddrs+i);
      str2buf(line+18, keys[i], sizeof(keys[i]));
      ++i;
    }
    fclose(file);
  }

  return 0;
}

int bt_mgmt_adapter_init(uint16_t index)
{
  int sk;

  if((sk = mgmt_create()) < 0)
  {
    fprintf(stderr, "can't open a management socket\n");
    return -1;
  }

  step = E_SET_POWERED;

  if(mgmt_set_mode(sk, index, MGMT_OP_SET_POWERED, TRUE) < 0)
  {
    close(sk);
    return -1;
  }

  if(mgmt_read_evt(sk, index) < 0)
  {
    close(sk);
    return -1;
  }

  step = E_SET_LOCAL_NAME;

  if(mgmt_set_local_name(sk, index) < 0)
  {
    close(sk);
    return -1;
  }

  if(mgmt_read_evt(sk, index) < 0)
  {
    close(sk);
    return -1;
  }

  step = E_SET_CONNECTABLE;

  if(mgmt_set_mode(sk, index, MGMT_OP_SET_CONNECTABLE, TRUE) < 0)
  {
    close(sk);
    return -1;
  }

  if(mgmt_read_evt(sk, index) < 0)
  {
    close(sk);
    return -1;
  }

  step = E_LOAD_LINK_KEYS;

  bdaddr_t bdaddrs[2];
  unsigned char keys[2][16];

  if(read_link_keys(index, 2, bdaddrs, keys) < 0)
  {
    close(sk);
    return -1;
  }

  if(mgmt_load_link_keys(sk, index, 2, bdaddrs, keys) < 0)
  {
    close(sk);
    return -1;
  }

  if(mgmt_read_evt(sk, index) < 0)
  {
    close(sk);
    return -1;
  }

  close(sk);

  return 0;
}
