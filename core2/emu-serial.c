/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#ifndef WIN32
#include <err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <termios.h>
#include <sys/resource.h>
#include <sched.h>
#else
#include <winsock2.h>
#endif
#include <sys/types.h>
#include "sixaxis.h"
#include "dump.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

static int debug = 0;

#define TCPPORT 21313 /* ('S'<<8+'A') */
#define DEFAULT_DEVICE "/dev/ttyUSB0"

#define BAUDRATE B500000

#define DEFAULT_MAX_AXIS_VALUE 65535
#define DEFAULT_MEAN_AXIS_VALUE DEFAULT_MAX_AXIS_VALUE/2

extern int sixaxis_number;

static int running = 1;

void sig_handler(int sig)
{
  running = 0;
}

typedef struct
{
  int16_t  X;
  int16_t  Y;
  int16_t  Z;
  int16_t  Rz;
  uint16_t Hat;
  uint16_t Bt;
} s_report_data;

extern const int digital_order[17];
extern int clamp(int min, int val, int max);

int send_report(int fd, struct sixaxis_state *state)
{
  s_report_data data = {};
  int i;
  struct timeval tv;
  uint8_t* pdata = (uint8_t*)&data;

  /*
   * Make sure the value is not out of range.
   */
  data.X = clamp(0, state->user.axis[0][0] + DEFAULT_MEAN_AXIS_VALUE, DEFAULT_MAX_AXIS_VALUE);
  data.Y = clamp(0, state->user.axis[0][1] + DEFAULT_MEAN_AXIS_VALUE, DEFAULT_MAX_AXIS_VALUE);
  data.Z = clamp(0, state->user.axis[1][0] + DEFAULT_MEAN_AXIS_VALUE, DEFAULT_MAX_AXIS_VALUE);
  data.Rz = clamp(0, state->user.axis[1][1] + DEFAULT_MEAN_AXIS_VALUE, DEFAULT_MAX_AXIS_VALUE);
  if(state->user.button[sb_square].pressed)
  {
    data.Bt |= 0x0001;
  }
  if(state->user.button[sb_cross].pressed)
  {
    data.Bt |= 0x0002;
  }
  if(state->user.button[sb_circle].pressed)
  {
    data.Bt |= 0x0004;
  }
  if(state->user.button[sb_triangle].pressed)
  {
    data.Bt |= 0x0008;
  }

  if(state->user.button[sb_select].pressed)
  {
    data.Bt |= 0x0100;
  }
  if(state->user.button[sb_start].pressed)
  {
    data.Bt |= 0x0200;
  }
  if(state->user.button[sb_l3].pressed)
  {
    data.Bt |= 0x0400;
  }
  if(state->user.button[sb_r3].pressed)
  {
    data.Bt |= 0x0800;
  }

  if(state->user.button[sb_l1].pressed)
  {
    data.Bt |= 0x0010;
  }
  if(state->user.button[sb_r1].pressed)
  {
    data.Bt |= 0x0020;
  }
  if(state->user.button[sb_l2].pressed)
  {
    data.Bt |= 0x0040;
  }
  if(state->user.button[sb_r2].pressed)
  {
    data.Bt |= 0x0080;
  }

  if(state->user.button[sb_right].pressed)
  {
    if(state->user.button[sb_down].pressed)
    {
      data.Hat = 0x0003;
    }
    else if(state->user.button[sb_up].pressed)
    {
      data.Hat = 0x0001;
    }
    else
    {
      data.Hat = 0x0002;
    }
  }
  else if(state->user.button[sb_left].pressed)
  {
    if(state->user.button[sb_down].pressed)
    {
      data.Hat = 0x0005;
    }
    else if(state->user.button[sb_up].pressed)
    {
      data.Hat = 0x0007;
    }
    else
    {
      data.Hat = 0x0006;
    }
  }
  else if(state->user.button[sb_down].pressed)
  {
    data.Hat = 0x0004;
  }
  else if(state->user.button[sb_up].pressed)
  {
    data.Hat = 0x0000;
  }
  else
  {
    data.Hat = 0x0008;
  }

  /* Dump contents */
  if (debug >= 2)
  {
    gettimeofday(&tv, NULL);
    printf("%ld.%06ld ", tv.tv_sec, tv.tv_usec);
    for (i = 0; i < sizeof(s_report_data); i++)
    {
      printf(" %02x", pdata[i]);
    }
    printf("\n");
  }

  return write(fd, pdata, sizeof(s_report_data));
}

int tcplisten(int port)
{
  struct sockaddr_in addr;
  int fd;
  int on = 0;

  if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    return -1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*) &on, sizeof(on)) < 0)
    return -1;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Listen only on localhost
  if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
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
  fd = accept(server, (struct sockaddr *) &addr, &addrlen);
  if (fd == -1)
    return -1;
  /* close server so nobody else can connect */
  close(server);
  return fd;
}

/* Handle data on TCP control port */
void handle_control(int tcpc, int rs232c, const unsigned char *buf, size_t len,
    struct sixaxis_state *state)
{
  int ret;

  /* Expect that we got 48 bytes, ignore anything else */
  if (len < sizeof(struct sixaxis_state))
  {
    printf("tcp control short packet %ld\n", (unsigned long) len);
    return;
  }

  if (len > sizeof(struct sixaxis_state))
  {
    printf("%d tcp packets merged\n", len / sizeof(struct sixaxis_state));
  }

  while (len >= sizeof(struct sixaxis_state))
  {
    state = (struct sixaxis_state *)buf;
    ret = send_report(rs232c, state);
    if (ret < 0)
    {
      printf("tcp control process error %d\n", ret);
      return;
    }
    buf += sizeof(struct sixaxis_state);
    len -= sizeof(struct sixaxis_state);
  }

  return;
}

int main(int argc, char *argv[])
{
  int i;
  char* device = NULL;
  int fdrs232 = -1;
  struct termios options;
  int fdmax;
  int tcps = -1, tcpc = -1;
  fd_set read_set;
  struct timeval timeout;
  unsigned char buf[1024];
  ssize_t len;
  struct sixaxis_state state;
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
  //setpriority(PRIO_PROCESS, getpid(), -20);

  setlinebuf(stdout);
#endif

  sixaxis_init(&state);

  /* Catch signals so we can do proper cleanup */
  signal(SIGINT, sig_handler);
#ifndef WIN32
  signal(SIGHUP, sig_handler);
#endif

  /* Check args */
  for(i=1; i<argc; ++i)
  {
    if(!strcmp(argv[i], "--device"))
    {
      device = argv[++i];
    }
    else if(!strcmp(argv[i], "--controller") && i<argc)
    {
      sixaxis_number = atoi(argv[++i]);
    }
    else if(!strcmp(argv[i], "--debug"))
    {
      debug = 2;
    }
  }

  if(!device)
  {
    device = DEFAULT_DEVICE;
    printf("using default device %s\n", DEFAULT_DEVICE);
  }

  if(sixaxis_number < 0)
  {
    sixaxis_number = 0;
    printf("using default sixaxis number 0\n");
  }

  /* Connect to the RS232 device */
  printf("connecting to %s\n", device);
  if ((fdrs232 = open(device, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
  {
    printf("can't connect to %s\n", device);//needed by sixemugui
    exit(-1);
  }

  tcgetattr(fdrs232, &options);
  cfsetispeed(&options, BAUDRATE);
  cfsetospeed(&options, BAUDRATE);
  options.c_cflag |= (CLOCAL | CREAD);
  tcsetattr(fdrs232, TCSANOW, &options);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  printf("connected\n");//needed by sixemugui

  /* Main loop */
  while (running)
  {
    /* Listen for TCP control connections */
    if (tcps < 0 && tcpc < 0)
    {
      if ((tcps = tcplisten(TCPPORT + sixaxis_number)) < 0)
      {
        printf("tcp listen\n");
      }
    }

    FD_ZERO(&read_set);
    //FD_SET(fdrs232, &read_set);
    FD_SET((tcpc >= 0) ? tcpc : tcps, &read_set);

    if(tcpc > tcps)
    {
      fdmax = tcpc;
    }
    else
    {
      fdmax = tcps;
    }

    timeout.tv_sec = 1;//wait up to 1s
    timeout.tv_usec = 0;

    /* select with timeout */
    if (select(fdmax+1, &read_set, NULL, NULL, &timeout) < 0)
    {
      fprintf(stderr, "select");
      break;
    }
    /* Read and handle data */
    if (FD_ISSET(fdrs232, &read_set))
    {
      //handle data from rs232
    }
    /* Read and handle tcp control connection */
    if(FD_ISSET(tcps, &read_set))
    {
      tcpc = tcpaccept(tcps);
      printf("client connected\n");
      if (tcpc > fdmax)
      {
        fdmax = tcpc;
      }
      if (tcpc < 0)
      {
        printf("tcp accept");
      }
      else
      {
        close(tcps);
        tcps = -1;
      }
    }
    if (FD_ISSET(tcpc, &read_set))
    {
      len = recv(tcpc, buf, 1024, recv_flags);
      if (len <= 0)
      {
        printf("client disconnected\n");
        if (len < 0)
        {
          printf("tcp recv");
        }
        close(tcpc);
        tcpc = -1;
      }
      else
      {
        handle_control(tcpc, fdrs232, buf, len, &state);
      }
    }
  }

  fprintf(stderr, "cleaning up\n");

  close(fdrs232);

  if (tcps > 0)
  {
    close(tcps);
  }
  if (tcpc > 0)
  {
    close(tcps);
  }

  return 0;
}
