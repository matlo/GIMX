/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/bluetooth/l2cap_abs.h>
#include <gimx.h>
#include <gpoll.h>

#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#define BT_SLOT 625 //microseconds

#ifdef BT_POWER
#warning "BT_POWER is already defined."
#else
#define BT_POWER 9
#define BT_POWER_FORCE_ACTIVE_OFF 0
#define BT_POWER_FORCE_ACTIVE_ON 1

struct bt_power {
  unsigned char force_active;
};
#endif

#define L2CAP_MTU 1024

#define HCI_REQ_TIMEOUT   1000

typedef struct
{
  unsigned int user;
  int devid;
  bdaddr_t ba_dst;
  uint16_t psm;
  uint16_t cid;
  uint16_t omtu;
  uint16_t handle;
  int fd;
  L2CAP_ABS_CONNECT_CALLBACK connect_callback;
  L2CAP_ABS_CLOSE_CALLBACK close_callback;
} s_channel;

static struct
{
  unsigned int nb;
  s_channel channels[L2CAP_ABS_MAX_PEERS*L2CAP_ABS_MAX_CHANNELS];
} channels = { 0, { } };

typedef struct
{
  int fd;
  L2CAP_ABS_LISTEN_ACCEPT_CALLBACK accept_callback;
  L2CAP_ABS_CLOSE_CALLBACK close_callback;
} s_listen_channel;

static struct
{
  unsigned int nb;
  s_listen_channel channels[L2CAP_ABS_MAX_CHANNELS];
} listen_channels = { 0, { } };

#if 0
static int l2cap_bluez_set_flush_timeout(int channel, int timeout_ms)
{
    int result = 0, dd;
    struct hci_request rq = { 0 };

    write_link_supervision_timeout_cp cmd_param;

    write_link_supervision_timeout_rp cmd_response;

    if ((dd = hci_open_dev(channels.channels[channel].devid)) < 0)
    {
      perror("hci_open_dev");
      return -1;
    }

    // build a command packet to send to the bluetooth microcontroller
    cmd_param.handle = channels.channels[channel].handle;
    cmd_param.timeout = htobs(timeout_ms/BT_SLOT);
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = OCF_WRITE_AUTOMATIC_FLUSH_TIMEOUT;
    rq.cparam = &cmd_param;
    rq.clen = sizeof(cmd_param);
    rq.rparam = &cmd_response;
    rq.rlen = sizeof(cmd_response);
    rq.event = EVT_CMD_COMPLETE;

    // send the command and wait for the response
    if( hci_send_req( dd, &rq, 0 ) < 0)
    {
      perror("hci_send_req");
      result = -1;
    }
    else if( cmd_response.status )
    {
      errno = bt_error(cmd_response.status);
      perror("failed to set flush timeout");
      result = -1;
    }

    hci_close_dev(dd);

    return result;
}
#endif

#define ACL_MTU 1024

/*
 * This function can be used to bypass the l2cap outgoing MTU check of the Linux kernel.
 * If plen is higher than ACL_MTU, it sends a segmented packet.
 */
static int l2cap_bluez_acl_send_data (int channel, unsigned char *data, unsigned short plen)
{
  int ret = -1, dd = -1;
  uint8_t type = HCI_ACLDATA_PKT;
  hci_acl_hdr acl_hdr;
  l2cap_hdr l2_hdr;
  struct iovec iv[4];
  int ivn;
  unsigned short data_len;

  if ((dd = hci_open_dev(channels.channels[channel].devid)) < 0)
  {
    perror("hci_open_dev");
    return -1;
  }

  data_len = ACL_MTU-1-HCI_ACL_HDR_SIZE-L2CAP_HDR_SIZE;
  if(plen < data_len)
  {
    data_len = plen;
  }

  iv[0].iov_base = &type;
  iv[0].iov_len = 1;

  acl_hdr.handle = htobs(acl_handle_pack(channels.channels[channel].handle, ACL_START));
  acl_hdr.dlen = htobs(data_len+L2CAP_HDR_SIZE);
  
  iv[1].iov_base = &acl_hdr;
  iv[1].iov_len = HCI_ACL_HDR_SIZE;

  l2_hdr.cid = htobs(channels.channels[channel].cid);
  l2_hdr.len = htobs(plen);

  iv[2].iov_base = &l2_hdr;
  iv[2].iov_len = L2CAP_HDR_SIZE;

  ivn = 3;
  
  if (data_len)
  {
    iv[3].iov_base = data;
    iv[3].iov_len = htobs(data_len);
    ivn = 4;
  }
  
  while ((ret = writev(dd, iv, ivn)) < 0)
  {
    if (errno == EAGAIN || errno == EINTR)
    {
      continue;
    }
    perror("writev");
    break;
  }
  
  if(ret != -1)
  {
    plen -= data_len;

    while(plen)
    {
      data += data_len;
      data_len = ACL_MTU-1-HCI_ACL_HDR_SIZE;
      if(plen < data_len)
      {
        data_len = plen;
      }

      iv[0].iov_base = &type;
      iv[0].iov_len = 1;

      acl_hdr.handle = htobs(acl_handle_pack(channels.channels[channel].handle, ACL_CONT));
      acl_hdr.dlen = htobs(plen);

      iv[1].iov_base = &acl_hdr;
      iv[1].iov_len = HCI_ACL_HDR_SIZE;
  
      iv[2].iov_base = data;
      iv[2].iov_len = htobs(data_len);
      ivn = 3;

      while ((ret = writev(dd, iv, ivn)) < 0)
      {
        if (errno == EAGAIN || errno == EINTR)
        {
          continue;
        }
        perror("writev");
        break;
      }

      if(ret == -1)
      {
        break;
      }

      plen -= data_len;
    }
  }
  
  hci_close_dev(dd);

  return ret;
}

static int l2cap_bluez_setsockopt(int fd, int link_mode)
{
  struct l2cap_options l2o = { 0 };
  socklen_t len = sizeof(l2o);

  int lm = 0;
  if(link_mode & L2CAP_ABS_LM_MASTER)
  {
    lm |= L2CAP_LM_MASTER;
  }
  if(link_mode & L2CAP_ABS_LM_AUTH)
  {
    lm |= L2CAP_LM_AUTH;
  }
  if(link_mode & L2CAP_ABS_LM_ENCRYPT)
  {
    lm |= L2CAP_LM_ENCRYPT;
  }
  if (setsockopt(fd, SOL_L2CAP, L2CAP_LM, &lm, sizeof(lm)) < 0)
  {
    perror("setsockopt L2CAP_LM");
    return -1;
  }

  if(getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len) < 0)
  {
    perror("getsockopt L2CAP_OPTIONS");
    return -1;
  }
  else
  {
    l2o.imtu = L2CAP_MTU;
    if(setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &l2o, sizeof(l2o)) < 0)
    {
      perror("setsockopt L2CAP_OPTIONS");
      return -1;
    }
  }

  /*if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
  {
    perror("fcntl O_NONBLOCK");
  }*/

  struct bt_power pwr = {.force_active = BT_POWER_FORCE_ACTIVE_OFF};
  if (setsockopt(fd, SOL_BLUETOOTH, BT_POWER, &pwr, sizeof(pwr)) < 0)
  {
    perror("setsockopt BT_POWER");
    return -1;
  }

  return 0;
}

static int l2cap_bluez_get_devid(bdaddr_t * ba_dst, int * devid)
{
  *devid = hci_get_route(ba_dst);
  if(*devid < 0)
  {
    fprintf(stderr, "can't get device id for destination\n");
    return -1;
  }
  return 0;
}

static int l2cap_bluez_get_cid(int fd, uint16_t * cid)
{
  int result = 0;

  struct sockaddr_l2 la;
  socklen_t len = sizeof(la);

  if(getsockname(fd, (struct sockaddr *)&la, &len) < 0)
  {
    perror("getsockname");
    result = -1;
  }
  else
  {
    *cid = la.l2_cid;
  }

  return result;
}

static int l2cap_bluez_get_handle(int fd, uint16_t * handle)
{
  int result = 0;

  struct l2cap_conninfo l2ci = { 0 };
  socklen_t len = sizeof(l2ci);

  if(getsockopt(fd, SOL_L2CAP, L2CAP_CONNINFO, &l2ci, &len) < 0)
  {
    perror("getsockopt L2CAP_CONNINFO");
    result = -1;
  }
  else
  {
    *handle = l2ci.hci_handle;
  }

  return result;
}

static int l2cap_bluez_get_outgoing_mtu(int fd, uint16_t * omtu)
{
  int result = 0;

  struct l2cap_options l2o = { 0 };
  socklen_t len = sizeof(l2o);

  if(getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len) < 0)
  {
    perror("getsockopt L2CAP_OPTIONS");
    result = -1;
  }
  else
  {
    *omtu = l2o.omtu;
  }

  return result;
}

static int l2cap_bluez_is_connected(int fd)
{
  int error = 0;
  socklen_t lerror = sizeof(error);

  int ret = getsockopt (fd, SOL_SOCKET, SO_ERROR, &error, &lerror);

  if(ret < 0)
  {
    perror("getsockopt SO_ERROR");
  }
  else
  {
    if(error == EINPROGRESS)
    {
      fprintf(stderr, "EINPROGRESS\n");
    }
    else if(error)
    {
      fprintf(stderr, "connection failed: %s\n", strerror(error));
    }
    else
    {
      return 1;
    }
  }
  return 0;
}

static int l2cap_bluez_connect_channel(int channel, int unused)
{
  int result = 0;

  int fd = channels.channels[channel].fd;

  if(l2cap_bluez_is_connected(fd))
  {
    gpoll_remove_fd(fd);

    if(channels.channels[channel].connect_callback(channels.channels[channel].user))
    {
      result = -1;
    }
    else
    {
      if(l2cap_bluez_get_outgoing_mtu(fd, &channels.channels[channel].omtu))
      {
        result = -1;
      }
      else
      {
        if(l2cap_bluez_get_handle(fd, &channels.channels[channel].handle))
        {
          result = -1;
        }
        else
        {
          if(l2cap_bluez_get_cid(fd, &channels.channels[channel].cid))
          {
            result = -1;
          }
          else
          {
            result = l2cap_bluez_get_devid(&channels.channels[channel].ba_dst,
                &channels.channels[channel].devid);
          }
        }
      }
    }
  }
  else
  {
    fprintf(stderr, "can't connect to psm 0x%04x\n", channels.channels[channel].psm);
    result = -1;
  }
  if(result)
  {
    channels.channels[channel].close_callback(channels.channels[channel].user);
  }
  return result;
}

static int l2cap_bluez_close_channel(int channel)
{
  return l2cap_bluez_connect_channel(channel, 0);
}

static int l2cap_bluez_connect(const char * bdaddr_src, const char * bdaddr_dest, unsigned short psm, int options,
    int user, L2CAP_ABS_CONNECT_CALLBACK connect_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
    int fd;
    struct sockaddr_l2 addr;

    if(channels.nb == sizeof(channels.channels) / sizeof(*channels.channels))
    {
      fprintf(stderr, "no space left for the l2cap connection (out)\n");
      return -1;
    }

    if ((fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET | SOCK_NONBLOCK, BTPROTO_L2CAP)) == -1)
    {
      perror("socket");
      return -1;
    }

    if(l2cap_bluez_setsockopt(fd, options) < 0)
    {
      close(fd);
      return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    str2ba(bdaddr_src, &addr.l2_bdaddr);
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm    = htobs(psm);
    str2ba(bdaddr_dest, &addr.l2_bdaddr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
      if(errno != EINPROGRESS)
      {
        perror("connect");
        close(fd);
        return -1;
      }
    }

    int channel = channels.nb;

    channels.channels[channel].fd = fd;

    bacpy(&channels.channels[channel].ba_dst, &addr.l2_bdaddr);
    channels.channels[channel].psm = psm;
    channels.channels[channel].user = user;
    channels.channels[channel].connect_callback = connect_callback;
    channels.channels[channel].close_callback = close_callback;

    gpoll_register_fd(fd, channel, NULL, l2cap_bluez_connect_channel, l2cap_bluez_close_channel);

    ++channels.nb;

    return channel;
}

static int l2cap_bluez_close(int channel)
{
  gpoll_remove_fd(channels.channels[channel].fd);
  close(channels.channels[channel].fd);
  channels.channels[channel].fd = -1;

  return 1;
}

static int l2cap_bluez_send(int channel, const unsigned char* buf, int len, int blocking)
{
  if(!channels.channels[channel].cid)
  {
    fprintf(stderr, "connection is still pending\n");
    return -1;
  }

  if(len > channels.channels[channel].omtu)
  {
    //bypass the kernel omtu check (usefull for the DS4)
    if(l2cap_bluez_acl_send_data(channel, (unsigned char*)buf, len) < 0)
    {
      fprintf(stderr, "acl_send_data failed\n");
      return -1;
    }
  }
  else
  {
    if(send(channels.channels[channel].fd, buf, len, blocking ? 0 : MSG_DONTWAIT) != len)
    {
      perror("send");
      return -1;
    }
  }
  return len;
}

static int l2cap_bluez_recv(int channel, unsigned char* buf, int len)
{
    return recv(channels.channels[channel].fd, buf, len, MSG_DONTWAIT);
}

static int l2cap_bluez_connect_accept(int listen_channel)
{
  struct sockaddr_l2 rem_addr = { 0 };
  int fd;
  char buf[sizeof("00:00:00:00:00:00")+1] = { 0 };
  socklen_t opt = sizeof(rem_addr);
  unsigned short psm;
  unsigned short cid;
  bdaddr_t src;

  int listen_fd = listen_channels.channels[listen_channel].fd;

  // accept one connection
  if((fd = accept(listen_fd, (struct sockaddr *) &rem_addr, &opt)) < 0)
  {
    perror("accept");
    return -1;
  }

  ba2str(&rem_addr.l2_bdaddr, buf);
  gprintf("accepted connection from %s (psm: 0x%04x)\n", buf, btohs(rem_addr.l2_psm));

  bacpy(&src, &rem_addr.l2_bdaddr);
  psm = btohs(rem_addr.l2_psm);
  cid = btohs(rem_addr.l2_cid);

  if(channels.nb == sizeof(channels.channels) / sizeof(*channels.channels))
  {
    fprintf(stderr, "no space left for the l2cap connection (in)\n");
    return -1;
  }

  uint16_t omtu = 0;

  if(l2cap_bluez_get_outgoing_mtu(fd, &omtu) < 0)
  {
    fprintf(stderr, "failed to get outgoing mtu\n");
    close(fd);
    return -1;
  }

  uint16_t handle = 0;

  if(l2cap_bluez_get_handle(fd, &handle) < 0)
  {
    fprintf(stderr, "failed to get handle\n");
    close(fd);
    return -1;
  }

  int devid = 0;

  if(l2cap_bluez_get_devid(&rem_addr.l2_bdaddr, &devid))
  {
    fprintf(stderr, "failed to get devid\n");
    close(fd);
    return -1;
  }

  int channel = channels.nb;

  //it's required to do this before the callback
  //as the callback may reenter
  channels.channels[channel].fd = fd;
  channels.channels[channel].devid = devid;
  channels.channels[channel].omtu = omtu;
  channels.channels[channel].handle = handle;
  channels.channels[channel].psm = psm;
  channels.channels[channel].cid = cid;
  ++channels.nb;

  if(listen_channels.channels[listen_channel].accept_callback(channel, &src))
  {
    //TODO MLA: close the channel
  }

  return 0;
}

static int l2cap_bluez_listen(int user, unsigned short psm, int options,
    L2CAP_ABS_LISTEN_ACCEPT_CALLBACK read_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
  struct sockaddr_l2 loc_addr = { 0 };
  int fd;

  if(listen_channels.nb == sizeof(listen_channels.channels) / sizeof(*listen_channels.channels))
  {
    fprintf(stderr, "no space left for listening psm 0x%04x\n", psm);
    return -1;
  }

  // allocate socket
  if((fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0)
  {
    perror("socket");
    return -1;
  }

  l2cap_bluez_setsockopt(fd, options);

  // bind socket to port psm of the first available
  // bluetooth adapter
  loc_addr.l2_family = AF_BLUETOOTH;
  loc_addr.l2_bdaddr = *BDADDR_ANY;
  loc_addr.l2_psm = htobs(psm);

  if(bind(fd, (struct sockaddr *) &loc_addr, sizeof(loc_addr)) < 0)
  {
    if(errno == EADDRINUSE)
    {
      fprintf(stderr, "bind: Address already device use\n");
      fprintf(stderr, "please stop the bluetooth service\n");
    }
    else
    {
      perror("bind");
    }
    close(fd);
    return -1;
  }

  // put socket into listening mode
  if(listen(fd, 10) < 0)
  {
    perror("listen");
    close(fd);
    return -1;
  }

  int channel = listen_channels.nb;

  listen_channels.channels[channel].fd = fd;
  listen_channels.channels[channel].accept_callback = read_callback;
  listen_channels.channels[channel].close_callback = close_callback;

  gpoll_register_fd(listen_channels.channels[channel].fd, channel, l2cap_bluez_connect_accept, NULL, l2cap_bluez_connect_accept);

  ++listen_channels.nb;

  gprintf("listening on psm: 0x%04x\n", psm);

  return channel;
}

static void l2cap_bluez_add_source(int channel, int user, L2CAP_ABS_READ_CALLBACK read_callback, L2CAP_ABS_PACKET_CALLBACK packet_callback, L2CAP_ABS_CLOSE_CALLBACK close_callback)
{
  channels.channels[channel].user = user;
  gpoll_register_fd(channels.channels[channel].fd, channels.channels[channel].user, read_callback, NULL, close_callback);
}

static int l2cap_bluez_disconnect(int channel)
{
  int result = 0;
  int dd;

  if ((dd = hci_open_dev(channels.channels[channel].devid)) < 0)
  {
    perror("hci_open_dev");
    return -1;
  }

  if(hci_disconnect(dd, channels.channels[channel].handle, HCI_OE_USER_ENDED_CONNECTION, 5*HCI_REQ_TIMEOUT) < 0)
  {
    perror("hci_disconnect");
    result = -1;
  }

  hci_close_dev(dd);

  return result;
}

static s_l2cap_abs l2cap_bluez =
{
    .connect = l2cap_bluez_connect,
    .listen = l2cap_bluez_listen,
    .send = l2cap_bluez_send,
    .recv = l2cap_bluez_recv,
    .close = l2cap_bluez_close,
    .add_source = l2cap_bluez_add_source,
    .disconnect = l2cap_bluez_disconnect,
};

void l2cap_bluez_init(void) __attribute__((constructor (101)));
void l2cap_bluez_init(void)
{
  l2cap_abs_register(E_BT_ABS_BLUEZ, &l2cap_bluez);
}
