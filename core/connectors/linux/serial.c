/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/serial.h>
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
#include <sys/uio.h>

#include <libintl.h>
#define _(STRING)    gettext(STRING)

#include <errno.h>

#include <adapter.h>

/*
 * The baud rate in bps.
 */
#define TTY_BAUDRATE B500000 //0.5Mbps
#define SPI_BAUDRATE 4000000 //4Mbps

static struct serial
{
  int fd;
  unsigned char data[HEADER_SIZE+BUFFER_SIZE];
  unsigned char bread;
} serials[MAX_CONTROLLERS] = {};

/*
 * This initializes the data before any other function of this file gets called.
 */
__attribute__((constructor (101))) static void struct_init(void)
{
  int i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    serials[i].fd = -1;
  }
}

/*
 * Connect to a serial port.
 */
int tty_connect(char* portname)
{
  struct termios options;
  int fd;

  printf(_("connecting to %s\n"), portname);

  if ((fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0)
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

int spi_connect(char* portname)
{
  int fd;

  unsigned int speed = SPI_BAUDRATE;
  unsigned char bits = 8;
  unsigned char mode = 0;

  if((fd = open(portname, O_RDWR | O_NONBLOCK)) < 0)
  {
    printf(_("can't connect to %s\n"), portname);
  }
  else if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
  {
    printf(_("can't set spi port speed\n"));
    close(fd);
    fd = -1;
  }
  else if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
  {
    printf(_("can't set bits per word written\n"));
    close(fd);
    fd = -1;
  }
  else if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
  {
    printf(_("can't set bits per word read\n"));
    close(fd);
    fd = -1;
  }
  else if (ioctl (fd, SPI_IOC_WR_MODE, &mode) < 0)
  {
    printf(_("can't set write mode\n"));
    close(fd);
    fd = -1;
  }
  else if (ioctl (fd, SPI_IOC_RD_MODE, &mode) < 0)
  {
    printf(_("can't set read mode\n"));
    close(fd);
    fd = -1;
  }

  return fd;
}

int serial_connect(int id, char* portname)
{
  int fd = 0;

  if(strstr(portname, "tty"))
  {
    fd = tty_connect(portname);
  }
  else if(strstr(portname, "spi"))
  {
    fd = spi_connect(portname);
  }
  else
  {
    fd = -1;
  }

  if(fd >= 0)
  {
    serials[id].fd = fd;
  }

  return fd;
}

/*
 * Send a data to the serial port.
 */
int serial_send(int id, void* pdata, unsigned int size)
{
  return write(serials[id].fd, pdata, size);
}

int serial_sendv(int id, void* pdata1, unsigned int size1, void* pdata2, unsigned int size2)
{
  struct iovec ov[2] =
  {
      {.iov_base = pdata1, .iov_len = size1},
      {.iov_base = pdata2, .iov_len = size2}
  };
  return writev(serials[id].fd, ov, sizeof(ov)/sizeof(*ov));
}

/*
 * This function tries to read 'size' bytes of data.
 * It blocks until 'size' bytes of data have been read or after 1s has elapsed.
 *
 * It should only be used in the initialization stages, i.e. before the mainloop.
 */
int serial_recv(int id, void* pdata, unsigned int size)
{
  int bread = 0;
  int res;

  fd_set readfds;

  struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

  while(bread != size)
  {
    FD_ZERO(&readfds);
    FD_SET(serials[id].fd, &readfds);
    int status = select(serials[id].fd+1, &readfds, NULL, NULL, &timeout);
    if(status > 0)
    {
      if(FD_ISSET(serials[id].fd, &readfds))
      {
        res = read(serials[id].fd, pdata, size-bread);
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

int serial_close(int fd)
{
  usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
  close(fd);
  return 0;
}

static int serial_callback(int id)
{
  int nread;
  int ret = 0;

  if(serials[id].bread < HEADER_SIZE)
  {
    /*
     * read the first two bytes first so as to retrieve the data length
     */
    nread = read(serials[id].fd, serials[id].data+serials[id].bread, HEADER_SIZE-serials[id].bread);

    if(nread >= 0)
    {
      serials[id].bread += nread;
    }
    else
    {
      perror("read");
      ret = -1;
    }
  }
  else
  {
    /*
     * read the data
     */
    nread = read(serials[id].fd, serials[id].data+serials[id].bread, serials[id].data[1]-(serials[id].bread-HEADER_SIZE));

    if(nread >= 0)
    {
      serials[id].bread += nread;

      if(serials[id].data[1] + HEADER_SIZE == serials[id].bread)
      {
        ret = adapter_process_packet(id, serials[id].data);

        serials[id].bread = 0;
      }
    }
    else
    {
      perror("read");
      ret = -1;
    }
  }

  return ret;
}

void serial_add_source(int id)
{
  GE_AddSource(serials[id].fd, id, serial_callback, NULL, serial_close);
}
