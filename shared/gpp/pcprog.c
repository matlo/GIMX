#include "pcprog.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include <hidapi/hidapi.h>

#ifdef WIN32
#define LINE_MAX 1024
#endif

#define USB_IDS_FILE "gpp.txt"

#define MAX_GPP_DEVICES 8

#define GPPKG_INPUT_REPORT      0x01
#define GPPKG_OUTPUT_REPORT     0x04
#define GPPKG_ENTER_CAPTURE     0x07
#define GPPKG_LEAVE_CAPTURE     0x08

#define REPORT_SIZE 64

typedef struct __attribute__ ((gcc_struct,packed))
{
  uint8_t type;
  uint16_t length;
  uint8_t first;
} s_gppReportHeader;

typedef struct __attribute__ ((gcc_struct,packed))
{
  uint8_t reportId;
  s_gppReportHeader header;
  uint8_t data[REPORT_SIZE-sizeof(s_gppReportHeader)];
} s_gppReport;

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
    fprintf(stderr, "%s:%d no space for any user defined usb ids!\n", __FILE__, __LINE__);
    return;
  }

  snprintf(file_path, sizeof(file_path), "%s/%s/%s", user_directory, app_directory, USB_IDS_FILE);

  fp = fopen(file_path, "r");
  if (fp)
  {
    while (fgets(line, LINE_MAX, fp))
    {
      if(sscanf(line, "%hx %hx", &vid, &pid) == 2)
      {
        if(nb_usb_ids == sizeof(usb_ids) / sizeof(*usb_ids))
        {
          fprintf(stderr, "%s:%d no more space for user defined usb ids!\n", __FILE__, __LINE__);
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

static struct
{
  hid_device * dev;
  char * path;
} devices[MAX_GPP_DEVICES] = {};

int8_t gpppcprog_send(int id, uint8_t type, uint8_t *data, uint16_t length);

static uint8_t is_device_opened(const char* device)
{
  int i;
  for(i = 0; i < sizeof(devices) / sizeof(*devices); ++i)
  {
    if(devices[i].path && !strcmp(devices[i].path, device))
    {
      return 1;
    }
  }
  return 0;
}

int8_t gppcprog_connect(int id, const char* device)
{
  int r = 0;

  gppcprog_disconnect(id);

  if(device)
  {
    // Connect to the given device

    if(is_device_opened(device))
    {
      return -1;
    }

    devices[id].dev = hid_open_path(device);

    if(devices[id].dev)
    {
      devices[id].path = strdup(device);
    }
    else
    {
      fprintf(stderr, "failed to open %s\n", device);
    }
  }
  else
  {
    // Connect to any GPP/Cronus/Titan

    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0000, 0x0000);
    cur_dev = devs;
    while (cur_dev)
    {
      int i;
      for(i = 0; i < sizeof(usb_ids) / sizeof(*usb_ids); ++i)
      {
        if(cur_dev->vendor_id == usb_ids[i].vid && cur_dev->product_id == usb_ids[i].pid)
        {
          if(!is_device_opened(cur_dev->path))
          {
            if ((devices[id].dev = hid_open_path(cur_dev->path)))
            {
              devices[id].path = strdup(cur_dev->path);
              break;
            }
          }
        }
      }
      if(devices[id].dev)
      {
        break;
      }
      cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
  }

  if (!devices[id].dev)
  {
    return -1;
  }

  // Enter Capture Mode
  r = gpppcprog_send(id, GPPKG_ENTER_CAPTURE, NULL, 0);
  if (r <= 0)
  {
    gppcprog_disconnect(id);
    return r;
  }

  return 1;
}

int8_t gppcprog_connected(int id)
{
  return !devices[id].dev;
}

void gppcprog_disconnect(int id)
{
  if (devices[id].dev)
  {
    // Leave Capture Mode
    gpppcprog_send(id, GPPKG_LEAVE_CAPTURE, NULL, 0);

    // Disconnect to GPP
    hid_close(devices[id].dev);
    devices[id].dev = NULL;
    free(devices[id].path);
    devices[id].path = NULL;
  }
  return;
}

int8_t gpppcprog_input(int id, GCAPI_REPORT *report, int timeout)
{
  int bytesReceived;
  uint8_t rcvBuf[64], i;

  if (!devices[id].dev || report == NULL)
    return (-1);
  bytesReceived = hid_read_timeout(devices[id].dev, rcvBuf, 64, timeout);
  if (bytesReceived < 0)
  {
    gppcprog_disconnect(id);
    return (-1);
  }
  else if (bytesReceived && rcvBuf[0] == GPPKG_INPUT_REPORT)
  {
    int8_t *rep01 = (int8_t *) (rcvBuf + 7);
    report->controller = *rep01++;
    report->console = *rep01++;
    report->led[0] = *rep01++;
    report->led[1] = *rep01++;
    report->led[2] = *rep01++;
    report->led[3] = *rep01++;
    report->rumble[0] = *rep01++;
    report->rumble[1] = *rep01++;
    report->battery_level = *rep01++;
    for (i = 0; i < GCAPI_INPUT_TOTAL; i++)
    {
      report->input[i].value = *rep01++;
    }
    return (1);
  }
  return (0);
}

int8_t gpppcprog_output(int id, int8_t *output)
{
  uint8_t outputReport[GCAPI_INPUT_TOTAL + 6];
  memset(outputReport, 0x00, GCAPI_INPUT_TOTAL + 6);
  memcpy(outputReport, (uint8_t *) output, GCAPI_INPUT_TOTAL);
  return (gpppcprog_send(id, GPPKG_OUTPUT_REPORT, outputReport,
      GCAPI_INPUT_TOTAL + 6));
}

int8_t gpppcprog_send(int id, uint8_t type, uint8_t *data, uint16_t length)
{
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
  {										// Data
    if (length)
    {
      sndLen = (((i + sizeof(report.data)) < length) ? sizeof(report.data) : (length - i));
      memcpy(report.data, data + i, sndLen);
      i += sndLen;
    }
    if (hid_write(devices[id].dev, (unsigned char*)&report, 64) == -1)
      return (0);
    report.header.first = 0;
  }
  while (i < length);
  return (1);
}

