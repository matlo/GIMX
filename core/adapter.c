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
#include <connectors/protocol.h>

#include <connectors/ffb_logitech.h>

static s_adapter adapter[MAX_CONTROLLERS] = {};

static struct
{
  unsigned char weak;
  unsigned char strong;
} motors[C_TYPE_MAX] =
{
  [C_TYPE_DS4]     = { .weak = 4, .strong = 5 },
  [C_TYPE_360_PAD] = { .weak = 4, .strong = 3 },
  [C_TYPE_SIXAXIS] = { .weak = 3, .strong = 5 },
  [C_TYPE_XONE_PAD] = { .weak = 7, .strong = 6 },
};

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
    for(j = 0; j < MAX_REPORTS; ++j)
    {
      adapter[i].report[j].type = BYTE_IN_REPORT;
    }
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
  if(nread < 2)
  {
    fprintf(stderr, "invalid packet size: %d\n", nread);
    return 0;
  }
  switch(buf[0])
  {
  case BYTE_TYPE:
    {
      // send the answer
      unsigned char answer[3] = {BYTE_TYPE, BYTE_LEN_1_BYTE, adapter[id].type};
      if (udp_sendto(adapter[id].src_fd, answer, sizeof(answer), (struct sockaddr*) &sa, salen) < 0)
      {
        fprintf(stderr, "adapter_network_read: can't send controller type\n");
        return 0;
      }
    }
    break;
  case BYTE_IN_REPORT:
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

int adapter_forward_control_in(int id, unsigned char* data, unsigned char length)
{
  if(adapter[id].portname >= 0)
  {
    s_packet packet =
    {
      .header =
      {
        .type = BYTE_CONTROL_DATA,
        .length = length
      }
    };
    memcpy(packet.value, data, length);
    if(serial_send(id, &packet, sizeof(packet.header)+packet.header.length) < 0)
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

int adapter_forward_interrupt_in(int id, unsigned char* data, unsigned char length)
{
  if(adapter[id].portname >= 0)
  {
    s_packet packet =
    {
      .header =
      {
        .type = BYTE_IN_REPORT,
        .length = length
      }
    };
    memcpy(packet.value, data, length);
    if(serial_send(id, &packet, sizeof(packet.header)+packet.header.length) < 0)
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

int adapter_forward_control_out(int id, unsigned char* data, unsigned char length)
{
  return usb_send_control(id, data, length);
}

int adapter_forward_interrupt_out(int id, unsigned char* data, unsigned char length)
{
  return usb_send_interrupt_out(id, data, length);
}

static int debug = 1;

static void dump(unsigned char* packet, unsigned char length)
{
  int i;
  for(i=0; i<length; ++i)
  {
    if(i && !(i%8))
    {
      gprintf("\n");
    }
    gprintf("0x%02x ", packet[i]);
  }
  gprintf("\n");
}

int adapter_process_packet(int id, s_packet* packet)
{
  unsigned char type = packet->header.type;
  unsigned char length = packet->header.length;
  unsigned char* data = packet->value;

  int ret = 0;

  if(type == BYTE_CONTROL_DATA)
  {
    ret = adapter_forward_control_out(id, data, length);

    if(ret < 0)
    {
      fprintf(stderr, "adapter_forward_data_out failed\n");
    }
  }
  else if(type == BYTE_OUT_REPORT)
  {
    int joystick = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, id);

    unsigned char send = 0;

    switch(adapter[id].type)
    {
      case C_TYPE_DS4:
        if(GE_GetJSType(joystick) == GE_JS_DS4)
        {
          ret = adapter_forward_interrupt_out(id, data, length);
          if(ret < 0)
          {
            fprintf(stderr, "adapter_forward_interrupt_out failed\n");
          }
        }
        else
        {
          send = 1;
        }
        break;
      case C_TYPE_360_PAD:
        if(GE_GetJSType(joystick) == GE_JS_360PAD)
        {
          ret = adapter_forward_interrupt_out(id, data, length);
          if(ret < 0)
          {
            fprintf(stderr, "adapter_forward_interrupt_out failed\n");
          }
        }
        else
        {
          send = 1;
        }
        break;
      case C_TYPE_SIXAXIS:
        send = 1;
        break;
      case C_TYPE_XONE_PAD:
        if(data[0] != XONE_USB_HID_RUMBLE_REPORT_ID || GE_GetJSType(joystick) == GE_JS_XONEPAD)
        {
          if(debug)
          {
            gprintf("forward OUT\n");
            dump(data, length);
          }
          ret = adapter_forward_interrupt_out(id, data, length);
          if(ret < 0)
          {
            fprintf(stderr, "adapter_forward_interrupt_out failed\n");
          }
        }
        else
        {
          send = 1;
        }
        break;
      default:
        break;
    }
    if(send && GE_JoystickHasRumble(joystick))
    {
      GE_Event event =
      {
        .jrumble =
        {
          .type = GE_JOYRUMBLE,
          .which = joystick,
          .weak = data[motors[adapter[id].type].weak] << 8,
          .strong = data[motors[adapter[id].type].strong] << 8
        }
      };
      GE_PushEvent(&event);
    }
  }
  else if(type == BYTE_DEBUG)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    gprintf("%ld.%06ld debug packet received (size = %d bytes)\n", tv.tv_sec, tv.tv_usec, length);
    dump(packet->value, length);
    //ffb_logitech_decode(data, length);
  }
  else
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
        fprintf(stderr, "%ld.%06ld ", tv.tv_sec, tv.tv_usec);
    fprintf(stderr, "unhandled packet (type=0x%02x)\n", type);
  }

  return ret;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
static int adapter_send_short_command(int id, unsigned char type)
{
  s_packet packet =
  {
    .header =
    {
      .type = type,
      .length = BYTE_LEN_0_BYTE
    }
  };

  if(serial_send(id, &packet.header, sizeof(packet.header)) < sizeof(packet.header))
  {
    fprintf(stderr, "serial_send\n");
    return -1;
  }

  /*
   * The adapter may send a packet before it processes the command,
   * so it is possible to receive a packet that is not the command response.
   */
  while(1)
  {
    if(serial_recv(id, &packet.header, sizeof(packet.header)) < sizeof(packet.header))
    {
      fprintf(stderr, "can't read packet header\n");
      return -1;
    }

    if(serial_recv(id, &packet.value, packet.header.length) < packet.header.length)
    {
      fprintf(stderr, "can't read packet data\n");
      return -1;
    }

    //Check this packet is the command response.
    if(packet.header.type == type)
    {
      if(packet.header.length != BYTE_LEN_1_BYTE)
      {
        fprintf(stderr, "bad response\n");
        return -1;
      }

      return packet.value[0];
    }
  }

  return 0;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_get_type(int id)
{
  return adapter_send_short_command(id, BYTE_TYPE);
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_send_start(int id)
{
  return adapter_send_short_command(id, BYTE_START);
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
int adapter_get_status(int id)
{
  return adapter_send_short_command(id, BYTE_STATUS);
}

int adapter_send_reset(int id)
{
  s_header header =
  {
    .type = BYTE_RESET,
    .length = BYTE_LEN_0_BYTE
  };

  if(serial_send(id, &header, sizeof(header)) != sizeof(header))
  {
    return -1;
  }

  return 0;
}
