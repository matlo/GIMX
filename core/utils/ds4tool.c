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

static char* bdaddr = NULL;

void fatal(char *msg)
{
  perror(msg);
  exit(1);
}

static void usage()
{
  fprintf(stderr, "Usage: ds4tool [-w bdaddr]\n");
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
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n", msg[15], msg[14], msg[13], msg[12],
      msg[11], msg[10]);

  printf("Current Bluetooth Device Address: ");
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n", msg[6], msg[5], msg[4], msg[3],
      msg[2], msg[1]);
}

void set_master(libusb_device_handle* devh, unsigned char mac[6])
{
  printf("Setting master bd_addr to %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],
      mac[1], mac[2], mac[3], mac[4], mac[5]);

  unsigned char msg[] =
  { 0x13, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], 0x56, 0xE8, 0x81,
      0x38, 0x08, 0x06, 0x51, 0x41, 0xC0, 0x7F, 0x12, 0xAA, 0xD9, 0x66, 0x3C,
      0xCE };

  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_SET_CONFIGURATION, 0x0313, 0x0000, msg, sizeof(msg), 5000);

  if (res < 0)
  {
    perror("USB_REQ_SET_CONFIGURATION");
  }
}

void process_device(libusb_device_handle* devh)
{
  unsigned char addr[6];

  show_bdaddrs(devh);

  if (bdaddr)
  {
    if (sscanf(bdaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6)
    {
      usage();
    }
    else
    {
      set_master(devh, addr);
    }
  }
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, ":w:")) != -1)
  {
    switch (opt)
    {
      case 'w':
        bdaddr = optarg;
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

