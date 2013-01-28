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
#include "emuclient.h"
#include <linux/spi/spidev.h>

#include <termios.h>

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

  gprintf(_("connecting to %s\n"), portname);

  if ((fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
    ret = -1;
  }
  else
  {
    tcgetattr(fd, &options);
    cfsetispeed(&options, TTY_BAUDRATE);
    cfsetospeed(&options, TTY_BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_oflag &= ~OPOST;
    if(tcsetattr(fd, TCSANOW, &options) < 0)
    {
      printf(_("can't set serial port options\n"));
      close(fd);
      ret = -1;
    }
    else
    {
      gprintf(_("connected\n"));
    }
  }

  return ret;
}

int spi_connect(char* portname)
{
  int ret = 0;

  unsigned int speed = SPI_BAUDRATE;

  if((fd = open(portname, O_RDWR | O_NDELAY)) < 0)
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
  return write(fd, (uint8_t*)pdata, size);
}

void serial_close()
{
  close(fd);
}
