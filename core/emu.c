/*
 Copyright (c) 2010 Mathieu Laurendeau
 Copyright (c) 2009 Jim Paris <jim@jtan.com>
 License: GPLv3
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "l2cap_con.h"
#include <sys/time.h>
#include <signal.h>
#ifndef WIN32
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <bluetooth/bluetooth.h>
#include "bt_utils.h"
#include <sys/resource.h>
#include <sched.h>
#else
#include <winsock2.h>
#endif
#include <sys/types.h>
#include "sixaxis.h"
#include "dump.h"

#ifdef WIN32
#define SHUT_RDWR SD_BOTH

static void err(int eval, const char *fmt)
{
    fprintf(stderr, fmt);
    exit(eval);
}

void timersub(struct timeval *a, struct timeval *b, struct timeval *res)
{
    res->tv_sec = a->tv_sec - b->tv_sec;
    res->tv_usec = a->tv_usec - b->tv_usec;
    if (res->tv_usec < 0) {
      res->tv_sec--;
      res->tv_usec += 1000000;
    }
}

void timeradd(struct timeval *a, struct timeval *b, struct timeval *res)
{
    res->tv_sec = a->tv_sec + b->tv_sec;
    res->tv_usec = a->tv_usec + b->tv_usec;
    if (res->tv_usec > 1000000) {
      res->tv_sec++;
      res->tv_usec -= 1000000;
    }
}
#endif

static int debug = 0;

static const char *hid_report_name[] = { 
    "reserved", "input", "output", "feature" };

int device_number;

#define CTRL 17
#define DATA 19
#define TCPPORT 21313 /* ('S'<<8+'A') */

static int running = 1;
void sig_handler(int sig)
{
    running = 0;
}

int send_report(int fd, uint8_t type, uint8_t report,
        struct sixaxis_state *state, int blocking)
{
    uint8_t buf[128];
    int len = 0;
    int i;
    struct timeval tv;

    /* Assemble report */
    for (i = 0; sixaxis_assemble[i].func; i++) {
        if (sixaxis_assemble[i].type == type &&
            sixaxis_assemble[i].report == report) {
            len = sixaxis_assemble[i].
                func(&buf[2], sizeof(buf) - 2, state);
            break;
        }
    }

    if (!sixaxis_assemble[i].func || len < 0) {
        printf("%s %s report 0x%02x, sending empty response\n",
               (len < 0) ? "Error assembling" : "Unknown",
               hid_report_name[type], report);
        len = 0;
    }

    /* Fill common portion */
    buf[0] = 0xa0 | type;
    buf[1] = report;
    len += 2;

    /* Dump contents */
    if (debug >= 2) {
        gettimeofday(&tv, NULL);
        printf("%ld.%06ld Sixaxis %-7s %02x:",
               tv.tv_sec, tv.tv_usec, hid_report_name[type], report);
        for (i = 2; i < len; i++)
            printf(" %02x", buf[i]);
        printf("\n");
    }

    /* Send response.  Some messages (periodic input report) can be
       sent nonblocking, since they're not critical */
    return l2cap_send(fd, buf, len, blocking);
}

int process_report(uint8_t type, uint8_t report, const uint8_t *buf, int len,
           struct sixaxis_state *state)
{
    int i;
    int ret = 0;
    struct timeval tv;

    /* Dump contents */
    if (debug >= 2) {
        gettimeofday(&tv, NULL);
        printf("%ld.%06ld     PS3 %-7s %02x:",
               tv.tv_sec, tv.tv_usec, hid_report_name[type], report);
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
        printf("\n");
    }

    /* Process report */
    for (i = 0; sixaxis_process[i].func; i++) {
        if (sixaxis_process[i].type == type &&
            sixaxis_process[i].report == report) {
            ret = sixaxis_process[i].
                func(buf, len, state);
            break;
        }
    }

    if (!sixaxis_process[i].func || ret < 0) {
        printf("%s %s report 0x%02x\n",
               (ret < 0) ? "Error processing" : "Unknown",
               hid_report_name[type], report);
    }

    return ret;
}

int process(int psm, const unsigned char *buf, int len,
        int ctrl, int data, struct sixaxis_state *state)
{
    uint8_t transaction;
    uint16_t maxsize;
    uint8_t type;
    uint8_t report;
    const char *name;
    int ret = 0;
    struct timeval tv1, tv2;
    unsigned long time;

    if (len < 1)
        return -1;

    transaction = (buf[0] & 0xf0) >> 4;
    switch (transaction) {
    case HID_HANDSHAKE:
        if (buf[0] & 0x0f) {
            printf("handshake error: 0x%x\n", buf[0] & 0x0f);
            return -1;
        }
        break;

    case HID_GET_REPORT:
        if (buf[0] & 0x08) {
            if (len < 4) {
                printf("GET_REPORT short\n");
                return -1;
            }
            maxsize = (buf[3] << 8) | buf[2];
            if (maxsize < 64) {
                printf("GET_REPORT short buf (%d)\n", maxsize);
                return -1;
            }
        }
        type = buf[0] & 0x03;
        if (type == HID_TYPE_RESERVED) {
            printf("GET_REPORT bad type\n");
            return -1;
        }
        report = buf[1];
        /* printf("<- GET_REPORT %s 0x%02x\n", hid_report_name[type], report); */
        if (debug >= 2) {
            gettimeofday(&tv1, NULL);
        }
        ret = send_report(psm == CTRL ? ctrl : data, type, report, state, 1);
        if (debug >= 2) {
            gettimeofday(&tv2, NULL);
            time = (tv2.tv_sec*1000+tv2.tv_usec) - (tv1.tv_sec*1000+tv1.tv_usec);
            printf("blocking send took: %ld µs\n", time);
        }
        break;

    case HID_SET_REPORT:
    case HID_DATA:
        /* SET_REPORT and DATA are similar */
        name = (transaction == HID_DATA) ? "DATA" : "SET_REPORT";
        if (len < 2) {
            printf("%s: short\n", name);
            return -1;
        }
        type = buf[0] & 0x03;
        if (type == HID_TYPE_RESERVED) {
            printf("%s bad type\n", name);
            return -1;
        }
        report = buf[1];
        ret = process_report(type, report, buf + 2, len - 2, state);
        /* Respond to these on CTRL port with a positive HANDSHAKE */
        if (psm == CTRL) {
            char foo = (HID_HANDSHAKE << 4) | 0x0;
            if(write(ctrl, &foo, 1) < 1)
            {
              printf("write error");
            }
        }
        break;

    default:
        printf("unknown transaction %d\n", transaction);
        return -1;
    }

    return ret;
}

int tcplisten(int port)
{
    struct sockaddr_in addr;
    int fd;
    int on = 0;

    if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return -1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
        return -1;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Listen only on localhost
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        return -1;
    if (listen(fd, 1) == -1)
        return -1;
    return fd;
}

int tcpaccept(int server)
{
    struct sockaddr_in addr;
    size_t addrlen = sizeof(struct sockaddr_in);
    int fd;
    fd = accept(server, (struct sockaddr *)&addr, &addrlen);
    if (fd == -1)
        return -1;
    /* close server so nobody else can connect */
    close(server);
    return fd;
}

/* Handle data on TCP control port */
void handle_control(int tcpc, const unsigned char *buf, size_t len,
            struct sixaxis_state *state)
{
    int ret;

    /* Expect that we got 48 bytes, ignore anything else */
    if (len < 48) {
        printf("tcp control short packet %ld\n", (unsigned long)len);
        return;
    }

    if(len > 48)
    {
        printf("%d tcp packets merged\n", len/48);
    }

    while(len >= 48)
    {
        /* Process it as input report 01 */
        ret = process_report(HID_TYPE_INPUT, 0x01, buf, 48, state);
        if (ret < 0) {
            printf("tcp control process error %d\n", ret);
            return;
        }
        buf+=48;
        len-=48;
    }

    return;
}

int main(int argc, char *argv[])
{
    char *bdaddr_dest = NULL;
    int ctrl, data;
    int tcps = -1, tcpc = -1;
#ifndef WIN32
    struct pollfd pfd[3];
    struct timespec timeout;
#else
    fd_set read_set;
    struct timeval timeout;
#endif
    unsigned char buf[1024];
    ssize_t len;
    struct timeval next_report, now, diff;
    struct sixaxis_state state;
    int send_report_now = 0;
    struct timeval tv1, tv2;
#ifdef WIN32
    int recv_flags = 0;
#else
    int recv_flags = MSG_DONTWAIT;
#endif

#ifndef WIN32
    /*
     * Set highest priority & scheduler policy.
     */
    struct sched_param p = {.sched_priority = 99};

    sched_setscheduler(0, SCHED_FIFO, &p);
    
    //setpriority(PRIO_PROCESS, getpid(), -20); only useful with SCHED_OTHER

    setlinebuf(stdout);
#endif

    sixaxis_init(&state);

    /* Catch signals so we can do proper cleanup */
    signal(SIGINT, sig_handler);
#ifndef WIN32
    signal(SIGHUP, sig_handler);
#endif

    /* Check args */
    if (argc > 1)
        bdaddr_dest = argv[1];

    if (bachk(bdaddr_dest) == -1) {
        fprintf(stderr, "bad mac address\n");
        fprintf(stderr, "usage: %s <ps3-mac-address> <bt device number> <sixaxis number>\n", *argv);
        return 1;
    }
#ifndef WIN32
    if (argc > 2)
    {
        device_number = atoi(argv[2]);
    }
    else
    {
        device_number = 0;
        printf("default bt device number 0 is used\n");
    }

    if (argc > 3)
    {
        sixaxis_number = atoi(argv[3]);
    }
    else
    {
        sixaxis_number = 0;
        printf("default sixaxis number 0 is used\n");
    }

    if(device_number < 0 || get_device_bdaddr(device_number, bdaddr_src) < 0)
    {
        fprintf(stderr, "bad bt device number\n");
        fprintf(stderr, "usage: %s <ps3-mac-address>  <bt device number> <sixaxis number>\n", *argv);
        return 1;
    }

    if(geteuid())
    {
        printf("run as root user to set device class\n");
    }
    else if(write_device_class(device_number) < 0)
    {
        printf("failed to set device class\n");
    }

#endif
    /* Connect to PS3 */
    printf("connecting with hci%d = %s to %s psm %d\n", device_number, bdaddr_src, bdaddr_dest, CTRL);
    if ((ctrl = l2cap_connect(bdaddr_src, bdaddr_dest, CTRL)) < 0) {
        printf("can't connect to control psm\n");//needed by sixemugui
        err(1, "can't connect to control psm");
    }
    printf("connecting with hci%d = %s to %s psm %d\n", device_number, bdaddr_src, bdaddr_dest, DATA);
    if ((data = l2cap_connect(bdaddr_src, bdaddr_dest, DATA)) < 0) {
        shutdown(ctrl, SHUT_RDWR);
        close(ctrl);
        printf("can't connect to data psm\n");//needed by sixemugui
        err(1, "can't connect to data psm");
    }
    printf("connected\n");//needed by sixemugui

    /* First report can be sent now */
    gettimeofday(&next_report, NULL);

    /* Main loop */
    while (running) {
        /* If timeout has passed, send report. */
        gettimeofday(&now, NULL);
        if (timercmp(&now, &next_report, >=))
            send_report_now = 1;

        /* Listen for TCP control connections */
        if (tcps < 0 && tcpc < 0)
            if ((tcps = tcplisten(TCPPORT+sixaxis_number)) < 0)
                printf("tcp listen\n");

#ifndef WIN32
        memset(&pfd, 0, sizeof(pfd));

        /* Listen for data on either fd */
        pfd[0].fd = ctrl;
        pfd[0].events = POLLIN | POLLERR;
        pfd[1].fd = data;
        pfd[1].events = POLLIN | POLLERR;

        /* Listen to control client, or for new connection */
        pfd[2].fd = (tcpc >= 0) ? tcpc : tcps;
        pfd[2].events = POLLIN | POLLERR;

        /* Check data PSM for output, if it's time to send a report */
        if (send_report_now)
            pfd[1].events |= POLLOUT;
#else
        FD_ZERO(&read_set);
        FD_SET(ctrl, &read_set);
        FD_SET(data, &read_set);
#endif

        /* Compute timeout so it expires when next report is due */
        gettimeofday(&now, NULL);
        if (timercmp(&next_report, &now, <))
            timerclear(&diff);
        else
            timersub(&next_report, &now, &diff);

        /* Poll with timeout */
#ifndef WIN32
        timeout.tv_sec = diff.tv_sec;
        timeout.tv_nsec = diff.tv_usec * 1000;
        if (ppoll(pfd, 3, &timeout, NULL) < 0) {
            warn("ppoll");
            break;
        }
        if (pfd[0].revents & POLLERR) {
            warnx("error on ctrl psm");
            break;
        }
        if (pfd[1].revents & POLLERR) {
            warnx("error on data psm");
            break;
        }
        if (pfd[2].revents & POLLERR) {
            warnx("error on control connection");
            if (tcpc >= 0) {
                close(tcpc);
                tcpc = -1;
            }
        }
#else
        if (select(3, &read_set, NULL, NULL, &timeout) < 0) {
            fprintf(stderr, "select");
            break;
        }
#endif
        /* Read and handle data */
#ifndef WIN32
        if (pfd[0].revents & POLLIN)
#else
        if (FD_ISSET(ctrl, &read_set))
#endif
        {
            len = l2cap_recv(ctrl, buf, 1024);
            if (len > 0)
                if (process(CTRL, buf, len,
                        ctrl, data, &state) == -1) {
                    fprintf(stderr, "error processing ctrl");
                    break;
                }
        }
#ifndef WIN32
        if (pfd[1].revents & POLLIN)
#else
        if (FD_ISSET(data, &read_set))
#endif
        {
            len = l2cap_recv(data, buf, 1024);
            if (len > 0) {
                if (process(DATA, buf, len,
                        ctrl, data, &state) == -1) {
                    fprintf(stderr, "error processing data");
                    break;
                } else {
                    /* Respond to data report with a report of our own */
//                    send_report_now = 1;
                }
            }
        }

        /* Read and handle tcp control connection */
#ifndef WIN32
        if (pfd[2].revents & POLLIN)
#else
        if (FD_ISSET(tcpc, &read_set))
#endif
        {
            if (tcpc >= 0) {
                len = recv(tcpc, buf, 1024, recv_flags);
                if (len <= 0) {
                    printf("client disconnected\n");
                    if (len < 0)
                        printf("tcp recv");
                    close(tcpc);
                    tcpc = -1;
                } else {
                    handle_control(tcpc, buf, len, &state);
                    send_report_now = 1;
                }
            } else {
                tcpc = tcpaccept(tcps);
                printf("client connected\n");
                if (tcpc < 0)
                    printf("tcp accept");
                else {
                    close(tcps);
                    tcps = -1;
                }
            }
        }

        /* Send report */
        if (send_report_now) {
            /* If we can, send it now.
               Otherwise, if we can't send it, just skip to the next one */
            //if (pfd[1].revents & POLLOUT) {
                if (debug >= 1)
                    sixaxis_dump_state(&state, 0);
                if (sixaxis_periodic_report(&state)) {

                    if (debug >= 2) {
                        gettimeofday(&tv1, NULL);
                    }

                    if (send_report(data, HID_TYPE_INPUT,
                            0x01, &state, 0) == -1) {
                        printf("send_report");
                    }

                    /* Dump contents */
                    if (debug >= 2) {
                        gettimeofday(&tv2, NULL);
                        printf("non blocking send took: %ld µs\n", (tv2.tv_sec*1000+tv2.tv_usec) - (tv1.tv_sec*1000+tv1.tv_usec));
                    }
                }
            //}

            /* Schedule next report */
            send_report_now = 0;
            gettimeofday(&now, NULL);
            timeradd(&now, (&(struct timeval){0,1000000}), &next_report);
        }
    }

    fprintf(stderr, "cleaning up");
    shutdown(ctrl, SHUT_RDWR);
    shutdown(data, SHUT_RDWR);
    close(ctrl);
    close(data);
    if (tcps > 0)
        close(tcps);
    if (tcpc > 0)
        close(tcps);

    return 0;
}
