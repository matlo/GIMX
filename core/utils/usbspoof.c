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

#include "pcapwriter.h"

#include <errno.h>

#define VENDOR 0x045e
#define PRODUCT 0x028e

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

#define REQTYPE_VENDOR (2 << 5)

#define BUFFER_SIZE 4096
#define BAUDRATE B500000

#define ITFNUM 2

static int bexit = 0;

static int fd = -1;

static int spoof = 0;
static int debug = 0;
static int verbose = 0;

static libusb_device_handle *devh = NULL;
static libusb_context* ctx = NULL;

static char* serial_port = NULL;
static char* file_name = NULL;

typedef struct
{
  unsigned char bRequestType;
  unsigned char bRequest;
  unsigned short wValue;
  unsigned short wIndex;
  unsigned short wLength;
} control_request_header;

static pcap_usb_header usb_header =
{
  .id = 0,
  .event_type = URB_SUBMIT,
  .transfer_type = URB_CONTROL,
  .endpoint_number = 0x80,
  .device_address = 0x01,
  .bus_id = 0x0001,
  .setup_flag = 0x00,
  .data_flag = 0x00,
  .status = EINPROGRESS,
  .urb_len = 0x00000000,
  .data_len = 0x00000000,
};

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
  
  libusb_set_debug(ctx, 3);

  devh = libusb_open_device_with_vid_pid(ctx, VENDOR, PRODUCT);
  
  if(!devh)
  {
    fatal("libusb_open_device_with_vid_pid");
  }
  
  if(libusb_reset_device(devh))
  {
	  fatal("libusb_reset_device");
	}

  if(libusb_detach_kernel_driver(devh, ITFNUM) < 0)
  {
    //fatal("libusb_detach_kernel_driver");
  }
  int i;
  for(i=0; i<4; ++i)
  {
  if (libusb_claim_interface(devh, i) < 0)
  {
    //fatal("usb_claim_interface");
  }
  }

  usb_header.bus_id = libusb_get_bus_number(libusb_get_device(devh));
  usb_header.device_address = libusb_get_device_address(libusb_get_device(devh));
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
  fprintf(stderr, "Usage: usbspoof [-v] [-d] [-p serial_port] [-w file_name]\n");
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
  while ((opt = getopt(argc, argv, "vdp:w:")) != -1)
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
      case 'w':
        file_name = optarg;
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

  if(file_name)
  {
    pcapwriter_init(file_name);
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

          if(file_name)
          {
            usb_header.id++;
            usb_header.event_type = URB_SUBMIT;
            usb_header.endpoint_number = USB_DIR_IN;
            usb_header.status = -EINPROGRESS;
            usb_header.setup_flag = 0x00;
            usb_header.setup.bmRequestType = creq.header.bRequestType;
            usb_header.setup.bRequest = creq.header.bRequest;
            usb_header.setup.wValue = creq.header.wValue;
            usb_header.setup.wIndex = creq.header.wIndex;
            usb_header.setup.wLength = creq.header.wLength;
            pcapwriter_write(&usb_header, 0, NULL);
          }

          if(creq.header.wValue == 0x5b17)
          {
            spoof = 1;
            printf("spoof started\n");
          }

          /*if(!spoof)
          {
            p_data = (unsigned char*)&creq.header;
            continue;
          }*/

          if(!(creq.header.bRequestType & REQTYPE_VENDOR))
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

		      if(file_name)
          {
            usb_header.event_type = URB_COMPLETE;
            usb_header.endpoint_number = USB_DIR_IN;
            usb_header.setup_flag = 0x01;
            usb_header.setup.bmRequestType = 0x00;
            usb_header.setup.bRequest = 0x00;
            usb_header.setup.wValue = 0x00;
            usb_header.setup.wIndex = 0x00;
            usb_header.setup.wLength = 0x00;
            if(ret >= 0)
            {
              usb_header.status = URB_STATUS_SUCCESS;
              pcapwriter_write(&usb_header, ret, creq.data);
            }
            else
            {
              usb_header.status = ret;
              pcapwriter_write(&usb_header, 0, NULL);
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

          if(file_name)
          {
            usb_header.id++;
            usb_header.event_type = URB_SUBMIT;
            usb_header.endpoint_number = USB_DIR_OUT;
            usb_header.status = -EINPROGRESS;
            usb_header.setup_flag = 0x00;
            usb_header.setup.bmRequestType = creq.header.bRequestType;
            usb_header.setup.bRequest = creq.header.bRequest;
            usb_header.setup.wValue = creq.header.wValue;
            usb_header.setup.wIndex = creq.header.wIndex;
            usb_header.setup.wLength = creq.header.wLength;
            pcapwriter_write(&usb_header, ret, creq.data);
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

          /*if(
                 (creq.header.bRequestType == 0x00 && creq.header.bRequest == 0x05)
              || (creq.header.bRequestType == 0x02 && creq.header.bRequest == 0x01)
            )*/
          /*if(creq.header.wLength == 0)
          {
            p_data = (unsigned char*)&creq.header;
            continue;
          }*/

          /*if(!spoof)
          {
            p_data = (unsigned char*)&creq.header;
            continue;
          }*/

          if(!(creq.header.bRequestType & REQTYPE_VENDOR))
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
		      if(file_name)
          {
            usb_header.event_type = URB_COMPLETE;
            usb_header.endpoint_number = USB_DIR_OUT;
            usb_header.setup_flag = 0x01;
            if(ret < 0)
            {
              usb_header.status = ret;
            }
            else
            {
              usb_header.status = URB_COMPLETE;
            }
            usb_header.setup.bmRequestType = 0x00;
            usb_header.setup.bRequest = 0x00;
            usb_header.setup.wValue = 0x00;
            usb_header.setup.wIndex = 0x00;
            usb_header.setup.wLength = 0x00;
            pcapwriter_write(&usb_header, 0, NULL);
          }

          p_data = (unsigned char*)&creq.header;
        }
      }
    }
  }
  if(file_name)
  {
    pcapwriter_close();
  }
  close(fd);
  libusb_close(devh);
  libusb_exit(ctx);
  return 0;
}
