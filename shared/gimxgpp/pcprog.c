#include "pcprog.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <gimxlog/include/glog.h>
#include <stddef.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <gimxfile/include/gfile.h>

#define USB_IDS_FILE "gpp.txt"

#define MAX_GPP_DEVICES 8

#define GPPKG_INPUT_REPORT      0x01
#define GPPKG_OUTPUT_REPORT     0x04
#define GPPKG_ENTER_CAPTURE     0x07
#define GPPKG_LEAVE_CAPTURE     0x08

#define REPORT_SIZE 64

typedef struct GIMX_PACKED
{
  uint8_t type;
  uint16_t length;
  uint8_t first;
} s_gppReportHeader;

typedef struct GIMX_PACKED
{
  uint8_t reportId;
  s_gppReportHeader header;
  uint8_t data[REPORT_SIZE-sizeof(s_gppReportHeader)];
} s_gppReport;

GLOG_INST(GLOG_NAME)

/*
 * https://www.consoletuner.com/kbase/compatible_devices_print.htm
 * http://controllermax.com/manual/Content/compatible_devices.htm
 */
GCAPI_USB_IDS usb_ids[16] =
{
    { .vid = 0x2508, .pid = 0x0001, .name = "GPP"                 }, //GPP, Cronus, CronusMAX, CronusMAX v2
    { .vid = 0x2508, .pid = 0x0002, .name = "Cronus"              }, //Cronus
    { .vid = 0x2508, .pid = 0x0003, .name = "Titan"               }, //Titan
    { .vid = 0x2508, .pid = 0x0004, .name = "CronusMAX v2"        }, //CronusMAX v2
    { .vid = 0x2008, .pid = 0x0001, .name = "CronusMAX PLUS (v3)" }, //CronusMAX PLUS (v3)

    // remaining space is for user-defined values!
};

static unsigned int nb_usb_ids = 5;

/*
 * Read user-defined usb ids.
 */
void gpppcprog_read_user_ids(const char * user_directory, const char * app_directory)
{
  char file_path[PATH_MAX];
  char line[LINE_MAX];
  FILE * fp;
  unsigned short vid;
  unsigned short pid;

  if(nb_usb_ids == sizeof(usb_ids) / sizeof(*usb_ids))
  {
    if (GLOG_LEVEL(GLOG_NAME,ERROR))
    {
      fprintf(stderr, "%s:%d no space for any user defined usb ids!\n", __FILE__, __LINE__);
    }
    return;
  }

  snprintf(file_path, sizeof(file_path), "%s/%s/%s", user_directory, app_directory, USB_IDS_FILE);

  fp = gfile_fopen(file_path, "r");
  if (fp)
  {
    while (fgets(line, LINE_MAX, fp))
    {
      if(sscanf(line, "%hx %hx", &vid, &pid) == 2)
      {
        if(nb_usb_ids == sizeof(usb_ids) / sizeof(*usb_ids))
        {
          if (GLOG_LEVEL(GLOG_NAME,ERROR))
          {
            fprintf(stderr, "%s:%d no more space for user defined usb ids!\n", __FILE__, __LINE__);
          }
          break;
        }
        if(vid && pid)
        {
          usb_ids[nb_usb_ids].vid = vid;
          usb_ids[nb_usb_ids].pid = pid;
          usb_ids[nb_usb_ids].name = "user-defined";
          ++nb_usb_ids;
        }
      }
    }
    fclose(fp);
  }
}

const GCAPI_USB_IDS * gpppcprog_get_ids(unsigned int * nb)
{
  *nb = nb_usb_ids;
  return usb_ids;
}

typedef struct
{
  struct ghid_device * device;
  char * path;
  unsigned int pending;
  GHID_READ_CALLBACK fp_read;
  GHID_WRITE_CALLBACK fp_write;
  GHID_CLOSE_CALLBACK fp_close;
} s_gpp_device;

static s_gpp_device devices[MAX_GPP_DEVICES] = {};

static inline int check_device(int device, const char * file, unsigned int line, const char * func) {
  if (device < 0 || device >= MAX_GPP_DEVICES) {
    if (GLOG_LEVEL(GLOG_NAME,ERROR))
    {
      fprintf(stderr, "%s:%d %s: invalid device\n", file, line, func);
    }
    return -1;
  }
  return 0;
}
#define CHECK_DEVICE(device,retValue) \
  if(check_device(device, __FILE__, __LINE__, __func__) < 0) { \
    return retValue; \
  }

static int8_t gpppcprog_send(int id, uint8_t type, uint8_t * data, uint16_t length)
{
  CHECK_DEVICE(id, -1)

  if (devices[id].pending > 0)
  {
    return 0;
  }
  s_gppReport report =
  {
    .reportId = 0x00,
    .header =
    {
      .type = type,
      .length = length,
      .first = 1
    }
  };
  uint16_t sndLen;
  uint16_t i = 0;

  do
  {
    if (length)
    {
      sndLen = (((i + sizeof(report.data)) < length) ? sizeof(report.data) : (uint16_t)(length - i));
      memcpy(report.data, data + i, sndLen);
      i += sndLen;
    }
    if(devices[id].fp_write) {
      if (ghid_write(devices[id].device, (unsigned char*)&report, sizeof(report)) == -1)
      {
        return -1;
      }
      ++devices[id].pending;
    }
    else {
      if (ghid_write_timeout(devices[id].device, (unsigned char*)&report, sizeof(report), 1000) == -1)
        return -1;
    }
    report.header.first = 0;
  }
  while (i < length);
  return 1;
}

static uint8_t is_device_opened(const char* device)
{
  unsigned int i;
  for(i = 0; i < sizeof(devices) / sizeof(*devices); ++i)
  {
    if(devices[i].path && !strcmp(devices[i].path, device))
    {
      return 1;
    }
  }
  return 0;
}

int8_t gppcprog_connect(int id, const char * path)
{
  CHECK_DEVICE(id, -1)

  int r = 0;

  if(path)
  {
    // Connect to the given device

    if(is_device_opened(path))
    {
      return -1;
    }

    if (ghid_init() < 0)
    {
      return -1;
    }

    devices[id].device = ghid_open_path(path);

    if(devices[id].device != NULL)
    {
      devices[id].path = strdup(path);
    }
    else
    {
      ghid_exit();
    }
  }
  else
  {
    if (ghid_init() < 0)
    {
      return -1;
    }
    // Connect to any GPP/Cronus/Titan

    struct ghid_device_info *devs, *cur_dev;

    devs = ghid_enumerate(0x0000, 0x0000);
    for(cur_dev = devs; cur_dev != NULL; cur_dev = cur_dev->next)
    {
      unsigned int i;
      for(i = 0; i < sizeof(usb_ids) / sizeof(*usb_ids); ++i)
      {
        if(cur_dev->vendor_id == usb_ids[i].vid && cur_dev->product_id == usb_ids[i].pid)
        {
          if(!is_device_opened(cur_dev->path))
          {
            if ((devices[id].device = ghid_open_path(cur_dev->path)) != NULL)
            {
              devices[id].path = strdup(cur_dev->path);
              break;
            }
          }
        }
      }
      if(devices[id].device != NULL)
      {
        break;
      }
    }
    ghid_free_enumeration(devs);
  }

  if (devices[id].device == NULL)
  {
    ghid_exit();
    return -1;
  }

  // Enter Capture Mode
  r = gpppcprog_send(id, GPPKG_ENTER_CAPTURE, NULL, 0);
  if (r <= 0)
  {
    return r;
  }

  return 1;
}

void gppcprog_disconnect(int id)
{
  CHECK_DEVICE(id,)

  if (devices[id].device != NULL)
  {
    //make sure the write is synchronous
    devices[id].fp_write = NULL;

    //unblock sending
    devices[id].pending = 0;

    // Leave Capture Mode
    gpppcprog_send(id, GPPKG_LEAVE_CAPTURE, NULL, 0);

    // Disconnect to GPP
    ghid_close(devices[id].device);
    devices[id].device = NULL;
    free(devices[id].path);
    devices[id].path = NULL;

    ghid_exit();
  }
  return;
}

int8_t gpppcprog_input(int id, GCAPI_REPORT *report, int timeout)
{
  CHECK_DEVICE(id, -1)

  int bytesReceived;
  uint8_t rcvBuf[65] = {};

  if (devices[id].device == NULL || report == NULL)
    return (-1);
  bytesReceived = ghid_read_timeout(devices[id].device, rcvBuf, sizeof(rcvBuf), timeout);
  if (bytesReceived < 0)
  {
    return (-1);
  }
  else if (bytesReceived && rcvBuf[0] == GPPKG_INPUT_REPORT)
  {
    *report = *(GCAPI_REPORT *)(rcvBuf + 7);
    return (1);
  }
  return (0);
}

static int read_callback(void * user, const void * buf, int status)
{
  if (status < 0) {
    return 1;
  }

  s_gpp_device * device = (s_gpp_device *) user;

  //TODO MLA: only 1 poll in each period
  int ret = ghid_poll(device->device);
  if (ret < 0) {
    return 1;
  }

  if(status > 0 && ((unsigned char *)buf)[0] == GPPKG_INPUT_REPORT) {
    return device->fp_read((void *)(intptr_t)(ptrdiff_t)(device - devices), buf, status);
  }

  return 0;
}

static int write_callback(void * user, int transfered)
{
  s_gpp_device * device = (s_gpp_device *) user;

  if (device->pending > 0)
  {
    --device->pending;
  }
  if(device->fp_write)
  {
    return device->fp_write((void *)(intptr_t)(ptrdiff_t)(device - devices), transfered);
  }
  return 0;
}

static int close_callback(void * user)
{
  s_gpp_device * device = (s_gpp_device *) user;

  return device->fp_close((void *)(intptr_t)(ptrdiff_t)(device - devices));
}

int8_t gpppcprog_start_async(int id, const GHID_CALLBACKS * callbacks)
{
  CHECK_DEVICE(id, -1)

  devices[id].fp_read = callbacks->fp_read;
  devices[id].fp_write = callbacks->fp_write;
  devices[id].fp_close = callbacks->fp_close;

  GHID_CALLBACKS ghid_callbacks = {
          .fp_read = read_callback,
          .fp_write = write_callback,
          .fp_close = close_callback,
          .fp_register = callbacks->fp_register,
          .fp_remove = callbacks->fp_remove,
  };

  int ret = ghid_register(devices[id].device, devices + id, &ghid_callbacks);
  if (ret < 0) {
    return -1;
  }

  ret = ghid_poll(devices[id].device);
  if (ret < 0) {
    return -1;
  }

  return 0;
}

int8_t gpppcprog_output(int id, int8_t output[GCAPI_INPUT_TOTAL])
{
  CHECK_DEVICE(id, -1)

  return gpppcprog_send(id, GPPKG_OUTPUT_REPORT, (uint8_t *)output, GCAPI_INPUT_TOTAL);
}
