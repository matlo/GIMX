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
#include <sys/time.h>

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

  printf("connecting to %s\n", portname);

  if ((fd = open(portname, O_RDONLY | O_NOCTTY)) < 0)
  {
    printf("can't connect to %s\n", portname);
  }
  else
  {
    tcgetattr(fd, &options);
    cfsetispeed(&options, TTY_BAUDRATE);
    cfsetospeed(&options, TTY_BAUDRATE);
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

void serial_close(int fd)
{
  close(fd);
}

typedef struct pcap_hdr_s {
  guint32 magic_number; /* magic number */
  guint16 version_major; /* major version number */
  guint16 version_minor; /* minor version number */
  gint32 thiszone; /* GMT to local correction */
  guint32 sigfigs; /* accuracy of timestamps */
  guint32 snaplen; /* max length of captured packets, in octets */
  guint32 network; /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
  guint32 ts_sec; /* timestamp seconds */
  guint32 ts_usec; /* timestamp microseconds */
  guint32 incl_len; /* number of octets of packet saved in file */
  guint32 orig_len; /* actual length of packet */
} pcaprec_hdr_t;

static pcap_hdr_t capture_header =
{
  .magic_number = 0xa1b2c3d4,
  .version_major = 0x0002,
  .version_minor = 0x0004,
  .thiszone = 0x00000000,
  .sigfigs = 0x00000000,
  .snaplen = 0x0000FFFF,
  .network = 0x000000C9, //DLT_BLUETOOTH_HCI_H4_WITH_PHDR
};

typedef struct _pcap_bluetooth_h4_header {
  guint32 direction; /* if first bit is set direction is incoming */
} pcap_bluetooth_h4_header;

static pcaprec_hdr_t packet_header = {};

static FILE* file = NULL;

void pcapwriter_init(char* file_name)
{
  file = fopen(file_name, "w");

  if(!file)
  {
    fprintf(stderr, "pcapwriter_init");
  }
  else
  {
    fwrite((char*)&capture_header, 1, sizeof(capture_header), file);
  }
}

void pcapwriter_close()
{
  if(file)
  {
    fclose(file);
  }
}

void pcapwriter_write(unsigned int direction, unsigned char packet_type, unsigned short data_length, unsigned char data[data_length])
{
  struct timeval tv;
  unsigned int length = 0;

  if(!file)
  {
    fprintf(stderr, "pcapwriter_write");
    return;
  }
  
  pcap_bluetooth_h4_header bt_h4_hdr =
  {
    .direction = dir
  };

  gettimeofday(&tv, NULL);

  packet_header.ts_sec = tv.tv_sec;
  packet_header.ts_usec = tv.tv_usec;

  packet_header.incl_len = sizeof(bt_h4_hdr)+1+data_length;
  packet_header.orig_len = sizeof(bt_h4_hdr)+1+data_length;

  fwrite((char*)&packet_header, 1, sizeof(packet_header), file);
  fwrite((char*)&bt_h4_hdr, 1, sizeof(bt_h4_hdr), file);
  fwrite((char*)&type, 1, sizeof(type), file);
  fwrite((char*)data, 1, data_length, file);
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

#define HCI_COMMAND_PKT         0x01
#define HCI_ACLDATA_PKT         0x02
#define HCI_SCODATA_PKT         0x03
#define HCI_EVENT_PKT           0x04
#define HCI_VENDOR_PKT          0x0E

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
  
  if(argc < 2)
  {
    printf("usage: ./sniffer <filename>");
  }
  
  pcapwriter_init(argv[1]);

  struct pollfd pfd[2] =
  {
      {.fd = fd1, .events = POLLIN},
      {.fd = fd2, .events = POLLIN},
  };

  unsigned char buf[sizeof(pfd)/sizeof(*pfd)][4096+4+2+2];

  int pos[sizeof(pfd)/sizeof(*pfd)] = {};
  unsigned int direction[sizeof(pfd)/sizeof(*pfd)] = {};
  int res;
  int i;
  int escape = 0;
  unsigned char type;
  unsigned short length;

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

                type = buf[2] >> 4;
                
                switch(type)
                {
                  case HCI_COMMAND_PKT:
                    direction[i] = 0;
                    break;
                  case HCI_EVENT_PKT:
                    direction[i] = 1;
                    break;
                }
                
                length = ((buf[2] & 0x0F) << 8) + buf[3];
                
                pcapwriter_write(direction[i], type, length, buf);

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
              switch(buf[i][pos[i]])
              {
                case SLIP_ESCAPE:
                  escape = 1;
                  break;
                case SLIP_ESCAPE_START_END:
                  if(escape)
                  {
                    pos[i]--;
                    buf[i][pos[i]] = SLIP_START_END;
                    escape = 0;
                  }
                  break;
                case SLIP_ESCAPE_ESCAPE:
                  if(escape)
                  {
                    pos[i]--;
                    buf[i][pos[i]] = SLIP_ESCAPE;
                    escape = 0;
                  }
                  break;
              }
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

  pcapwriter_close()

  serial_close(fd1);
  serial_close(fd2);

  return 0;
}
