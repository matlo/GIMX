/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

/*
 * gcc -g -o usbtrace usbtrace.c -lusb
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>

/*
 * The baud rate in bps.
 */
#define BAUDRATE B500000 //0.5Mbps

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
  struct termios options;
  int fd;

  printf("connecting to %s\n", portname);

  if ((fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)
  {
    printf("can't connect to %s\n", portname);
  }
  else
  {
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    cfmakeraw(&options);
    if(tcsetattr(fd, TCSANOW, &options) < 0)
    {
      printf("can't set serial port options\n");
      close(fd);
      fd = -1;
    }
    else
    {
      printf("connected\n");
    }
    tcflush(fd, TCIFLUSH);
  }

  return fd;
}

int serial_recv(int fd, void* pdata, unsigned int size)
{
  int bread = 0;
  int res;

  fd_set readfds;

  //struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

  while(bread != size)
  {
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    //int status = select(fd+1, &readfds, NULL, NULL, &timeout);
    int status = select(fd+1, &readfds, NULL, NULL, NULL);
    if(status > 0)
    {
      if(FD_ISSET(fd, &readfds))
      {
        res = read(fd, pdata, size-bread);
        if(res > 0)
        {
          bread += res;
        }
      }
    }
    else if(status == EINTR)
    {
      continue;
    }
    else
    {
      break;
    }
  }

  return bread;
}

void serial_close(int fd)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  close(fd);
}

static volatile int bexit = 0;

void ex_program(int sig)
{
  bexit = 1;
}

static char* serial_port = NULL;

static void usage()
{
  fprintf(stderr, "Usage: usbtrace [-p serial_port]\n");
  exit(EXIT_FAILURE);
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, "p:")) != -1)
  {
    switch (opt)
    {
      case 'p':
        serial_port = optarg;
        break;
      default: /* '?' */
        usage();
        break;
    }
  }
}

typedef struct
{
  unsigned char bRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
} control_request_header;

int main(int argc, char *argv[])
{
  control_request_header header;
  unsigned char* p_data = (unsigned char*)&header;
  int ret;
  int i;
  int fd;

  read_args(argc, argv);

  if(!serial_port)
  {
    fprintf(stderr, "No serial port specified!\n");
    usage();
  }

  (void) signal(SIGINT, ex_program);

  fd = serial_connect(serial_port);

  if(fd < 0)
  {
    exit(-1);
  }

  while(!bexit)
  {
    control_request_header header;

    ret = serial_recv(fd, &header, sizeof(control_request_header));
    if(ret < 0)
    {
      fprintf(stderr, "serial_recv error\n");
      exit(-1);
    }
    if(ret != sizeof(control_request_header))
    {
      fprintf(stderr, "bad recv len: %d\n", ret);
      exit(-1);
    }

    printf("bRequest: 0x%02x\n", header.bRequest);
    printf("bRequestType: 0x%02x\n", header.bRequestType);
    printf("wIndex: 0x%04x\n", header.wIndex);
    printf("wLength: 0x%04x\n", header.wLength);
    printf("wValue: 0x%04x\n", header.wValue);
  }

  /*
   * tcdrain(fd) does not work, and there does not seem to be a better work-around.
   */
  usleep(500000);
  serial_close(fd);

  return 0;
}
