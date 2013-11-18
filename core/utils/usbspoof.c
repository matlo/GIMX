/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

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
#include <signal.h>
#else
#include <windows.h>
#include <mmsystem.h>
#define EINPROGRESS 115
#endif
#include <stdlib.h>

#include "pcapwriter.h"

#include <usb_spoof.h>

#include <errno.h>

#define STANDARD_REQUESTS 0

#define SPOOF_TIMEOUT 5

#define ITFNUM 2

static volatile int bexit = 0;

static int spoof = 0;
static int response = 0;
static int debug = 0;
static int verbose = 0;
static int libusb_debug = 0;

static char* serial_port = NULL;
static char* file_name = NULL;

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



static void usage()
{
  fprintf(stderr, "Usage: usbspoof [-v] [-d] [-p serial_port] [-w file_name]\n");
  exit(EXIT_FAILURE);
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[])
{
  int opt;

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
  int fd;

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

  fd = serial_connect(serial_port);

  if(fd < 0)
  {
    exit(-1);
  }

  ret = usb_spoof_get_adapter_status(fd);

  if(ret < 0)
  {
    exit(-1);
  }
  else if(ret > 0)
  {
    printf("already spoofed\n");
    printf("spoof successful\n");
    exit(0);
  }

  ret = usb_spoof_init_usb_device(X360_VENDOR, X360_PRODUCT, &usb_header.bus_id, &usb_header.device_address, libusb_debug);

  if(ret < 0)
  {
    exit(-1);
  }

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
    unsigned char packet_type;

    while(!bexit)
    {
      ret = serial_recv(fd, &packet_type, sizeof(packet_type));
      if(ret < 0)
      {
        fprintf(stderr, "serial_recv error\n");
        exit(-1);
      }
      else if(ret == sizeof(packet_type))
      {
        break;
      }
    }

    if(packet_type != BYTE_SPOOF_DATA)
    {
      fprintf(stderr, "bad packet type: %02x\n", packet_type);
      exit(-1);
    }

    unsigned char packet_len;

    ret = serial_recv(fd, &packet_len, sizeof(packet_len));
    if(ret != sizeof(packet_len))
    {
      fprintf(stderr, "serial_recv error\n");
      exit(-1);
    }

    ret = serial_recv(fd, p_data, packet_len);
    if(ret != packet_len)
    {
      fprintf(stderr, "serial_recv error\n");
      exit(-1);
    }

    if(bexit)
    {
      break;
    }

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

      /*if(!spoof)
      {
        continue;
      }*/

      if(!STANDARD_REQUESTS && !(creq.header.bRequestType & REQTYPE_VENDOR))
      {
        if(verbose || debug)
        {
          printf("--> standard requests are not forwarded\n");
          printf("\n");
        }
        continue;
      }

      if(response > 1)
      {
        continue;
      }

      ret = usb_spoof_forward_to_device(&creq);

      if(ret < 0)
      {
        fprintf(stderr, "usb_spoof_forward_to_device failed with error: %d\n", ret);
        exit(-1);
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
        if(ret > 0xff)
        {
          fprintf(stderr, "data length (%d) is higher than 255.", ret);
        }

        ret = usb_spoof_forward_to_adapter(fd, creq.data, ret & 0xFF);

        if(ret < 0)
        {
          exit(-1);
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

      if(creq.header.wValue == 0x5b17)
      {
        spoof = 1;
        printf("spoof started\n");
      }
      else if(creq.header.wValue == 0x5c10)
      {
        if(response)
        {
          printf("spoof successful\n");
          break;
        }
        ++response;
      }

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

      /*if(creq.header.wValue == 0x001e)
      {
        spoof = 1;
        printf("spoof successful\n");
        break;
      }*/

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
        continue;
      }*/

      /*if(!spoof)
      {
        continue;
      }*/

      if(!STANDARD_REQUESTS && !(creq.header.bRequestType & REQTYPE_VENDOR))
      {
        if(verbose || debug)
        {
          printf("--> standard requests are not forwarded\n");
          printf("\n");
        }
        continue;
      }

      if(response > 1)
      {
        continue;
      }

      /*
       * Forward the request to the 360 controller.
       * No need to forward anything back to the serial port.
       */
      ret = usb_spoof_forward_to_device(&creq);

      if(ret < 0)
      {
        fprintf(stderr, "usb_spoof_forward_to_device failed with error: %d\n", ret);
        exit(-1);
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

      if(verbose || debug)
      {
        printf("\n");
      }
    }
  }
  if(file_name)
  {
    pcapwriter_close();
  }
  printf("exiting\n");
#ifndef WIN32
  /*
   * tcdrain(fd) does not work, and there does not seem to be a better work-around.
   */
  usleep(500000);
  serial_close(fd);
#else
  timeEndPeriod(1000);
  CloseHandle(serial);
#endif
  usb_spoof_release_usb_device();
  if(bexit)
  {
    return -1;
  }
  return 0;
}
