/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <serial.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "emuclient.h"

/*
 * The serial connection.
 */
static int serial = -1;

/*
 * The baud rate in bps.
 */
#define BAUDRATE B500000

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
  struct termios options;
  int ret = 0;

  gprintf(_("connecting to %s\n"), portname);

  if ((serial = open(portname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
    ret = -1;
  }
  else
  {
    tcgetattr(serial, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_oflag &= ~OPOST;
    if(tcsetattr(serial, TCSANOW, &options) < 0)
    {
      printf(_("can't set serial port options\n"));
      ret = -1;
    }
    else
    {
      gprintf(_("connected\n"));
    }
  }

  return ret;
}

/*
 * Send a usb report to the serial port.
 */
int serial_send(void* pdata, unsigned int size)
{
  return write(serial, (uint8_t*)pdata, size);
}

void serial_close()
{
  close(serial);
}
