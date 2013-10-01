/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serial.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#include <sys/select.h>

#include <termios.h>

#include <sys/ioctl.h>

#include <libintl.h>
#define _(STRING)    gettext(STRING)

#include <errno.h>

/*
 * The serial connection.
 */
static int fd = -1;

/*
 * The baud rate in bps.
 */
#define TTY_BAUDRATE B500000 //0.5Mbps
#define SPI_BAUDRATE 4000000 //4Mbps

/*
 * Connect to a serial port.
 */
int tty_connect(char* portname)
{
  struct termios options;
  int ret = 0;

  printf(_("connecting to %s\n"), portname);

  if ((fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
    ret = -1;
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
      ret = -1;
    }
    else
    {
      printf(_("connected\n"));
    }
  }

  tcflush(fd, TCIFLUSH);

  return ret;
}

int spi_connect(char* portname)
{
  int ret = 0;

  unsigned int speed = SPI_BAUDRATE;
  unsigned char bits = 8;
  unsigned char mode = 0;

  if((fd = open(portname, O_RDWR | O_NONBLOCK)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
    ret = -1;
  }
  else if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
  {
    printf(_("can't set spi port speed\n"));
    close(fd);
    ret = -1;
  }
  else if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
  {
    printf(_("can't set bits per word written\n"));
    close(fd);
    ret = -1;
  }
  else if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
  {
    printf(_("can't set bits per word read\n"));
    close(fd);
    ret = -1;
  }
  else if (ioctl (fd, SPI_IOC_WR_MODE, &mode) < 0)
  {
    printf(_("can't set write mode\n"));
    close(fd);
    ret = -1;
  }
  else if (ioctl (fd, SPI_IOC_RD_MODE, &mode) < 0)
  {
    printf(_("can't set read mode\n"));
    close(fd);
    ret = -1;
  }

  return ret;
}

int serial_connect(char* portname)
{
  int ret = 0;

  if(strstr(portname, "tty"))
  {
    if(tty_connect(portname) < 0)
    {
      ret = -1;
    }
  }
  else if(strstr(portname, "spi"))
  {
    if(spi_connect(portname) < 0)
    {
      ret = -1;
    }
  }
  else
  {
    ret = -1;
  }

  return ret;
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(void* pdata, unsigned int size)
{
  return write(fd, pdata, size);
}

int serial_recv(void* pdata, unsigned int size)
{
  int bread = 0;
  int res;

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);

  struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

  while(bread != size)
  {
    int status = select(fd+1, &readfds, NULL, NULL, &timeout);
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

void serial_close()
{
  close(fd);
}
