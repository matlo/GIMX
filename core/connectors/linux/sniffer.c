/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <poll.h>

#include <termios.h>

#include <sys/ioctl.h>

#include <libintl.h>
#define _(STRING)    gettext(STRING)

#include <errno.h>

#define PORT1 "/dev/ttyUSB0"
#define PORT2 "/dev/ttyUSB1"

/*
 * The baud rate in bps.
 */
#define TTY_BAUDRATE B3000000 //3Mbps

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
  struct termios options;
  int fd;

  printf(_("connecting to %s\n"), portname);

  if ((fd = open(portname, O_RDONLY | O_NOCTTY)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
  }
  else
  {
    tcgetattr(fd, &options);
    cfsetispeed(&options, TTY_BAUDRATE);
    cfsetospeed(&options, TTY_BAUDRATE);
    cfmakeraw(&options);
    if(tcsetattr(fd, TCSANOW, &options) < 0)
    {
      printf(_("can't set serial port options\n"));
      close(fd);
      fd = -1;
    }
    else
    {
      printf(_("connected\n"));
    }
    tcflush(fd, TCIFLUSH);
  }

  return fd;
}

void serial_close(int fd)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  close(fd);
}

static volatile int done = 0;

void terminate(int sig)
{
  done = 1;
}

#define SLIP_START_END        0xC0
#define SLIP_ESCAPE           0xDB
#define SLIP_ESCAPE_START_END 0xDC
#define SLIP_ESCAPE_ESCAPE    0xDD

int main(int argc, char* argv[])
{
  int fd1 = serial_connect(PORT1);
  if(fd1 < 0)
  {
    fprintf(stderr, "can't open %s\n", PORT1);
    exit(-1);
  }

  int fd2 = serial_connect(PORT2);
  if(fd1 < 0)
  {
    fprintf(stderr, "can't open %s\n", PORT2);
    exit(-1);
  }

  struct pollfd pfd[2] =
  {
      {.fd = fd1, .events = POLLIN},
      {.fd = fd2, .events = POLLIN},
  };

  unsigned char buf[sizeof(pfd)/sizeof(*pfd)][4096+4+2+2];

  int pos[sizeof(pfd)/sizeof(*pfd)] = {};
  int res;
  int i;

  while(!done)
  {
    if(poll(pfd, 2, -1) > 0)
    {
      for(i=0; i<sizeof(pfd)/sizeof(*pfd); ++i)
      {
        if(pfd[i].revents & POLLIN)
        {
          printf("got POLLIN event from fd=%d\n", pfd[i].fd);
          res = read(pfd[i].fd, buf[i]+pos[i], 1);
          if(res < 0)
          {
            if(errno == EINTR)
            {
              continue;
            }
            else
            {
              fprintf(stderr, "error reading from fd=%d\n", pfd[i].fd);
              done = 1;
            }
          }
          else if(res == 1)
          {
            if(buf[i][pos[i]] == SLIP_START_END)
            {
              if(pos[i] == 1)
              {
                printf("skipping end of SLIP packet from fd=%d\n", pfd[i].fd);
              }
              else if(pos[i] > 1)
              {
                printf("got end of SLIP packet from fd=%d\n", pfd[i].fd);
                printf("packet size: %d\n", pos[i]-1);

                /*
                 * TODO: decode and save packet
                 */

                pos[i] = 0;
              }
              else
              {
                printf("got start of SLIP packet from fd=%d\n", pfd[i].fd);
                pos[i]++;
              }
            }
            else if(pos[i] == 0)
            {
              printf("skipping data from fd=%d\n", pfd[i].fd);
            }
            else
            {
              printf("got data from fd=%d\n", pfd[i].fd);
              pos[i]++;
            }
          }
        }
        if(pfd[i].revents & POLLERR)
        {
          fprintf(stderr, "error reading from fd=%d\n", pfd[i].fd);
          done = 1;
        }
      }
    }
  }

  close(fd1);
  close(fd2);

  return 0;
}
