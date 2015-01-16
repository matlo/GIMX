#include <stdlib.h>
#include <string.h>

#include <hidapi/hidapi.h>
#include "pcprog.h"

#define GPPKG_INPUT_REPORT      0x01
#define GPPKG_OUTPUT_REPORT     0x04
#define GPPKG_ENTER_CAPTURE     0x07
#define GPPKG_LEAVE_CAPTURE     0x08

static unsigned short product_ids[] =
{
    0x0001, //GPP
    0x0002, //Cronus
    0x0003, //Titan
};

static hid_device* dev = NULL;

int8_t gpppcprog_send(uint8_t type, uint8_t *data, uint16_t lenght);

int8_t gppcprog_connected_flag = 0;

int8_t gppcprog_connect()
{
  int r = 0;

  gppcprog_disconnect();

  // Connect to GPP/Cronus/Titan
  int i;
  for(i=0; i<sizeof(product_ids)/sizeof(*product_ids); ++i)
  {
    if ((dev = hid_open(0x2508, product_ids[i], NULL)))
    {
      break;
    }
  }

  if (!dev)
  {
    return -1;
  }

  gppcprog_connected_flag = 1;

  // Enter Capture Mode
  r = gpppcprog_send(GPPKG_ENTER_CAPTURE, NULL, 0);
  if (r <= 0)
  {
    gppcprog_disconnect();
    return r;
  }

  return 1;
}

int8_t gppcprog_connected()
{
  return (gppcprog_connected_flag);
}

void gppcprog_disconnect()
{
  if (gppcprog_connected_flag)
  {
    // Leave Capture Mode
    gpppcprog_send(GPPKG_LEAVE_CAPTURE, NULL, 0);

    // Disconnect to GPP
    hid_close(dev);
    gppcprog_connected_flag = 0;
  }
  return;
}

int8_t gpppcprog_input(GCAPI_REPORT *report, int timeout)
{
  int bytesReceived;
  uint8_t rcvBuf[64], i;

  if (!gppcprog_connected_flag || report == NULL)
    return (-1);
  bytesReceived = hid_read_timeout(dev, rcvBuf, 64, timeout);
  if (bytesReceived < 0)
  {
    gppcprog_disconnect();
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

int8_t gpppcprog_output(int8_t *output)
{
  uint8_t outputReport[GCAPI_INPUT_TOTAL + 6];
  memset(outputReport, 0x00, GCAPI_INPUT_TOTAL + 6);
  memcpy(outputReport, (uint8_t *) output, GCAPI_INPUT_TOTAL);
  return (gpppcprog_send(GPPKG_OUTPUT_REPORT, outputReport,
      GCAPI_INPUT_TOTAL + 6));
}

int8_t gpppcprog_send(uint8_t type, uint8_t *data, uint16_t lenght)
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
    if (hid_write(dev, sndBuf, 64) == -1)
      return (0);
    if (*(sndBuf + 3) == 1)
      *(sndBuf + 3) = 0;
  }
  while (i < lenght);
  return (1);
}

