/*
 * Derived from sixpair.c version 2007-04-18
 * Updated to work with libusb 1.0.
 *
 * Compile with: gcc -o sixaddr sixaddr.c -lusb-1.0
 *
 * Displays the bdaddr of the PS3 and the bdaddr of the sixaxis.
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

#define VENDOR 0x054c
#define PRODUCT 0x0268

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

static unsigned char msg_master[8];
static unsigned char msg_slave[18];

static const int itfnum = 0;

int get_bdaddrs(libusb_device_handle* devh)
{
  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x03f5, itfnum, msg_master, sizeof(msg_master), 5000);

  if (res < 0)
  {
    fprintf(stderr, "Control transfer failed: %s.\n", libusb_strerror(res));
    return res;
  }

  res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        LIBUSB_REQUEST_CLEAR_FEATURE, 0x3f2, itfnum, msg_slave, sizeof(msg_slave), 5000);

  if (res < 0)
  {
    fprintf(stderr, "Control transfer failed: %s.\n", libusb_strerror(res));
  }

  return res;
}

int process_device(libusb_device_handle* devh)
{
  if(get_bdaddrs(devh) < 0)
  {
    return -1;
  }

  printf("Current Bluetooth master: ");
  printf("%02X:%02X:%02X:%02X:%02X:%02X\n", msg_master[2], msg_master[3], msg_master[4], msg_master[5], msg_master[6], msg_master[7]);

  printf("Current Bluetooth Device Address: ");
  printf("%02X:%02X:%02X:%02X:%02X:%02X\n", msg_slave[4], msg_slave[5], msg_slave[6], msg_slave[7], msg_slave[8], msg_slave[9]);

  return 0;
}

int main(int argc, char *argv[])
{
  libusb_device** devs;
  libusb_device_handle* devh = NULL;
  libusb_context* ctx = NULL;
  int ret = -1;
  int i;

  if(libusb_init(&ctx))
  {
    fprintf(stderr, "Can't initialize libusb.\n");
    return -1;
  }

  //libusb_set_debug(ctx, 128);

  ssize_t cnt = libusb_get_device_list(ctx, &devs);

  if(cnt < 0)
  {
    fprintf(stderr, "Can't get USB device list.\n");
  }

  for(i=0; i<cnt; ++i)
  {
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[i], &desc);
    if(ret < 0)
    {
      continue;
    }

    if(desc.idVendor != VENDOR || desc.idProduct != PRODUCT)
    {
      continue;
    }

    ret = libusb_open(devs[i], &devh);
    if(ret < 0)
    {
      continue;
    }

#if defined(LIBUSB_API_VERSION) || defined(LIBUSBX_API_VERSION)
    libusb_set_auto_detach_kernel_driver(devh, 1);
#else
#ifndef WIN32
    ret = libusb_kernel_driver_active(devh, 0);
    if(ret > 0)
    {
      ret = libusb_detach_kernel_driver(devh, 0);
      if(ret < 0)
      {
        fprintf(stderr, "Can't detach kernel driver: %s.\n", libusb_strerror(ret));
        continue;
      }
    }
#endif
#endif
    ret = libusb_claim_interface(devh, 0);
    if(ret < 0)
    {
      fprintf(stderr, "Can't claim interface: %s.\n", libusb_strerror(ret));
      continue;
    }

    ret = process_device(devh);

    ret = libusb_release_interface(devh, 0);
    if(ret < 0)
    {
      fprintf(stderr, "Can't release interface: %s.\n", libusb_strerror(ret));
    }

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
#ifndef WIN32
    ret = libusb_attach_kernel_driver(devh, 0);
    if(ret < 0)
    {
      fprintf(stderr, "Can't attach kernel driver: %s.\n", libusb_strerror(ret));
    }
#endif
#endif

    libusb_close(devh);
    devh = NULL;
  }

  if(devh != NULL)
  {
    libusb_close(devh);
  }

  libusb_free_device_list(devs, 1);

  libusb_exit(ctx);

  return ret;

}

