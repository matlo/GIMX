/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <adapter.h>
#include <emuclient.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

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
    adapter[i].serial = SERIALOBJECT_UNDEF;
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

void adapter_dump_state(s_adapter* c)
{
  int i;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int* axis = c->axis;

  printf("%ld %ld.%06ld", (c-adapter)/sizeof(s_adapter), tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", control_get_name(c->type, i), axis[i]);
  }

  printf("\n");
}

int adapter_network_read(int id)
{
  unsigned char buf[sizeof(adapter->axis)];
  int nread = 0;
  int ret;
  while(nread != sizeof(buf))
  {
    if((ret = read(adapter[id].src_fd, buf+nread, sizeof(buf)-nread)) < 0)
    {
      if(errno != EAGAIN)
      {
        return -1;
      }
    }
    else
    {
      nread += ret;
    }
  }
  memcpy(adapter[id].axis, buf, sizeof(adapter->axis));
  adapter[id].send_command = 1;
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
