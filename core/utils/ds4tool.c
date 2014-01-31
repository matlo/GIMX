/*
 * Compile with: gcc -o ds4tool ds4tool.c -lusb-1.0
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <libusb-1.0/libusb.h>
#else
#include <libusbx-1.0/libusb.h>
#endif

#define VENDOR 0x054c
#define PRODUCT 0x05c4

static char* master = NULL;
static char* link_key = NULL;
static char* slave = NULL;

void fatal(char *msg)
{
  perror(msg);
  exit(1);
}

static void usage()
{
  fprintf(stderr, "Usage: ds4tool [-l <link key> -m <master bdaddr> -s <slave bdaddr>]\n");
  exit(EXIT_FAILURE);
}

void show_bdaddrs(libusb_device_handle* devh)
{
  unsigned char msg[0x0010];

  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x0312, 0x0000, msg, sizeof(msg), 5000);

  if (res < 0)
  {
    perror("USB_REQ_GET_CONFIGURATION");
    return;
  }

  printf("Current Bluetooth master: ");
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n", msg[15], msg[14], msg[13], msg[12], msg[11], msg[10]);

  printf("Current Bluetooth Device Address: ");
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n", msg[6], msg[5], msg[4], msg[3], msg[2], msg[1]);
}

void set_master(libusb_device_handle* devh, unsigned char master[6], unsigned char lk[16])
{
  unsigned char msg[] =
  {
      0x13,
      master[5], master[4], master[3], master[2], master[1], master[0],
      lk[0], lk[1], lk[2], lk[3], lk[4], lk[5], lk[6], lk[7],
      lk[8], lk[9], lk[10], lk[11], lk[12], lk[13], lk[14], lk[15]
  };

  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_SET_CONFIGURATION, 0x0313, 0x0000, msg, sizeof(msg), 5000);

  if (res < 0)
  {
    perror("USB_REQ_SET_CONFIGURATION");
  }
}

void set_slave(libusb_device_handle* devh, unsigned char slave[6])
{
  unsigned char msg[] =
  {
      slave[5], slave[4], slave[3], slave[2], slave[1], slave[0],
  };

  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_SET_CONFIGURATION, 0x0312, 0x0000, msg, sizeof(msg), 5000);

  if (res < 0)
  {
    perror("USB_REQ_SET_CONFIGURATION");
  }
}

void show_link_key(libusb_device_handle* devh)
{
  unsigned char msg[0x0010];

  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x0313, 0x0000, msg, sizeof(msg), 5000);

  if (res < 0)
  {
    return;
  }

  printf("Current link key: ");
  int i;
  for(i=0; i<res; ++i)
  {
    printf("%02x", msg[i]);
  }
  printf("\n");
}

void process_device(libusb_device_handle* devh)
{
  unsigned char bdaddr[6];
  unsigned char lk[16] = {};

  show_bdaddrs(devh);

  show_link_key(devh);

  if (master)
  {
    if (sscanf(master, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", bdaddr, bdaddr+1, bdaddr+2, bdaddr+3, bdaddr+4, bdaddr+5) != 6)
    {
      usage();
    }
    if(link_key)
    {
      if (sscanf(link_key, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
          lk, lk+1, lk+2, lk+3, lk+4, lk+5, lk+6, lk+7, lk+8, lk+9, lk+10, lk+11, lk+12, lk+13, lk+14, lk+15) != 16)
      {
        usage();
      }
    }
    printf("Setting master bdaddr to %s\n", master);
    printf("Setting link key to %s\n", link_key);
    set_master(devh, bdaddr, lk);
  }
  if (slave)
  {
    if (sscanf(slave, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", bdaddr+0, bdaddr+1, bdaddr+2, bdaddr+3, bdaddr+4, bdaddr+5) != 6)
    {
      usage();
    }
    printf("Setting slave bdaddr to %s\n", slave);
    set_slave(devh, bdaddr);
  }
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, "l:m:s:")) != -1)
  {
    switch (opt)
    {
      case 'm':
        master = optarg;
        break;
      case 's':
        slave = optarg;
        break;
      case 'l':
        link_key = optarg;
        break;
      default: /* '?' */
        usage();
        break;
    }
  }
}

int main(int argc, char *argv[])
{
  static libusb_device_handle* devh = NULL;
  static libusb_context* ctx = NULL;

  read_args(argc, argv);

  if(libusb_init(&ctx))
  {
    fprintf(stderr, "Can't initialize libusb.\n");
    return -1;
  }

  //libusb_set_debug(ctx, 128);

  devh = libusb_open_device_with_vid_pid(ctx, VENDOR, PRODUCT);

  if(!devh)
  {
    fprintf(stderr, "Can't find any DS4 controller on USB busses.\n");
    return -1;
  }

  if(libusb_detach_kernel_driver(devh, 0) < 0)
  {
    fprintf(stderr, "Can't detach kernel driver.\n");
    return -1;
  }

  if(libusb_claim_interface(devh, 0) < 0)
  {
    fprintf(stderr, "Can't claim interface.\n");
    return -1;
  }

  process_device(devh);

  if(libusb_release_interface(devh, 0))
  {
    fprintf(stderr, "Can't release interface.\n");
  }

  if(libusb_attach_kernel_driver(devh, 0) < 0)
  {
    fprintf(stderr, "Can't attach kernel driver.\n");
  }

  libusb_close(devh);

  return 0;

}

