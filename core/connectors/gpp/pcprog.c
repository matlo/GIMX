#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <hidapi/hidapi.h>
#include "pcprog.h"

#include <adapter.h>

#define GPPKG_INPUT_REPORT      0x01
#define GPPKG_OUTPUT_REPORT     0x04
#define GPPKG_ENTER_CAPTURE     0x07
#define GPPKG_LEAVE_CAPTURE     0x08

#define CONSOLETUNER_VID 0x2508

#define GPP_PID     0x0001
#define CRONUS_PID  0x0002
#define TITAN_PID   0x0003

static unsigned short product_ids[] =
{
  GPP_PID,
  CRONUS_PID,
  TITAN_PID,
};

static struct
{
  hid_device* dev;
  char* path;
} devices[MAX_CONTROLLERS] = {};

int8_t gpppcprog_send(int id, uint8_t type, uint8_t *data, uint16_t lenght);

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

    devs = hid_enumerate(CONSOLETUNER_VID, 0x0);
    cur_dev = devs;
    while (cur_dev)
    {
      int i;
      for(i = 0; i < sizeof(product_ids) / sizeof(*product_ids); ++i)
      {
        if(cur_dev->product_id == product_ids[i])
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

int8_t gpppcprog_send(int id, uint8_t type, uint8_t *data, uint16_t lenght)
{
  uint8_t sndBuf[64];
  uint16_t sndLen;
  uint16_t i = 0;

  *(sndBuf + 0) = type;						    // Report type
  *((uint16_t *) (sndBuf + 1)) = lenght;           // Total length
  *(sndBuf + 3) = 1;							// First Packet
  do
  {										// Data
    if (lenght)
    {
      sndLen = (((i + 60) < lenght) ? 60 : (lenght - i));
      memcpy(sndBuf + 4, data + i, sndLen);
      i += sndLen;
    }
    if (hid_write(devices[id].dev, sndBuf, 64) == -1)
      return (0);
    if (*(sndBuf + 3) == 1)
      *(sndBuf + 3) = 0;
  }
  while (i < lenght);
  return (1);
}

