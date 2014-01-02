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
#include <signal.h>

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

#define guint32 unsigned int
#define guint16 unsigned short
#define gint32 signed int

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

void pcapwriter_write(struct timeval* tv, unsigned int direction, unsigned short data_length, unsigned char data[data_length])
{
  if(!file)
  {
    fprintf(stderr, "pcapwriter_write\n");
    return;
  }
  
  pcap_bluetooth_h4_header bt_h4_hdr =
  {
    .direction = direction
  };

  packet_header.ts_sec = tv->tv_sec;
  packet_header.ts_usec = tv->tv_usec;

  packet_header.incl_len = sizeof(bt_h4_hdr)+data_length;
  packet_header.orig_len = sizeof(bt_h4_hdr)+data_length;

  fwrite((char*)&packet_header, 1, sizeof(packet_header), file);
  fwrite((char*)&bt_h4_hdr, 1, sizeof(bt_h4_hdr), file);
  fwrite((char*)data, 1, data_length, file);
}

static volatile int done = 0;

void terminate(int sig)
{
  done = 1;
}

#define HCI_COMMAND_PKT         0x01
#define HCI_ACLDATA_PKT         0x02
#define HCI_SCODATA_PKT         0x03
#define HCI_EVENT_PKT           0x04
#define HCI_VENDOR_PKT          0xff

#define BUFFER_SIZE 4096

int main(int argc, char* argv[])
{

  (void) signal(SIGINT, terminate);

  int fd1 = serial_connect(PORT1);
  if(fd1 < 0)
  {
    exit(-1);
  }

  int fd2 = serial_connect(PORT2);
  if(fd1 < 0)
  {
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

  unsigned char buf[sizeof(pfd)/sizeof(*pfd)][BUFFER_SIZE];

  int pos[sizeof(pfd)/sizeof(*pfd)] = {};
  unsigned int direction[sizeof(pfd)/sizeof(*pfd)] = {};
  struct timeval tv[sizeof(pfd)/sizeof(*pfd)] = {};
  int res;
  int i;
  unsigned char type[sizeof(pfd)/sizeof(*pfd)] = {};
  unsigned short length[sizeof(pfd)/sizeof(*pfd)] = {};

  while(!done)
  {
    if(poll(pfd, 2, -1) > 0)
    {
      for(i=0; i<sizeof(pfd)/sizeof(*pfd); ++i)
      {
        if(pfd[i].revents & POLLIN)
        {
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
            if(pos[i] == 0)
            {
              gettimeofday(&tv[i], NULL);
              
              type[i] = buf[i][0];

              switch(type[i])
              {
                case 0x00:
                  printf("skipping null byte\n");
                  continue;
                  break;
                case HCI_COMMAND_PKT:
                  direction[i] = 0;
                  break;
                case HCI_EVENT_PKT:
                  direction[i] = 1;
                  break;
              }
            }

            switch(type[i])
            {
              case HCI_COMMAND_PKT:
                if(pos[i] == 3)
                {
                  length[i] = buf[i][3]+4;
                }
                break;
              case HCI_ACLDATA_PKT:
                if(pos[i] == 6)
                {
                  length[i] = buf[i][5]+(buf[i][6] << 8)+7;
                  if(length[i] > BUFFER_SIZE)
                  {
                    fprintf(stderr, "length is higher than %d: %d\n", BUFFER_SIZE, length[i]);
                    done = 1;
                  }
                }
                break;
              case HCI_EVENT_PKT:
                if(pos[i] == 2)
                {
                  length[i] = buf[i][2]+3;
                }
                break;
              case HCI_VENDOR_PKT:
                if(pos[i] == 2)
                {
                  length[i] = buf[i][2]+3;
                }
                break;
              default:
                printf("unknown packet type: 0x%02x\n", type[i]);
                done = 1;
                break;
            }

            pos[i]++;

            if(length[i] == pos[i])
            {
              printf("packet type: %d\n", type[i]);
              printf("packet length: %d\n", length[i]);
              
              int j;
              for(j=0; j<length[i]; ++j)
              {
                if(!(j%8))
                {
                  printf("\n");
                }
                printf("0x%02x ", buf[i][j]);
              }
              printf("\n");

              pcapwriter_write(tv+i, direction[i], length[i], buf[i]);

              pos[i] = 0;
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

  pcapwriter_close();

  serial_close(fd1);
  serial_close(fd2);

  return 0;
}
