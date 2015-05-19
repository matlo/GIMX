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
#include <libusb-1.0/libusb.h>
#endif

#define VENDOR 0x054c
#define PRODUCT 0x05c4

#define TYPE_DS4 0
#define TYPE_TEENSY 1

#if !defined(LIBUSB_API_VERSION) && !defined(LIBUSBX_API_VERSION)
const char * LIBUSB_CALL libusb_strerror(enum libusb_error errcode)
{
  return libusb_error_name(errcode);
}
#endif

static unsigned char type = TYPE_DS4;

static char* master = NULL;
static char* link_key = NULL;
static char* slave = NULL;

static unsigned char msg_bdaddrs[0x0010];
static unsigned char msg_link_key[0x0010];

static void usage()
{
  fprintf(stderr, "Usage: ds4tool [-t] [-l lk] [-m master] [-s slave]\n");
  fprintf(stderr, "  -t: Teensy mode\n");
  fprintf(stderr, "  -l lk: the link key to set\n");
  fprintf(stderr, "  -m master: the master bdaddr to set\n");
  fprintf(stderr, "  -s slave: the slave bdaddr to set (only with -t)\n");
}

int get_bdaddrs(libusb_device_handle* devh)
{
  int res = libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x0312, 0x0000, msg_bdaddrs, sizeof(msg_bdaddrs), 5000);

  if (res < 0)
  {
    fprintf(stderr, "Control transfer failed: %s.\n", libusb_strerror(res));
  }

  return res;
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
    fprintf(stderr, "Control transfer failed: %s.\n", libusb_strerror(res));
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
    fprintf(stderr, "Control transfer failed: %s.\n", libusb_strerror(res));
  }
}

int get_link_key(libusb_device_handle* devh)
{
  return libusb_control_transfer(devh, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
      LIBUSB_REQUEST_CLEAR_FEATURE, 0x0313, 0x0000, msg_link_key, sizeof(msg_link_key), 5000);
}

int process_device(libusb_device_handle* devh)
{
  int i;
  unsigned char bdaddr[6];
  unsigned char lk[16] = {};
#ifdef WIN32
  unsigned int i_bdaddr[6];
  unsigned int i_lk[16] = {};
#endif

  if(get_bdaddrs(devh) < 0)
  {
    return -1;
  }

  if(get_link_key(devh) < 0)
  {
    if(type == TYPE_TEENSY)
    {
      return -1;
    }
  }
  else
  {
    if(type == TYPE_DS4)
    {
      return -1;
    }
  }

  printf("Current Bluetooth master: ");
  printf("%02X:%02X:%02X:%02X:%02X:%02X\n", msg_bdaddrs[15], msg_bdaddrs[14], msg_bdaddrs[13], msg_bdaddrs[12], msg_bdaddrs[11], msg_bdaddrs[10]);

  printf("Current Bluetooth Device Address: ");
  printf("%02X:%02X:%02X:%02X:%02X:%02X\n", msg_bdaddrs[6], msg_bdaddrs[5], msg_bdaddrs[4], msg_bdaddrs[3], msg_bdaddrs[2], msg_bdaddrs[1]);

  if(type == TYPE_TEENSY)
  {
    printf("Current link key: ");
    for(i=0; i<16; ++i)
    {
      printf("%02X", msg_link_key[i]);
    }
    printf("\n");
  }

  if (master)
  {
#ifdef WIN32
    if (sscanf(master, "%2x:%2x:%2x:%2x:%2x:%2x", i_bdaddr, i_bdaddr+1, i_bdaddr+2, i_bdaddr+3, i_bdaddr+4, i_bdaddr+5) != 6)
#else
    if (sscanf(master, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", bdaddr, bdaddr+1, bdaddr+2, bdaddr+3, bdaddr+4, bdaddr+5) != 6)
#endif
    {
      usage();
    }
#ifdef WIN32
    for(i=0; i<6; ++i)
    {
      bdaddr[i] = i_bdaddr[i];
    }
#endif
    if(link_key)
    {
#ifdef WIN32
      if (sscanf(link_key, "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
                  i_lk, i_lk+1, i_lk+2, i_lk+3, i_lk+4, i_lk+5, i_lk+6, i_lk+7, i_lk+8, i_lk+9, i_lk+10, i_lk+11, i_lk+12, i_lk+13, i_lk+14, i_lk+15) != 16)
#else
      if (sscanf(link_key, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                lk, lk+1, lk+2, lk+3, lk+4, lk+5, lk+6, lk+7, lk+8, lk+9, lk+10, lk+11, lk+12, lk+13, lk+14, lk+15) != 16)
#endif
      {
        usage();
        return -1;
      }
#ifdef WIN32
      for(i=0; i<16; ++i)
      {
        i_lk[i] = i_lk[i];
      }
#endif
    }
    printf("Setting master bdaddr to %s\n", master);
    printf("Setting link key to %s\n", link_key);
    set_master(devh, bdaddr, lk);
  }
  if (slave)
  {
#ifdef WIN32
    if (sscanf(master, "%2x:%2x:%2x:%2x:%2x:%2x", i_bdaddr, i_bdaddr+1, i_bdaddr+2, i_bdaddr+3, i_bdaddr+4, i_bdaddr+5) != 6)
#else
    if (sscanf(master, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", bdaddr, bdaddr+1, bdaddr+2, bdaddr+3, bdaddr+4, bdaddr+5) != 6)
#endif
    {
      usage();
      return -1;
    }
#ifdef WIN32
    for(i=0; i<6; ++i)
    {
      bdaddr[i] = i_bdaddr[i];
    }
#endif
    printf("Setting slave bdaddr to %s\n", slave);
    set_slave(devh, bdaddr);
  }
  return 0;
}

/*
 * Reads command-line arguments.
 */
static void read_args(int argc, char* argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, "l:m:s:t::")) != -1)
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
      case 't':
        type = TYPE_TEENSY;
        break;
      default: /* '?' */
        usage();
        exit(EXIT_FAILURE);
        break;
    }
  }

  if(type == TYPE_DS4 && slave)
  {
    usage();
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  libusb_device** devs;
  libusb_device_handle* devh = NULL;
  libusb_context* ctx = NULL;
  int ret = -1;
  int status;
  int i;

  read_args(argc, argv);

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

  for(i = 0; i < cnt; ++i)
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

    int config;

    ret = libusb_get_configuration(devh, &config);
    if(ret != LIBUSB_SUCCESS)
    {
      fprintf(stderr, "Can't get configuration: %s.\n", libusb_strerror(ret));
      continue;
    }

    if(config != 1)
    {
      ret = libusb_set_configuration(devh, 1);
      if(ret != LIBUSB_SUCCESS)
      {
        fprintf(stderr, "Can't set configuration: %s.\n", libusb_strerror(ret));
        continue;
      }
    }

    ret = libusb_claim_interface(devh, 0);
    if(ret < 0)
    {
      fprintf(stderr, "Can't claim interface: %s.\n", libusb_strerror(ret));
      continue;
    }

    status = process_device(devh);

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

    if(status == 0 && (slave || master))
    {
      break;
    }
  }
  
  if(devh != NULL)
  {
    libusb_close(devh);
  }

  libusb_free_device_list(devs, 1);

  libusb_exit(ctx);

  return ret;

}

