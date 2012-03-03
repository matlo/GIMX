/*
 * gcc -g -o usbspoof usbspoof.c -lusb
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#ifndef WIN32
#include <termios.h>
#include <signal.h>
#else
#include <windows.h>
#include <mmsystem.h>
#define EINPROGRESS 115
#endif
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#include "pcapwriter.h"

#include <errno.h>

#define SPOOF_TIMEOUT 15
#define USB_REQ_TIMEOUT 1000

#define VENDOR 0x045e
#define PRODUCT 0x028e

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

#define REQTYPE_VENDOR (2 << 5)

#define BUFFER_SIZE 4096

#define ITFNUM 2

static int bexit = 0;

#ifndef WIN32
#define BAUDRATE B500000
static int fd = -1;
#else
static int baudrate = 500000;
static HANDLE serial;
#endif

static int spoof = 0;
static int debug = 0;
static int verbose = 0;
static int libusb_debug = 0;

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

#ifndef WIN32
void catch_alarm (int sig)
{
  bexit = 1;
  printf("spoof ko!\n");
  signal (sig, catch_alarm);
}
#else
void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
  bexit = 1;
  printf("spoof ko!\n");
}
#endif

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
  
  libusb_set_debug(ctx, libusb_debug);

  devh = libusb_open_device_with_vid_pid(ctx, VENDOR, PRODUCT);
  
  if(!devh)
  {
    fatal("libusb_open_device_with_vid_pid");
  }
  
  if(libusb_reset_device(devh))
  {
	  fatal("libusb_reset_device");
	}
#ifndef WIN32
  if(libusb_detach_kernel_driver(devh, ITFNUM) < 0)
  {
    //fatal("libusb_detach_kernel_driver");
  }
#endif
  int i;
  for(i=0; i<4; ++i)
  {
    if (libusb_claim_interface(devh, i) < 0)
    {
      perror("usb_claim_interface");
    }
  }

  usb_header.bus_id = libusb_get_bus_number(libusb_get_device(devh));
  usb_header.device_address = libusb_get_device_address(libusb_get_device(devh));

#ifdef WIN32
  int config;
  if(libusb_get_configuration(devh, &config) < 0)
  {
    fatal("libusb_get_configuration");
  }
  if(config == 0)
  {
    printf("set configuration 1");
    if(libusb_set_configuration(devh, 1) < 0)
    {
      fatal("libusb_set_configuration");
    }
  }
#endif
}

#ifndef WIN32
/*
 * Opens ttyUSB0 for reading and writing.
 */
static void serial_port_init()
{
  struct termios options;

  fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY/* | O_NONBLOCK*/);

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
#else
static void serial_port_init()
{
  DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
  char scom[16];
  snprintf(scom, sizeof(scom), "\\\\.\\%s", serial_port);
  serial = CreateFile(scom, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
  if (serial == INVALID_HANDLE_VALUE)
  {
    printf("can't open serial port\n");
    exit(-1);
  }
  DCB dcbSerialParams =
  { 0 };
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  if (!GetCommState(serial, &dcbSerialParams))
  {
    printf("can't get serial port state\n");
    exit(-1);
  }
  dcbSerialParams.BaudRate = baudrate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  if (!SetCommState(serial, &dcbSerialParams))
  {
    printf("can't set serial port params\n");
    exit(-1);
  }
  COMMTIMEOUTS timeouts =
  { 0 };
  timeouts.ReadIntervalTimeout = 50;
  timeouts.ReadTotalTimeoutConstant = 50;
  timeouts.ReadTotalTimeoutMultiplier = 10;
  timeouts.WriteTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 10;
  if (!SetCommTimeouts(serial, &timeouts))
  {
    printf("can't set serial port timeouts\n");
    exit(-1);
  }
}

int serial_write(void* pdata, unsigned int size)
{
  DWORD dwBytesWrite = 0;

  if(WriteFile(serial, (uint8_t*)pdata, size, &dwBytesWrite, NULL))
  {
    return dwBytesWrite;
  }
  return 0;
}

int serial_read(void* pdata, unsigned int size)
{
  DWORD dwBytesWrite = 0;

  if(ReadFile(serial, (uint8_t*)pdata, size, &dwBytesWrite, NULL))
  {
    return dwBytesWrite;
  }
  return 0;
}
#endif

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
  while ((opt = getopt(argc, argv, "vdp:w:u:")) != -1)
  {
    switch (opt)
    {
      case 'u':
        libusb_debug = atoi(optarg);
        break;
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

int main(int argc, char *argv[])
{
  control_request creq;
  unsigned char* p_data = (unsigned char*)&creq.header;
  int ret;
  int i;
  int j;

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

#ifndef WIN32
  signal (SIGALRM, catch_alarm);
  alarm (SPOOF_TIMEOUT);
#else
  timeBeginPeriod(1000);
  MMRESULT timerID;
  timerID = timeSetEvent(1000*SPOOF_TIMEOUT, 0, TimeProc, 0, TIME_ONESHOT );
#endif

  while(!bexit)
  {
    /*
     * Get data from the serial port.
     */
#ifndef WIN32
    if(read(fd, p_data, sizeof(*p_data)) == sizeof(*p_data))
#else
    if(serial_read(p_data, sizeof(*p_data)) == sizeof(*p_data))
#endif
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
            printf("--> GET\n");
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
            if(verbose || debug)
            {
              printf("--> standard requests are not forwarded\n");
              printf("\n");
            }
            p_data = (unsigned char*)&creq.header;
            continue;
          }

          /*
           * Forward the request to the 360 controller.
           */
		      ret = libusb_control_transfer(devh, creq.header.bRequestType, creq.header.bRequest,
              creq.header.wValue, creq.header.wIndex, creq.data, creq.header.wLength, USB_REQ_TIMEOUT);

		      if(ret < 0)
          {
            printf("libusb_control_transfer failed with error: %d\n", ret);
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
#ifndef WIN32
            if(write(fd, &length, sizeof(length)) < sizeof(length) || write(fd, creq.data, ret) < ret)
#else
            if(serial_write(&length, sizeof(length)) < sizeof(length) || serial_write(creq.data, ret) < ret)
#endif
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

          if(verbose || debug)
          {
            printf("\n");
          }
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
            printf("--> SET\n");
            printf("bRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.header.bRequestType, creq.header.bRequest, creq.header.wValue, creq.header.wIndex, creq.header.wLength);
          }

          if(creq.header.wValue == 0x001e)
          {
            spoof = 1;
            printf("spoof successful\n");
            break;
          }

#ifndef WIN32
          ret = read(fd, p_data, creq.header.wLength);
#else
          ret = serial_read(p_data, creq.header.wLength);
#endif
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
            if(verbose || debug)
            {
              printf("--> standard requests are not forwarded\n");
              printf("\n");
            }
            p_data = (unsigned char*)&creq.header;
            continue;
          }

          /*
           * Forward the request to the 360 controller.
           * No need to forward anything back to the serial port.
           */
          ret = libusb_control_transfer(devh, creq.header.bRequestType, creq.header.bRequest,
              creq.header.wValue, creq.header.wIndex, creq.data, ret, USB_REQ_TIMEOUT);

		      if(ret < 0)
          {
            printf("libusb_control_transfer failed with error: %d\n", ret);
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

          if(verbose || debug)
          {
            printf("\n");
          }
        }
      }
    }
  }
  if(file_name)
  {
    pcapwriter_close();
  }
  printf("exiting\n");
#ifndef WIN32
  close(fd);
#else
  timeEndPeriod(1000);
  CloseHandle(serial);
#endif
  libusb_close(devh);
  libusb_exit(ctx);
  if(bexit)
  {
    return -1;
  }
  return 0;
}
