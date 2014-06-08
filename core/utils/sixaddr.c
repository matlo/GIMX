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

#ifndef WIN32
#include <libusb-1.0/libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#define VENDOR 0x054c
#define PRODUCT 0x0268

static unsigned char msg_master[8];
static unsigned char msg_slave[18];

static const int itfnum = 0;

int get_bdaddrs(libusb_device_handle* devh)
{
  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x03f5, itfnum, msg_master, sizeof(msg_master), 5000);

  if (res < 0)
  {
    perror("USB_REQ_GET_CONFIGURATION");
    return res;
  }

  res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        LIBUSB_REQUEST_CLEAR_FEATURE, 0x3f2, itfnum, msg_slave, sizeof(msg_slave), 5000);

  if (res < 0)
  {
    perror("USB_REQ_GET_CONFIGURATION");
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
    return -1;
  }

  for(i=0; i<cnt; ++i)
  {
    struct libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(devs[i], &desc);
    if(!ret)
    {
      if(desc.idVendor == VENDOR && desc.idProduct == PRODUCT)
      {
        ret = libusb_open(devs[i], &devh);
        if(!ret)
        {
          if(libusb_detach_kernel_driver(devh, 0) < 0)
          {
            fprintf(stderr, "Can't detach kernel driver.\n");
            ret = -1;
          }
          else
          {
            if(libusb_claim_interface(devh, 0) < itfnum)
            {
              fprintf(stderr, "Can't claim interface.\n");
              ret = -1;
            }
            else
            {
              ret = process_device(devh);

              if(libusb_release_interface(devh, itfnum))
              {
                fprintf(stderr, "Can't release interface.\n");
              }
            }

            if(libusb_attach_kernel_driver(devh, 0) < 0)
            {
              fprintf(stderr, "Can't attach kernel driver.\n");
            }
          }

          libusb_close(devh);
        }
      }
    }
  }

  libusb_free_device_list(devs, 1);

  libusb_exit(ctx);

  return ret;

}

