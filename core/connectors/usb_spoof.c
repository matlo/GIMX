/*
 Copyright (c) 2013 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */
 
#include <stdio.h>
#include <adapter.h>
#include <connectors/usb_spoof.h>
#include <gimx.h>
#include <connectors/serial.h>

#define USB_REQ_TIMEOUT 1000

static libusb_device_handle *devh = NULL;
static libusb_context* ctx = NULL;

/*
 * Opens a usb_dev_handle for the first usb device found.
 */
int usb_spoof_init_usb_device(int vendor, int product, uint16_t* bus_id, uint8_t* device_address, int libusb_debug)
{
  if(libusb_init(&ctx))
  {
    fprintf(stderr, "libusb_init\n");
    return -1;
  }

  libusb_set_debug(ctx, libusb_debug);

  devh = libusb_open_device_with_vid_pid(ctx, vendor, product);

  if(!devh)
  {
    fprintf(stderr, "libusb_open_device_with_vid_pid\n");
    return -1;
  }

  if(libusb_reset_device(devh))
  {
    fprintf(stderr, "libusb_reset_device\n");
    return -1;
  }

  if(bus_id) *bus_id = libusb_get_bus_number(libusb_get_device(devh));
  if(device_address) *device_address = libusb_get_device_address(libusb_get_device(devh));

  return 0;
}

void usb_spoof_release_usb_device()
{
  if(devh)
  {
    libusb_close(devh);
    libusb_exit(ctx);
  }
}

int usb_spoof_forward_to_device(control_request* creq)
{
  return libusb_control_transfer(devh, creq->setup.bmRequestType, creq->setup.bRequest,
      creq->setup.wValue, creq->setup.wIndex, creq->data, creq->setup.wLength, USB_REQ_TIMEOUT);
}

int usb_spoof_forward_to_adapter(int id, unsigned char* data, unsigned char length)
{
  unsigned char byte_spoof_data = BYTE_SPOOF_DATA;
  if(serial_send(id, &byte_spoof_data, sizeof(byte_spoof_data)) < sizeof(byte_spoof_data)
      || serial_send(id, &length, sizeof(length)) < sizeof(length)
      || serial_send(id, data, length) < length)
  {
    fprintf(stderr, "serial_send\n");
    return -1;
  }
  return length;
}

int usb_spoof_spoof_360_controller(int id)
{
  control_request creq;
  unsigned char* p_data = (unsigned char*)&creq.setup;
  int spoofed = 0;
  int response = 0;

  int ret = adapter_send_start(id);

  if(ret == 0)
  {
    ret = usb_spoof_init_usb_device(X360_VENDOR, X360_PRODUCT, NULL, NULL, 0);

    if(ret < 0)
    {
      ret = -1;
    }
    else
    {
      while(!spoofed)
      {
        unsigned char packet_type;

        ret = serial_recv(id, &packet_type, sizeof(packet_type));
        if(ret != sizeof(packet_type))
        {
          fprintf(stderr, "serial_recv error\n");
          ret = -1;
          break;
        }

        if(packet_type != BYTE_SPOOF_DATA)
        {
          fprintf(stderr, "bad packet type: %02x\n", packet_type);
          ret = -1;
          break;
        }

        unsigned char packet_len;

        ret = serial_recv(id, &packet_len, sizeof(packet_len));
        if(ret != sizeof(packet_len))
        {
          fprintf(stderr, "serial_recv error\n");
          ret = -1;
          break;
        }

        ret = serial_recv(id, p_data, packet_len);
        if(ret != packet_len)
        {
          fprintf(stderr, "serial_recv error\n");
          ret = -1;
          break;
        }

        if(creq.setup.bmRequestType & USB_DIR_IN)
        {
          /*
           * Forward the request to the 360 controller.
           */
          ret = usb_spoof_forward_to_device(&creq);

          if(ret < 0)
          {
            fprintf(stderr, "usb_spoof_forward_to_device failed with error: %d\n", ret);
            fprintf(stderr, "bmRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.setup.bmRequestType, creq.setup.bRequest, creq.setup.wValue, creq.setup.wIndex, creq.setup.wLength);
            ret = -1;
            break;
          }
          else if(ret > 0xFF)
          {
            fprintf(stderr, "usb_spoof_forward_to_device returned a size higher that 0xFF: %d\n", ret);
            fprintf(stderr, "bmRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.setup.bmRequestType, creq.setup.bRequest, creq.setup.wValue, creq.setup.wIndex, creq.setup.wLength);
            ret = -1;
            break;
          }
          else
          {
            /*
             * Forward data back to the adapter.
             */
            ret = usb_spoof_forward_to_adapter(id, creq.data, ret);

            if(ret < 0)
            {
              ret = -1;
              break;
            }
          }

          if(creq.setup.wValue == 0x5b17)
          {
            printf(_("spoof started\n"));
          }
          else if(creq.setup.wValue == 0x5c10)
          {
            if(response)
            {
              printf(_("spoof successful\n"));
              spoofed = 1;
              break;
            }
            ++response;
          }
        }
        else
        {
          /*
           * Forward the request to the 360 controller.
           * No need to forward anything back to the serial port.
           */
          ret = usb_spoof_forward_to_device(&creq);

          if(ret < 0)
          {
            fprintf(stderr, "usb_spoof_forward_to_device failed with error: %d\n", ret);
            fprintf(stderr, "bmRequestType: 0x%02x bRequest: 0x%02x wValue: 0x%04x wIndex: 0x%04x wLength: 0x%04x\n", creq.setup.bmRequestType, creq.setup.bRequest, creq.setup.wValue, creq.setup.wIndex, creq.setup.wLength);
            ret = -1;
            break;
          }
        }
      }

      usb_spoof_release_usb_device();
    }
  }
  else if(ret > 0)
  {
    printf(_("already spoofed\n"));
    ret = 0;
  }
  else
  {
    fprintf(stderr, _("can't read adapter status\n"));
    ret = -1;
  }

  return ret;
}
