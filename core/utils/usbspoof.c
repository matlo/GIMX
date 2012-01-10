/*
 * gcc -g -o usbspoof usbspoof.c -lusb
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>

#define VENDOR 0x045e
#define PRODUCT 0x028e

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

#define REQTYPE_VENDOR (2 << 5)

#define BUFFER_SIZE 4096
#define BAUDRATE B500000

//#define ITFNUM 3

static int bexit = 0;

static int fd = -1;

static int spoof = 0;
static int debug = 0;
static int verbose = 0;

static libusb_device_handle *devh = NULL;
static libusb_context* ctx = NULL;

static char* serial_port = NULL;

typedef struct
{
  unsigned char bRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
} control_request_header;

typedef struct
{
  control_request_header header;
  unsigned char data[BUFFER_SIZE];
} control_request;

void ex_program(int sig)
{
  bexit = 1;
}

void fatal(char *msg)
{
  perror(msg);
  exit(1);
}

/*
 * Opens a usb_dev_handle for the first 360 controller found.
 */
static void usb_init_spoof()
{
  if(libusb_init(&ctx))
  {
    perror("libusb_init");
  }
  
  devh = libusb_open_device_with_vid_pid(ctx, VENDOR, PRODUCT);
  
  if(!devh)
  {
    fatal("libusb_open_device_with_vid_pid");
  }
  
  if(libusb_reset_device(devh))
  {
	fatal("libusb_reset_device");
  }
}

/*
 * Opens ttyUSB0 for reading and writing.
 */
static void serial_port_init()
{
  struct termios options;

  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY/* | O_NONBLOCK*/);

  if(fd < 0)
  {
    fatal("open");
  }

  tcgetattr(fd, &options);
  cfsetispeed(&options, BAUDRATE);
  cfsetospeed(&options, BAUDRATE);
  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_oflag &= ~OPOST;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_iflag &= ~(IXON | IXOFF | IXANY);
  if(tcsetattr(fd, TCSANOW, &options) < 0)
  {
    printf("can't set serial port options\n");
    exit(-1);
  }

  tcflush(fd, TCIFLUSH);
}

static void usage()
{
  fprintf(stderr, "Usage: usbspoof [-v] [-d] [-p serial_port]\n");
  exit(EXIT_FAILURE);
}

/*
 * Reads command-line arguments.
 */
static int read_args(int argc, char* argv[])
{
  int flags, opt;
  int nsecs, tfnd;

  nsecs = 0;
  tfnd = 0;
  flags = 0;
  while ((opt = getopt(argc, argv, "vdp:")) != -1)
  {
    switch (opt)
    {
      case 'v':
        verbose = 1;
        break;
      case 'd':
        debug = 1;
        verbose = 1;
        break;
      case 'p':
        serial_port = optarg;
        break;
      default: /* '?' */
        usage();
        break;
    }
  }
}

static void timeout()
{
  sleep(15);
  printf("spoof ko!\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
  control_request creq;
  unsigned char* p_data = (unsigned char*)&creq.header;
  int ret;
  int i;
  int j;
  pthread_t thread;
  pthread_attr_t thread_attr;

  read_args(argc, argv);

  if(!serial_port)
  {
    fprintf(stderr, "No serial port specified!\n");
    usage();
  }

  (void) signal(SIGINT, ex_program);

  usb_init_spoof();

  serial_port_init();

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&thread, &thread_attr, (void*) timeout, NULL);

  while(!bexit)
  {
    /*
     * Get data from the serial port.
     */
    if(read(fd, p_data, sizeof(*p_data)) == sizeof(*p_data))
    {
      if(debug)
      {
        printf(" 0x%02x ", *p_data);
      }
      p_data++;
      /*
       * Check if the header is complete.
       */
      if(p_data >= (unsigned char*)&creq.header + sizeof(creq.header))
      {
        /*
         * Buffer overflow protection.
         */
        if(creq.header.wLength > BUFFER_SIZE)
        {
          fatal("bad length");
        }

        /*
         * No more data to wait for.
         */
        if(creq.header.bRequestType & USB_DIR_IN)
        {
          if(debug)
          {
            printf("\n");
          }
          if(verbose)
          {
            printf("bRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.header.bRequestType, creq.header.bRequest, creq.header.wValue, creq.header.wIndex, creq.header.wLength);
          }

          if(creq.header.wValue == 0x5b17)
          {
            spoof = 1;
            printf("spoof started\n");
          }

          if(!spoof)
          {
            p_data = (unsigned char*)&creq.header;
            continue;
          }

          /*
           * Forward the request to the 360 controller.
           */
		      ret = libusb_control_transfer(devh, creq.header.bRequestType, creq.header.bRequest,
              creq.header.wValue, creq.header.wIndex, creq.data, creq.header.wLength, 1000);

          if(ret < 0)
          {
            perror("libusb_control_transfer");
          }
          else
          {
            if(debug)
            {
              printf("read from controller: %d data: {", ret);
              for (i = 0; i < ret; ++i)
              {
                printf("0x%02hhx,", creq.data[i]);
              }
              printf("}\n");
            }

            unsigned char length[2];
            length[0] = ret & 0xFF;
            length[1] = ret >> 8;

            /*
             * Forward the length and the data to the serial port.
             */
            if(write(fd, &length, sizeof(length)) < sizeof(length) || write(fd, creq.data, ret) < ret)
            {
              perror("write");
            }
          }

          p_data = (unsigned char*)&creq.header;
        }
        /*
         * Check if data has to be waited for.
         */
        else
        {
          if(debug)
          {
            printf("\n");
          }
          if(verbose)
          {
            printf("bRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.header.bRequestType, creq.header.bRequest, creq.header.wValue, creq.header.wIndex, creq.header.wLength);
          }

          if(creq.header.wValue == 0x001e)
          {
            spoof = 1;
            printf("spoof successful\n");
            exit(0);
          }

          ret = read(fd, p_data, creq.header.wLength);
          if(ret < creq.header.wLength)
          {
            printf("\nread error!! expected: %d received: %d\n\n", creq.header.wLength, ret);
          }
          if(ret)
          {
            if(debug)
            {
              for (i = 0; i < ret; ++i)
              {
                printf(" 0x%02x ", creq.data[i]);
              }
              printf("\n");
              printf(" data:");

              for(i=0; i<ret; ++i)
              {
                 printf(" 0x%02x", creq.data[i]);
              }
              printf("\n");
            }
          }

          if(!spoof)
          {
            p_data = (unsigned char*)&creq.header;
            continue;
          }

          /*
           * Forward the request to the 360 controller.
           * No need to forward anything back to the serial port.
           */
          ret = libusb_control_transfer(devh, creq.header.bRequestType, creq.header.bRequest,
              creq.header.wValue, creq.header.wIndex, creq.data, ret, 1000);

		      if(ret < 0)
          {
            perror("libusb_control_transfer");
          }

          p_data = (unsigned char*)&creq.header;
        }
      }
    }
  }
  close(fd);
  libusb_close(devh);
  libusb_exit(ctx);
  return 0;
}
