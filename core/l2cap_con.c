
#ifndef WIN32
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <unistd.h>

int l2cap_connect(const char *bdaddr_src, const char *bdaddr_dest, int psm)
{
    int fd;
    struct sockaddr_l2 addr;
    struct linger l = { .l_onoff = 1, .l_linger = 5 };
    int opt;

    if ((fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) == -1)
        return -1;
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
