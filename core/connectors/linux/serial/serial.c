/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <connectors/serial.h>
#include <connectors/protocol.h>
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

#include <adapter.h>

/*
 * The baud rate in bps.
 */
#define TTY_BAUDRATE B500000 //0.5Mbps
#define SPI_BAUDRATE 4000000 //4Mbps

static struct
{
  int fd;
  s_packet packet;
  unsigned char bread;
} serials[MAX_CONTROLLERS] = {};

/*
 * \brief Initialize all file descriptors to -1.
 */
void serial_init()
{
  int i;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    serials[i].fd = -1;
  }
}

/*
 * \brief Open a tty port.
 *
 * \param portname  the serial port name, e.g. /dev/ttyUSB0 or /dev/ttyACM0
 *
 * \return a file descriptor or -1 in case of an error
 */
static int tty_open(char* portname)
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

/*
 * \brief Open a spi port.
 *
 * \param portname  the spi port name, e.g. /dev/spidev1.1
 *
 * \return a file descriptor or -1 in case of an error
 */
static int spi_open(char* portname)
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

/*
 * \brief Open a serial port. The serial port is registered for further operations.
 *
 * \param id        the instance id
 * \param portname  the serial port to open, e.g. /dev/ttyUSB0, /dev/ttyACM0, /dev/spidev1.1
 *
 * \return 0 in case of a success, -1 in case of an error
 */
int serial_open(int id, char* portname)
{
  int ret = 0;

  if(strstr(portname, "tty"))
  {
    serials[id].fd = tty_open(portname);
  }
  else if(strstr(portname, "spi"))
  {
    serials[id].fd = spi_open(portname);
  }

  if(serials[id].fd < 0)
  {
    ret = -1;
  }

  return ret;
}

/*
 * \brief Send data to the serial port.
 *
 * \param id     the serial port instance
 * \param pdata  a pointer to the data to send
 * \param size   the size in bytes of the data to send
 *
 * \return the number of bytes actually written, or -1 in case of an error
 */
int serial_send(int id, void* pdata, unsigned int size)
{
  return write(serials[id].fd, pdata, size);
}

/*
 * \brief This function tries to read 'size' bytes of data.
 *
 * It blocks until 'size' bytes of data have been read or after 1s has elapsed.
 * It should only be used in the initialization stages, i.e. before the mainloop.
 *
 * \param id     the instance id
 * \param pdata  the pointer where to store the data
 * \param size   the number of bytes to retrieve
 *
 * \return the number of bytes actually read
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

/*
 * \brief This function closes a serial port.
 *
 * \param id  the instance id
 *
 * \return 0
 */
int serial_close(int id)
{
  if(serials[id].fd >= 0)
  {
    usleep(10000);//sleep 10ms to leave enough time for the last packet to be sent
    close(serials[id].fd);
    serials[id].fd = -1;
  }
  return 0;
}

/*
 * \brief the serial callback for serial ports that are added as event sources.
 *
 * \param id  the instance id
 *
 * \return 0 in case of a success, -1 in case of an error
 */
static int serial_callback(int id)
{
  int nread;
  int ret = 0;
  
  unsigned char* base = (unsigned char*)&serials[id].packet;

  if(serials[id].bread < HEADER_SIZE)
  {
    /*
     * read the first two bytes first so as to retrieve the data length
     */
    nread = read(serials[id].fd, base+serials[id].bread, HEADER_SIZE-serials[id].bread);

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
    nread = read(serials[id].fd, base+serials[id].bread, serials[id].packet.header.length-(serials[id].bread-HEADER_SIZE));

    if(nread >= 0)
    {
      serials[id].bread += nread;

      if(serials[id].packet.header.length + HEADER_SIZE == serials[id].bread)
      {
        ret = adapter_process_packet(id, &serials[id].packet);

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

/*
 * \brief Add a serial port as an event source.
 *
 * \param id  the instance id
 */
void serial_add_source(int id)
{
  GE_AddSource(serials[id].fd, id, serial_callback, NULL, serial_close);
}
