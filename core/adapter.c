/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <connectors/udp_con.h>
#include <adapter.h>
#include <gimx.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <mainloop.h>
#include <connectors/usb_con.h>

static s_adapter adapter[MAX_CONTROLLERS] = {};

/*
 * These tables are used to retrieve the default controller for a device and vice versa.
 */
static int adapter_device[E_DEVICE_TYPE_NB][MAX_CONTROLLERS];
static int device_adapter[E_DEVICE_TYPE_NB][MAX_DEVICES];

void adapter_init()
{
  unsigned int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter[i].type = C_TYPE_DEFAULT;
    adapter[i].dst_fd = -1;
    adapter[i].src_fd = -1;
  }
  for(j=0; j<E_DEVICE_TYPE_NB; ++j)
  {
    for(i=0; i<MAX_CONTROLLERS; ++i)
    {
      adapter_device[j][i] = -1;
    }
    for(i=0; i<MAX_DEVICES; ++i)
    {
      device_adapter[j][i] = -1;
    }
  }
}

/*
 * Set the port of the controller.
 * If it's already used for another controller, do nothing.
 */
int adapter_set_port(unsigned char index, char* portname)
{
  unsigned char i;
  if(index >= MAX_CONTROLLERS)
  {
    return -1;
  }
  for(i=0; i<index; ++i)
  {
    if(adapter[i].portname && !strcmp(adapter[i].portname, portname))
    {
      return -1;
    }
  }
  adapter[index].portname = portname;
  return 0;
}

inline s_adapter* adapter_get(unsigned char index)
{
  if(index < MAX_CONTROLLERS)
  {
    return adapter+index;
  }
  return NULL;
}

void adapter_set_axis(unsigned char c, int axis, int value)
{
  if(axis >= 0 && axis < AXIS_MAX)
  {
    adapter[c].axis[axis] = value;
  }
}

void adapter_dump_state(int id)
{
  int i;
  struct timeval tv;
  gettimeofday(&tv, NULL);

  s_adapter* adapter = adapter_get(id);
  int* axis = adapter->axis;

  printf("%d %ld.%06ld", id, tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", control_get_name(adapter->type, i), axis[i]);
  }

  printf("\n");
}

/*
 * Read a packet from a remote GIMX client.
 * The packet can be:
 * - a "get controller type" request
 * - a report to be sent.
 * Note that the socket operations should not block.
 */
int adapter_network_read(int id)
{
  static unsigned char buf[256+2];
  int nread = 0;
  struct sockaddr_in sa;
  socklen_t salen = sizeof(sa);
  // retrieve the packet and the address of the client
  if((nread = udp_recvfrom(adapter[id].src_fd, buf, sizeof(buf), (struct sockaddr*) &sa, &salen)) <= 0)
  {
    return 0;
  }
  switch(buf[0])
  {
  case BYTE_TYPE:
    {
      // send the answer
      unsigned char answer[3] = {BYTE_TYPE, BYTE_LEN_1_BYTE, adapter[id].type};
      if (udp_sendto(adapter[id].src_fd, answer, sizeof(answer), (struct sockaddr*) &sa, salen) == -1)
      {
        fprintf(stderr, "adapter_network_read: can't send controller type\n");
        return 0;
      }
    }
    break;
  case BYTE_SEND_REPORT:
    if(buf[1] != sizeof(adapter->axis))
    {
      fprintf(stderr, "adapter_network_read: wrong packet size\n");
      return 0;
    }
    // store the report (no answer)
    memcpy(adapter[id].axis, buf+2, sizeof(adapter->axis));
    adapter[id].send_command = 1;
    break;
  }
  // require a report to be sent immediately, except for a Sixaxis controller working over bluetooth
  if((adapter[id].type == C_TYPE_SIXAXIS || adapter[id].type == C_TYPE_DEFAULT) && adapter[id].bdaddr_dst)
  {
    return 0;
  }
  return 1;
}

int adapter_network_close(int id)
{
  set_done();
  
  //the cleaning is done by connector_close
  
  return 0;
}

/*
 * Set the default device for a controller.
 */
void adapter_set_device(int controller, e_device_type device_type, int device_id)
{
  int type_index = device_type-1;

  if(controller < 0 || controller >= MAX_CONTROLLERS
  || type_index < 0 || type_index >= E_DEVICE_TYPE_NB
  || device_id < 0 || type_index > MAX_DEVICES)
  {
    fprintf(stderr, "set_controller_device error\n");
    return;
  }
  if(adapter_device[type_index][controller] < 0)
  {
    adapter_device[type_index][controller] = device_id;
    device_adapter[type_index][device_id] = controller;
  }
  else if(adapter_device[type_index][controller] != device_id)
  {
    gprintf(_("macros are not not available for: "));
    if(device_type == E_DEVICE_TYPE_KEYBOARD)
    {
      gprintf(_("keyboard %s (%d)\n"), GE_KeyboardName(device_id), GE_KeyboardVirtualId(device_id));
    }
    else if(device_type == E_DEVICE_TYPE_MOUSE)
    {
      gprintf(_("mouse %s (%d)\n"), GE_MouseName(device_id), GE_MouseVirtualId(device_id));
    }
    else if(device_type == E_DEVICE_TYPE_JOYSTICK)
    {
      gprintf(_("joystick %s (%d)\n"), GE_JoystickName(device_id), GE_JoystickVirtualId(device_id));
    }
  }
}

/*
 * Get the default device for a controller.
 */
int adapter_get_device(e_device_type device_type, int controller)
{
  return adapter_device[device_type-1][controller];
}

/*
 * Get the default controller for a device.
 */
int adapter_get_controller(e_device_type device_type, int device_id)
{
  return device_adapter[device_type-1][device_id];
}

int adapter_forward_data_in(int id, unsigned char* data, unsigned char length)
{
  if(adapter[id].portname >= 0)
  {
    unsigned char header[] = {BYTE_SPOOF_DATA, length};
    if(serial_sendv(id, header, HEADER_SIZE, data, length) < 0)
    {
      return -1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    fprintf(stderr, "no serial port opened for adapter %d\n", id);
    return -1;
  }
}

int adapter_forward_data_out(int id, unsigned char* data, unsigned char length)
{
  return usb_send(id, data, length);
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_get_type(int id)
{
  unsigned char get_type_request[] = {BYTE_TYPE, BYTE_LEN_0_BYTE};

  if(serial_send(id, get_type_request, sizeof(get_type_request)) == sizeof(get_type_request))
  {
    unsigned char get_type_answer[3];

    if(serial_recv(id, get_type_answer, sizeof(get_type_answer)) == sizeof(get_type_answer))
    {
      if(get_type_answer[0] == BYTE_TYPE && get_type_answer[1] == BYTE_LEN_1_BYTE)
      {
        return get_type_answer[2];
      }
    }
  }

  return -1;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_send_start(int id)
{
  unsigned char spoof_request[] = {BYTE_START_SPOOF, BYTE_LEN_0_BYTE};
  if(serial_send(id, spoof_request, sizeof(spoof_request)) < sizeof(spoof_request))
  {
    fprintf(stderr, "serial_send\n");
    return -1;
  }

  unsigned char spoof_answer[3];

  if(serial_recv(id, spoof_answer, sizeof(spoof_answer)) < sizeof(spoof_answer))
  {
    fprintf(stderr, "serial_recv\n");
    return -1;
  }

  if(spoof_answer[0] != BYTE_START_SPOOF && spoof_answer[1] != BYTE_LEN_1_BYTE)
  {
    fprintf(stderr, "bad response\n");
    return -1;
  }

  if(spoof_answer[2] == BYTE_STATUS_SPOOFED)
  {
    return 1;
  }

  return 0;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_get_status(int id)
{
  unsigned char get_status_request[] = {BYTE_STATUS, BYTE_LEN_0_BYTE};

  if(serial_send(id, get_status_request, sizeof(get_status_request)) == sizeof(get_status_request))
  {
    unsigned char get_status_answer[3];

    if(serial_recv(id, get_status_answer, sizeof(get_status_answer)) == sizeof(get_status_answer))
    {
      if(get_status_answer[0] == BYTE_TYPE && get_status_answer[1] == BYTE_LEN_1_BYTE)
      {
        return get_status_answer[2];
      }
    }
  }

  return -1;
}

int adapter_send_reset(int id)
{
  unsigned char reset_request[] = {BYTE_RESET, BYTE_LEN_0_BYTE};

  if(serial_send(id, reset_request, sizeof(reset_request)) != sizeof(reset_request))
  {
    return -1;
  }

  return 0;
}
