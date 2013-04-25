/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef WIN32
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <unistd.h>
#include <emuclient.h>

#include <errno.h>
#include <stdlib.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define BT_SLOT 625 //microseconds


int l2cap_set_flush_timeout(bdaddr_t *ba, int timeout_ms)
{
    int err = 0, dd;
    struct hci_conn_info_req *cr = 0;
    struct hci_request rq = { 0 };

    struct {
        uint16_t handle;
        uint16_t flush_timeout;
    } cmd_param;

    struct {
        uint8_t  status;
        uint16_t handle;
    } cmd_response;

    // find the connection handle to the specified bluetooth device
    cr = (struct hci_conn_info_req*) malloc(
            sizeof(struct hci_conn_info_req) +
            sizeof(struct hci_conn_info));
    bacpy( &cr->bdaddr, ba );
    cr->type = ACL_LINK;
    dd = hci_open_dev( hci_get_route( &cr->bdaddr ) );
    if( dd < 0 ) {
        err = dd;
        goto cleanup;
    }
    err = ioctl(dd, HCIGETCONNINFO, (unsigned long) cr );
    if( err ) goto cleanup;

    // build a command packet to send to the bluetooth microcontroller
    cmd_param.handle = cr->conn_info->handle;
    cmd_param.flush_timeout = htobs(timeout_ms/BT_SLOT);
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = 0x28;
    rq.cparam = &cmd_param;
    rq.clen = sizeof(cmd_param);
    rq.rparam = &cmd_response;
    rq.rlen = sizeof(cmd_response);
    rq.event = EVT_CMD_COMPLETE;

    // send the command and wait for the response
    err = hci_send_req( dd, &rq, 0 );
    if( err ) goto cleanup;

    if( cmd_response.status ) {
        err = -1;
        errno = bt_error(cmd_response.status);
    }

cleanup:
    free(cr);
    if( dd >= 0) close(dd);
    return err;
}

int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, int psm)
{
    int fd;
    struct sockaddr_l2 addr;
    struct linger l = { .l_onoff = 1, .l_linger = 5 };
    int opt;

    if ((fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) == -1)
    {
      return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
      close(fd);
      return -2;
    }

    opt = 0;
    if (setsockopt(fd, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0) {
      close(fd);
      return -3;
    }

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    str2ba(bdaddr_src, &addr.l2_bdaddr);
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm    = htobs(psm);
    str2ba(bdaddr_dest, &addr.l2_bdaddr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        return -4;
    }

    int f = BT_FLUSHABLE_ON;
    if (setsockopt(fd, SOL_BLUETOOTH, BT_FLUSHABLE, &f, sizeof(f)) < 0)
    {
      close(fd);
      return -5;
    }

    return fd;
}

int l2cap_send(int fd, const unsigned char* buf, int len, int blocking)
{
    return send(fd, buf, len, blocking ? 0 : MSG_DONTWAIT);
}

int l2cap_recv(int fd, unsigned char* buf, int len)
{
    return recv(fd, buf, len, MSG_DONTWAIT);
}
#else
#include <string.h>
#include <ctype.h>

int l2cap_connect(const char *bdaddr, int psm)
{
    return 1;
}

int l2cap_send(int fd, const unsigned char* buf, int len, int blocking)
{
    return len;
}

int l2cap_recv(int fd, unsigned char* buf, int len)
{
    return 0;
}

int bachk(const char *str)
{
    char tmp[18], *ptr = tmp;

    if (!str)
        return -1;

    if (strlen(str) != 17)
        return -1;

    memcpy(tmp, str, 18);

    while (*ptr) {
        *ptr = toupper(*ptr);
        if (*ptr < '0' || (*ptr > '9' && *ptr < 'A') || *ptr > 'F')
            return -1;
        ptr++;

        *ptr = toupper(*ptr);
        if (*ptr < '0' || (*ptr > '9' && *ptr < 'A') || *ptr > 'F')
            return -1;
        ptr++;

        *ptr = toupper(*ptr);
        if (*ptr == 0)
            break;
        if (*ptr != ':')
            return -1;
        ptr++;
    }

    return 0;
}


#endif
