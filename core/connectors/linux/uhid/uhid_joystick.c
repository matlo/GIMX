/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#include <uhid_joystick.h>

#include <uhidasync.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uhid.h>
#include <dirent.h>
#include <string.h>

#define UHID_JOYSTICK_MAX_DEVICES 256

#define PRINT_ERROR_ALLOC_FAILED(func) fprintf(stderr, "%s:%d %s: %s failed\n", __FILE__, __LINE__, __func__, func);

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

#define PRINT_ERROR_ERRNO(msg) fprintf(stderr, "%s:%d %s: %s failed with error: %m\n", __FILE__, __LINE__, __func__, msg);

static struct {
    int hid;
    int uhid;

} uhid_joystick_devices[UHID_JOYSTICK_MAX_DEVICES] = { };

void uhid_joystick_init(void) __attribute__((constructor (101)));
void uhid_joystick_init(void) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        uhid_joystick_devices[i].hid = -1;
        uhid_joystick_devices[i].uhid = -1;
    }
}

static void uhid_joystick_close(int id) {

    if (uhid_joystick_devices[id].hid >= 0) {
        hidasync_close(uhid_joystick_devices[id].hid);
        uhid_joystick_devices[id].hid = -1;
    }
    if (uhid_joystick_devices[id].uhid >= 0) {
        uhidasync_close(uhid_joystick_devices[id].uhid);
        uhid_joystick_devices[id].uhid = -1;
    }
}

void uhid_joystick_clean(void) __attribute__((destructor (101)));
void uhid_joystick_clean(void) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        uhid_joystick_close(i);
    }
}

inline int uhid_joystick_check_device(int device, const char * file, unsigned int line, const char * func) {
    if (device < 0 || device >= UHID_JOYSTICK_MAX_DEVICES) {
        PRINT_ERROR_OTHER("invalid device")
        return -1;
    }
    if (uhid_joystick_devices[device].hid < 0) {
        PRINT_ERROR_OTHER("no such device")
        return -1;
    }
    return 0;
}
#define UHID_JOYSTICK_CHECK_DEVICE(device,retValue) \
  if(uhid_joystick_check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static int add_device(int hid, int uhid) {
    int i;
    for (i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if (uhid_joystick_devices[i].hid == -1
            && uhid_joystick_devices[i].uhid == -1) {
            uhid_joystick_devices[i].hid = hid;
            uhid_joystick_devices[i].uhid = uhid;
            return i;
        }
    }
    return -1;
}

static unsigned short lg_wheel_products[] = {
    USB_DEVICE_ID_LOGITECH_WINGMAN_FFG,
    USB_DEVICE_ID_LOGITECH_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFP_WHEEL,
    USB_DEVICE_ID_LOGITECH_G25_WHEEL,
    USB_DEVICE_ID_LOGITECH_DFGT_WHEEL,
    USB_DEVICE_ID_LOGITECH_G27_WHEEL,
    USB_DEVICE_ID_LOGITECH_WII_WHEEL,
    USB_DEVICE_ID_LOGITECH_MOMO_WHEEL2,
    USB_DEVICE_ID_LOGITECH_VIBRATION_WHEEL,
};

static int is_logitech_wheel(unsigned short vendor, unsigned short product) {

    if(vendor != USB_VENDOR_ID_LOGITECH) {
        return 0;
    }
    int i;
    for(i = 0; i < sizeof(lg_wheel_products) / sizeof(*lg_wheel_products); ++i) {
        if(lg_wheel_products[i] == product) {
            return 1;
        }
    }
    return 0;
}

#define DEV_INPUT "/dev/input"
#define EV_DEV_NAME "event%u"

static int is_event_file(const struct dirent *dir) {
  unsigned int num;
  if(dir->d_type == DT_CHR && sscanf(dir->d_name, EV_DEV_NAME, &num) == 1 && num < 256) {
    return 1;
  }
  return 0;
}

static int set_evdev_correction(int uhid) {

  int ret = -1;
  int i;
  int fd;
  char device[sizeof("/dev/input/event255")];

  struct dirent **namelist;
  int n;

  n = scandir(DEV_INPUT, &namelist, is_event_file, alphasort);
  if (n >= 0)
  {
    for(i = 0; i < n; ++i)
    {
      if(ret == -1)
      {
        snprintf(device, sizeof(device), "%s/%s", DEV_INPUT, namelist[i]->d_name);

        fd = open (device, O_RDONLY);
        if(fd != -1)
        {
          char uniq[sizeof(((struct uhid_create_req*)NULL)->uniq)] = {};
          if(ioctl(fd, EVIOCGUNIQ(sizeof(uniq)), uniq) != -1)
          {
            pid_t pid;
            int id;
            if(sscanf(uniq, "GIMX %d %d", &pid, &id) == 2)
            {
              if(pid == getpid() && id == uhid)
              {
                struct input_absinfo absinfo = {};
                if(ioctl(fd, EVIOCGABS(ABS_X), &absinfo) != -1)
                {
                  absinfo.flat = 0;
                  absinfo.fuzz = 0;
                  if(ioctl(fd, EVIOCSABS(ABS_X), &absinfo) < 0)
                  {
                    PRINT_ERROR_ERRNO("ioctl EVIOCSABS ABS_X")
                  }
                }
                if(ioctl(fd, EVIOCGABS(ABS_Y), &absinfo) != -1)
                {
                  absinfo.flat = 0;
                  absinfo.fuzz = 0;
                  if(ioctl(fd, EVIOCSABS(ABS_Y), &absinfo) < 0)
                  {
                    PRINT_ERROR_ERRNO("ioctl EVIOCSABS ABS_Y")
                  }
                }
                if(ioctl(fd, EVIOCGABS(ABS_Z), &absinfo) != -1)
                {
                  absinfo.flat = 0;
                  absinfo.fuzz = 0;
                  if(ioctl(fd, EVIOCSABS(ABS_Z), &absinfo) < 0)
                  {
                    PRINT_ERROR_ERRNO("ioctl EVIOCSABS ABS_Z")
                  }
                }
                if(ioctl(fd, EVIOCGABS(ABS_RZ), &absinfo) != -1)
                {
                  absinfo.flat = 0;
                  absinfo.fuzz = 0;
                  if(ioctl(fd, EVIOCSABS(ABS_RZ), &absinfo) < 0)
                  {
                    PRINT_ERROR_ERRNO("ioctl EVIOCSABS ABS_RZ")
                  }
                }
                ret = 0;
                char name[1024] = {};
                if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) < 0)
                {
                  PRINT_ERROR_ERRNO("ioctl EVIOCGNAME")
                }
                printf("reset flat/fuzz values for \"%s\" (%s)\n", name, device);
              }
            }
          }
          close(fd);
        }
      }
      free(namelist[i]);
    }
    free(namelist);
  }
  else
  {
    PRINT_ERROR_ERRNO("scandir")
  }

  return ret;
}

int uhid_joystick_open_all() {

    s_hid_dev * hid_devs = hidasync_enumerate(0x0000, 0x0000);

    s_hid_dev * current;
    for(current = hid_devs; ; ++current) {

        if(is_logitech_wheel(current->vendorId, current->productId)) {

            int hid = hidasync_open_path(current->path);
            if(hid >= 0) {

                const s_hid_info * hid_info = hidasync_get_hid_info(hid);
                int uhid = uhidasync_create(hid_info);
                if(uhid >= 0) {

                    if(set_evdev_correction(uhid) == -1) {
                        PRINT_ERROR_OTHER("cannot set evdev correction")
                        hidasync_close(hid);
                        uhidasync_close(uhid);
                        continue;
                    }

                    if(add_device(hid, uhid) < 0) {

                        PRINT_ERROR_OTHER("cannot add device")
                        hidasync_close(hid);
                        uhidasync_close(uhid);
                        continue;
                    }
                }
            }
        }
        if(current->next == 0) {
            break;
        }
    }

    hidasync_free_enumeration(hid_devs);

    return 0;
}

int uhid_joystick_get_hid_id(int uhid_id) {

    if(uhid_id < 0) {

        PRINT_ERROR_OTHER("uhid_id is invalid")
        return -1;
    }

    int i;
    for(i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if(uhid_joystick_devices[i].uhid == uhid_id) {
            return uhid_joystick_devices[i].hid;
        }
    }

    return -1;
}

int uhid_joystick_close_unused() {

    int i;
    for(i = 0; i < UHID_JOYSTICK_MAX_DEVICES; ++i) {
        if(uhid_joystick_devices[i].uhid < 0
            && uhid_joystick_devices[i].hid < 0) {
            continue;
        }
        if(uhidasync_is_opened(i) == 0) {
            uhid_joystick_close(i);
        }
    }

    return 0;
}

int uhid_joystick_close_all() {

    uhid_joystick_clean();

    return 0;
}
