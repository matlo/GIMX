#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <libudev.h>
#include <linux/hidraw.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <errno.h>

//#define printf(...)  // comment this out for lots of info

struct udev_device*dev = NULL;
int fd = -1;

//  rawhid_recv - receive a packet
//    Inputs:
//  num = device to receive from (zero based)
//  buf = buffer to receive packet
//  len = buffer's size
//  timeout = time to wait, in milliseconds
//    Output:
//  number of bytes received, or -1 on error
//
int rawhid_recv(int num, void *buf, int len, int timeout)
{
  fd_set rfds;
  struct timeval tm;
  int ret;
  int i;
  if (fd < 0)
  {
    fprintf(stderr, "Device not opened\n");
    exit(-1);
  }
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  tm.tv_sec = timeout / 1000;
  tm.tv_usec = (timeout - tm.tv_sec * 1000) * 1000;
  ret = select(fd + 1, &rfds, NULL, NULL, &tm);
  if (ret > 0)
  {
    ret = read(fd, buf, len);
    /*for(i=0; i<ret; ++i)
     {
     printf("%02x", ((unsigned char*)buf)[i]);
     }
     printf("\n");*/
  }
  return ret;
}

//  rawhid_send - send a packet
//    Inputs:
//  num = device to transmit to (zero based)
//  buf = buffer containing packet to send
//  len = number of bytes to transmit
//  timeout = time to wait, in milliseconds
//    Output:
//  number of bytes sent, or -1 on error
//
int rawhid_send(int num, void *buf, int len, int timeout)
{
  fd_set wfds;
  struct timeval tm;
  int ret;
  if (fd < 0)
  {
    fprintf(stderr, "Device not opened\n");
    exit(-1);
  }
  FD_ZERO(&wfds);
  FD_SET(fd, &wfds);
  tm.tv_sec = timeout / 1000;
  tm.tv_usec = (timeout - tm.tv_sec * 1000) * 1000;
  //ret = select(fd+1, NULL, &wfds, NULL, &tm);
  //if(ret > 0)
  {
    ret = write(fd, buf, len);
    //printf("write: %d\n", ret);
  }
  return ret;
}

//  rawhid_open - open 1 or more devices
//
//    Inputs:
//  max = maximum number of devices to open
//  vid = Vendor ID, or -1 if any
//  pid = Product ID, or -1 if any
//  usage_page = top level usage page, or -1 if any
//  usage = top level usage number, or -1 if any
//    Output:
//  actual number of devices opened
//
int rawhid_open(int max, int vid, int pid, int usage_page, int usage)
{
  struct udev *udev;
  struct udev_enumerate *enumerate;
  struct udev_list_entry *devices, *dev_list_entry;
  struct udev_device *dev;
  char* cvid;
  char* cpid;
  int ivid;
  int ipid;
  char buf[256];
  int res;
  struct hidraw_devinfo info;
  int r;
  int len;
  struct stat devstat;

  /* Create the udev object */
  udev = udev_new();
  if (!udev)
  {
    fprintf(stderr, "Can't create udev\n");
    exit(1);
  }

  /* Create a list of the devices in the 'hidraw' subsystem. */
  enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "hidraw");
  udev_enumerate_scan_devices(enumerate);
  devices = udev_enumerate_get_list_entry(enumerate);
  /* For each item enumerated, print out its information.
   udev_list_entry_foreach is a macro which expands to
   a loop. The loop will be executed for each member in
   devices, setting dev_list_entry to a list entry
   which contains the device's path in /sys. */
  udev_list_entry_foreach(dev_list_entry, devices)
  {
    const char *path;

    /* Get the filename of the /sys entry for the device
     and create a udev_device object (dev) representing it */
    path = udev_list_entry_get_name(dev_list_entry);
    dev = udev_device_new_from_syspath(udev, path);

    /* The device pointed to by dev contains information about
     the hidraw device. In order to get information about the
     USB device, get the parent device with the
     subsystem/devtype pair of "usb"/"usb_device". This will
     be several levels up the tree, but the function will find
     it.*/
    dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    if (!dev)
    {
      fprintf(stderr, "Unable to find parent usb device.\n");
      exit(-1);
    }

    cvid = (char*)udev_device_get_sysattr_value(dev,"idVendor");
    cpid = (char*)udev_device_get_sysattr_value(dev, "idProduct");

    if(!cvid || !cpid)
    {
      udev_device_unref(dev);
      continue;
    }

    if(!sscanf(cvid, "%x", &ivid) || !sscanf(cpid, "%x", &ipid))
    {
      udev_device_unref(dev);
      continue;
    }

    if(vid != ivid || pid != ipid )
    {
      udev_device_unref(dev);
      continue;
    }

    /* usb_device_get_devnode() returns the path to the device node
     itself in /dev. */
    printf("Device Node Path: %s\n", udev_device_get_devpath(dev));

    /* From here, we can call get_sysattr_value() for each file
     in the device's /sys entry. The strings passed into these
     functions (idProduct, idVendor, serial, etc.) correspond
     directly to the files in the directory which represents
     the USB device. Note that USB strings are Unicode, UCS2
     encoded, but the strings returned from
     udev_device_get_sysattr_value() are UTF-8 encoded. */
    printf("  VID/PID: %s %s\n",
        udev_device_get_sysattr_value(dev,"idVendor"),
        udev_device_get_sysattr_value(dev, "idProduct"));
    printf("  %s\n  %s\n",
        udev_device_get_sysattr_value(dev,"manufacturer"),
        udev_device_get_sysattr_value(dev,"product"));
    printf("  serial: %s\n",
        udev_device_get_sysattr_value(dev, "serial"));

    r = stat("/dev/hidraw7", &devstat);
    if (r < 0)
    {
      perror("");
      exit(-1);
    }

    fd = open("/dev/hidraw7", O_RDWR|O_NONBLOCK);

    if (fd < 0)
    {
      perror("Unable to open device");
      exit(-1);
    }

    udev_device_unref(dev);

    break;
  }
  /* Free the enumerator object */
  udev_enumerate_unref(enumerate);

  udev_unref(udev);

  return 1;
}

//  rawhid_close - close a device
//
//    Inputs:
//  num = device to close (zero based)
//    Output
//  (nothing)
//
void rawhid_close(int num)
{
  if (fd >= 0)
  {
    close(fd);
  }
}
