/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <adapter.h>
#include <gimx.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <mainloop.h>
#include <display.h>
#include <stats.h>
#include <connectors/protocol.h>
#include <connectors/udp_con.h>
#include <connectors/gpp_con.h>
#include <connectors/usb_con.h>
#include "connectors/sixaxis.h"
#ifndef WIN32
#include "connectors/btds4.h"
#endif
#include <controller2.h>

#include <ffb_logitech.h>
#include <hidasync.h>
#include <uhidasync.h>
#include <uhid_joystick.h>

#define BAUDRATE 500000 //bps
#define SERIAL_TIMEOUT 1 //second
/*
 * The adapter restarts about 15ms after receiving the reset command.
 * This time is doubled so as to include the reset command transfer duration.
 */
#define ADAPTER_RESET_TIME 30000 //microseconds

#ifdef WIN32
#define REGISTER_FUNCTION GE_AddSourceHandle
#else
#define REGISTER_FUNCTION GE_AddSource
#endif

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

void adapter_init_static(void) __attribute__((constructor (101)));
void adapter_init_static(void)
{
  unsigned int i, j;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter[i].atype = E_ADAPTER_TYPE_NONE;
    adapter[i].ctype = C_TYPE_NONE;
    adapter[i].dst_fd = -1;
    adapter[i].src_fd = -1;
    adapter[i].serialdevice = -1;
    adapter[i].hid_id = -1;
#ifndef WIN32
    adapter[i].uhid_id = -1;
#endif
    adapter[i].bread = 0;
    for(j = 0; j < MAX_REPORTS; ++j)
    {
      adapter[i].report[j].type = BYTE_IN_REPORT;
    }
    adapter[i].status = 0;
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

static void adapter_dump_state(int id)
{
  int i;
  struct timeval tv;
  gettimeofday(&tv, NULL);

  s_adapter* adapter = adapter_get(id);
  int* axis = adapter->axis;

  printf("%d %ld.%06ld", id, tv.tv_sec, tv.tv_usec);

  for (i = 0; i < AXIS_MAX; i++) {
      if (axis[i])
          printf(", %s (%d)", controller_get_axis_name(adapter->ctype, i), axis[i]);
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
static int adapter_network_read(int id)
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
      unsigned char answer[3] = {BYTE_TYPE, BYTE_LEN_1_BYTE, adapter[id].ctype};
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
  if(adapter[id].ctype == C_TYPE_SIXAXIS && adapter[id].atype == E_ADAPTER_TYPE_BLUETOOTH)
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
    static unsigned char warned[E_DEVICE_TYPE_NB][MAX_DEVICES] = {};
    if(warned[type_index][device_id] == 0)
    {
      warned[type_index][device_id] = 1;
      gprintf(_("macros are not available for: "));
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

static int debug = 0;

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

#define DEBUG_PACKET(PACKET, LENGTH) \
  if(debug) \
  { \
    gprintf("%s\n", __func__); \
    dump(data, length); \
  }

static int adapter_forward(int id, unsigned char type, unsigned char* data, unsigned char length)
{
  if(adapter[id].serialdevice >= 0)
  {
    DEBUG_PACKET(data, length)
    s_packet packet =
    {
      .header =
      {
        .type = type,
        .length = length
      }
    };
    memcpy(packet.value, data, length);
    if(serialasync_write(adapter[id].serialdevice, &packet, sizeof(packet.header)+packet.header.length) < 0)
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

int adapter_forward_control_in(int id, unsigned char* data, unsigned char length)
{
  return adapter_forward(id, BYTE_CONTROL_DATA, data, length);
}

int adapter_forward_interrupt_in(int id, unsigned char* data, unsigned char length)
{
  return adapter_forward(id, BYTE_IN_REPORT, data, length);
}

static int adapter_forward_control_out(int id, unsigned char* data, unsigned char length)
{
  DEBUG_PACKET(data, length)
  return usb_send_control(id, data, length);
}

static int adapter_forward_interrupt_out(int id, unsigned char* data, unsigned char length)
{
  DEBUG_PACKET(data, length)
  if(adapter[id].ctype == C_TYPE_XONE_PAD && data[0] == 0x06 && data[1] == 0x20)
  {
    adapter[id].status = 1;
  }
  return usb_send_interrupt_out(id, data, length);
}

static void adapter_send_next_hid_report(int id)
{
  if(!adapter[id].hid_busy)
  {
    s_ffb_report * report = ffb_logitech_get_report(id);
    if(report != NULL)
    {
      if(hidasync_write(adapter[id].hid_id, report->data, sizeof(report->data)) == 0)
      {
        adapter->hid_busy = 1;
      }
    }
  }
}

static int adapter_process_packet(int id, s_packet* packet)
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

    switch(adapter[id].ctype)
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
        if(GE_GetJSType(joystick) == GE_JS_XONEPAD || !adapter->status)
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
      case C_TYPE_T300RS_PS4:
        //TODO MLA: decode and process T300RS FFB reports
        break;
      case C_TYPE_G27_PS3:
        if(adapter[id].hid_id >= 0)
        {
          ffb_logitech_process_report(id, data);
          adapter_send_next_hid_report(id);
        }
        break;
      case C_TYPE_G29_PS4:
        if(adapter[id].hid_id >= 0)
        {
          if(data[0] == 0x30)
          {
            ffb_logitech_process_report(id, data + 1);
            adapter_send_next_hid_report(id);
          }
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
          .weak = data[motors[adapter[id].ctype].weak] << 8,
          .strong = data[motors[adapter[id].ctype].strong] << 8
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

static int adapter_hid_write_cb(int id, int transfered)
{
  adapter[id].hid_busy = 0;
  if(transfered != -1) {
    ffb_logitech_ack(id);
  }
  adapter_send_next_hid_report(id);
  return 0;
}

static int adapter_hid_close_cb(int id)
{
  adapter[id].hid_id = -1;
  return 1;
}

#ifndef WIN32
static int adapter_hid_read_cb(int id, const void * buf, unsigned int count)
{
  if(adapter[id].uhid_id >= 0)
  {
    uhidasync_write(adapter[id].uhid_id, buf, count);
  }
  return 0;
}

static int start_hidasync(int id)
{
  if(hidasync_register(adapter[id].hid_id, id, adapter_hid_read_cb, adapter_hid_write_cb, adapter_hid_close_cb, REGISTER_FUNCTION) < 0)
  {
    return -1;
  }
  return 0;
}

void adapter_set_uhid_id(int controller, int uhid_id)
{
  if(controller < 0 || controller >= MAX_CONTROLLERS)
  {
    fprintf(stderr, "%s: invalid controller\n", __func__);
    return;
  }

  if(adapter[controller].uhid_id < 0)
  {
    adapter[controller].uhid_id = uhid_id;
    adapter[controller].hid_id = uhid_joystick_get_hid_id(uhid_id);
    start_hidasync(controller);
  }
}
#else
void adapter_set_usb_ids(int controller, unsigned short vendor, unsigned short product)
{
  if(controller < 0 || controller >= MAX_CONTROLLERS)
  {
    fprintf(stderr, "%s: invalid controller\n", __func__);
    return;
  }

  if(adapter_device[E_DEVICE_TYPE_JOYSTICK - 1][controller] >= 0)
  {
    adapter[controller].usb_ids.vendor = vendor;
    adapter[controller].usb_ids.product = product;
  }
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

  if(vendor != USB_VENDOR_ID_LOGITECH)
  {
    return 0;
  }
  unsigned int i;
  for(i = 0; i < sizeof(lg_wheel_products) / sizeof(*lg_wheel_products); ++i)
  {
    if(lg_wheel_products[i] == product)
    {
      return 1;
    }
  }
  return 0;
}

static int start_hid(int id)
{
  if(is_logitech_wheel(adapter[id].usb_ids.vendor, adapter[id].usb_ids.product))
  {
    adapter[id].hid_id = hidasync_open_ids(adapter[id].usb_ids.vendor, adapter[id].usb_ids.product);
    if(adapter[id].hid_id >= 0)
    {
      if(hidasync_register(adapter[id].hid_id, id, NULL, adapter_hid_write_cb, adapter_hid_close_cb, REGISTER_FUNCTION) < 0)
      {
        return -1;
      }
    }
  }
  return 0;
}
#endif

static int adapter_serial_read_cb(int id, const void * buf, unsigned int count)
{
  int ret = 0;
  
  if(adapter[id].bread + count < sizeof(s_packet)) {
    memcpy((unsigned char *)&adapter[id].packet + adapter[id].bread, buf, count);
    adapter[id].bread += count;
    unsigned int remaining;
    if(adapter[id].bread < sizeof(s_header))
    {
      remaining = sizeof(s_header) - adapter[id].bread;
    }
    else
    {
      remaining = adapter[id].packet.header.length - (adapter[id].bread - sizeof(s_header));
    }
    if(remaining == 0)
    {
      ret = adapter_process_packet(id, &adapter[id].packet);
      adapter[id].bread = 0;
      serialasync_set_read_size(adapter[id].serialdevice, sizeof(s_header));
    }
    else
    {
      serialasync_set_read_size(adapter[id].serialdevice, remaining);
    }
  }
  else
  {
    // this is a critical error (no possible recovering)
    fprintf(stderr, "%s:%d %s: invalid data size (count=%u, available=%zu)\n", __FILE__, __LINE__, __func__, count, sizeof(s_packet) - adapter[id].bread);
    return -1;
  }
  return ret;
}

static int adapter_serial_write_cb(int id, int transfered)
{
  //TODO MLA: anything to do in the serial write callback?
  return 0;
}

static int adapter_serial_close_cb(int id)
{
  //TODO MLA: anything to do in the serial close callback?
  return 0;
}

static int adapter_start_serialasync(int id)
{
  if(serialasync_set_read_size(adapter[id].serialdevice, sizeof(s_header)) < 0)
  {
    return -1;
  }
  if(serialasync_register(adapter[id].serialdevice, id, adapter_serial_read_cb, adapter_serial_write_cb, adapter_serial_close_cb, REGISTER_FUNCTION) < 0)
  {
    return -1;
  }
  return 0;
}

/*
 * This function should only be used in the initialization stages, i.e. before the mainloop.
 */
static int adapter_send_short_command(int device, unsigned char type)
{
  s_packet packet =
  {
    .header =
    {
      .type = type,
      .length = BYTE_LEN_0_BYTE
    }
  };

  int ret = serialasync_write_timeout(device, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
  if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
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
    ret = serialasync_read_timeout(device, &packet.header, sizeof(packet.header), SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < sizeof(packet.header))
    {
      fprintf(stderr, "can't read packet header\n");
      return -1;
    }

    ret = serialasync_read_timeout(device, &packet.value, packet.header.length, SERIAL_TIMEOUT);
    if(ret < 0 || (unsigned int)ret < packet.header.length)
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

static int adapter_send_reset(int device)
{
  s_header header =
  {
    .type = BYTE_RESET,
    .length = BYTE_LEN_0_BYTE
  };

  if(serialasync_write_timeout(device, &header, sizeof(header), SERIAL_TIMEOUT) != sizeof(header))
  {
    return -1;
  }

  return 0;
}

int adapter_detect()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      int rtype = gpp_connect(i, adapter->portname);
      if (rtype < 0)
      {
        fprintf(stderr, _("No controller detected.\n"));
        ret = -1;
      }
      else if(rtype < C_TYPE_MAX)
      {
        printf(_("Detected controller: %s.\n"), controller_get_name(rtype));
        adapter->ctype = rtype;
      }
      else
      {
        fprintf(stderr, _("Unknown GPP controller type.\n"));
        ret = -1;
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->portname)
      {
        adapter->serialdevice = serialasync_open(adapter->portname, BAUDRATE);
        if(adapter->serialdevice < 0)
        {
          fprintf(stderr, _("Check the wiring (maybe you swapped Rx and Tx?).\n"));
          ret = -1;
        }
        else
        {
          int rtype = adapter_send_short_command(i, BYTE_TYPE);

          if(rtype >= 0)
          {
            printf(_("Detected USB adapter: %s.\n"), controller_get_name(rtype));

            if(adapter->ctype == C_TYPE_NONE)
            {
              adapter->ctype = rtype;
            }
            else if(adapter->ctype != (e_controller_type) rtype)
            {
              fprintf(stderr, _("Wrong controller type.\n"));
              ret = -1;
            }

            int status = adapter_send_short_command(i, BYTE_STATUS);

            if(status < 0)
            {
              fprintf(stderr, _("Can't get adapter status.\n"));
              ret = -1;
            }

            if(ret != -1)
            {
              switch(adapter->ctype)
              {
                case C_TYPE_DS4:
                case C_TYPE_T300RS_PS4:
                case C_TYPE_G29_PS4:
                case C_TYPE_G27_PS3:
                  if(status == BYTE_STATUS_STARTED)
                  {
                    if(adapter_send_reset(i) < 0)
                    {
                      fprintf(stderr, _("Can't reset the adapter.\n"));
                      ret = -1;
                    }
                    else
                    {
                      printf(_("Reset sent to the adapter.\n"));
                      //Leave time for the adapter to reinitialize.
                      usleep(ADAPTER_RESET_TIME);
                    }
                  }
                  break;
                case C_TYPE_XONE_PAD:
                case C_TYPE_360_PAD:
                  if(status == BYTE_STATUS_SPOOFED)
                  {
                    adapter->status = 1;
                  }
                  else
                  {
                    if(adapter_send_reset(i) < 0)
                    {
                      fprintf(stderr, _("Can't reset the adapter.\n"));
                      ret = -1;
                    }
                    else
                    {
                      printf(_("Reset sent to the adapter.\n"));
                      //Leave time for the adapter to reinitialize.
                      usleep(ADAPTER_RESET_TIME);
                    }
                  }
                  break;
                case C_TYPE_SIXAXIS:
                  //TODO MLA: fix the EMUPS3 firmware and remove this!
                  gimx_params.force_updates = 0;
                  printf(_("Disable force updates to work-around #335.\n"));
                  break;
                default:
                  break;
              }
            }

            if(ret != -1)
            {
              int usb_res = usb_init(i, adapter->ctype);
              if(usb_res < 0)
              {
                if((adapter->ctype != C_TYPE_360_PAD
                    && adapter->ctype != C_TYPE_XONE_PAD)
                    || status != BYTE_STATUS_SPOOFED)
                {
                  fprintf(stderr, _("No controller was found on USB buses.\n"));
                  ret = -1;
                }
              }
            }

            if(ret != -1)
            {
              controller_init_report(adapter->ctype, &adapter->report[0].value);
            }
          }
        }
        if(adapter->ctype == C_TYPE_NONE)
        {
          fprintf(stderr, _("No controller detected.\n"));
          ret = -1;
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->dst_ip)
      {
        adapter->dst_fd = udp_connect(adapter->dst_ip, adapter->dst_port, (int *)&adapter->ctype);
        if(adapter->dst_fd < 0)
        {
          struct in_addr addr = { .s_addr = adapter->dst_ip };
          fprintf(stderr, _("Can't connect to: %s:%d.\n"), inet_ntoa(addr), adapter->dst_port);
          ret = -1;
        }
        else
        {
          printf(_("Detected controller: %s.\n"), controller_get_name(adapter->ctype));
        }
      }
    }
  }
  return ret;
}

static int adapter_gpp_read(int id, const void * buf, unsigned int count)
{
  GCAPI_REPORT * report = (GCAPI_REPORT *)(buf + 7);

  int joystick = adapter_get_device(E_DEVICE_TYPE_JOYSTICK, id);

  if(GE_JoystickHasRumble(joystick))
  {
    GE_Event event =
    {
      .jrumble =
      {
        .type = GE_JOYRUMBLE,
        .which = joystick,
        .weak = report->rumble[0] << 8,
        .strong = report->rumble[1] << 8
      }
    };
    GE_PushEvent(&event);
  }

  return 0;
}

static int adapter_gpp_write(int id, int transfered)
{
  //TODO MLA: anything to do in the serial write callback?
  return 0;
}

static int adapter_gpp_close(int id)
{
  //TODO MLA: anything to do in the serial close callback?
  return 0;
}

int adapter_start()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serialdevice >= 0)
      {
#ifdef WIN32
        switch(adapter->ctype)
        {
        case C_TYPE_G29_PS4:
        case C_TYPE_G27_PS3:
          start_hid(i);
          break;
        default:
          break;
        }
#endif
        if(adapter_send_short_command(i, BYTE_START) < 0)
        {
          fprintf(stderr, _("Can't start the adapter.\n"));
          ret = -1;
        }
        else if(adapter_start_serialasync(i) < 0)
        {
          fprintf(stderr, _("Can't start the serial asynchronous processing.\n"));
          ret = -1;
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bdaddr_dst)
      {
        if(adapter->ctype == C_TYPE_SIXAXIS)
        {
          if(sixaxis_connect(i, adapter->dongle_index, adapter->bdaddr_dst) < 0)
          {
            fprintf(stderr, _("Can't initialize sixaxis.\n"));
            ret = -1;
          }
        }
  #ifndef WIN32
        else if(adapter->ctype == C_TYPE_DS4)
        {
          if(btds4_init(i, adapter->dongle_index, adapter->bdaddr_dst) < 0)
          {
            fprintf(stderr, _("Can't initialize btds4.\n"));
            ret = -1;
          }
          controller_init_report(C_TYPE_DS4, &adapter->report[0].value);
        }
  #endif
        else
        {
          fprintf(stderr, _("Wrong controller type.\n"));
        }
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      ret = gpp_start_async(i, adapter_gpp_read, adapter_gpp_write, adapter_gpp_close, REGISTER_FUNCTION);
    }

    if(adapter->src_ip)
    {
      adapter->src_fd = udp_listen(adapter->src_ip, adapter->src_port);
      if(adapter->src_fd < 0)
      {
        struct in_addr addr = { .s_addr = adapter->src_ip };
        fprintf(stderr, _("Can't listen on: %s:%d.\n"), inet_ntoa(addr), adapter->src_port);
        ret = -1;
      }
      else
      {
        GE_AddSource(adapter->src_fd, i, adapter_network_read, NULL, adapter_network_close);
      }
    }
  }
  return ret;
}

int adapter_send()
{
  int ret = 0;
  int i;
  s_adapter* adapter;

  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);

    if(adapter->ctype == C_TYPE_NONE)
    {
      continue;
    }

    if (gimx_params.force_updates || adapter->send_command)
    {
      if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
      {
        if(adapter->dst_fd >= 0)
        {
          static unsigned char report[sizeof(adapter->axis)+2] = { BYTE_IN_REPORT, sizeof(adapter->axis) };
          memcpy(report+2, adapter->axis, sizeof(adapter->axis));
          ret = udp_send(adapter->dst_fd, report, sizeof(report));
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
      {
        if(adapter->serialdevice >= 0)
        {
          unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

          s_report_packet* report = adapter->report+index;

          switch(adapter->ctype)
          {
          case C_TYPE_SIXAXIS:
            ret = serialasync_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_DS4:
            report->value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = serialasync_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_T300RS_PS4:
          case C_TYPE_G29_PS4:
            report->length = DS4_USB_INTERRUPT_PACKET_SIZE;
            ret = serialasync_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            break;
          case C_TYPE_XONE_PAD:
            if(adapter->status)
            {
              ret = serialasync_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            }
            break;
          default:
            if(adapter->ctype != C_TYPE_PS2_PAD)
            {
              ret = serialasync_write(adapter->serialdevice, report, HEADER_SIZE+report->length);
            }
            else
            {
              ret = serialasync_write(adapter->serialdevice, &report->value.ds2, report->length);
            }
            break;
          }
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
      {
        if(adapter->bdaddr_dst)
        {
          unsigned int index = controller_build_report(adapter->ctype, adapter->axis, adapter->report);

          s_report_packet* report = adapter->report+index;

          switch(adapter->ctype)
          {
          case C_TYPE_SIXAXIS:
            ret = sixaxis_send_interrupt(i, &report->value.ds3);
            break;
#ifndef WIN32
          case C_TYPE_DS4:
            ret = btds4_send_interrupt(i, &report->value.ds4, adapter->send_command);
            break;
#endif
          default:
            break;
          }
        }
      }
      else if(adapter->atype == E_ADAPTER_TYPE_GPP)
      {
        ret = gpp_send(i, adapter->ctype, adapter->axis);
      }


      if(gimx_params.curses)
      {
        stats_update(i);
      }

      if (adapter->send_command)
      {
        if(gimx_params.status)
        {
          adapter_dump_state(i);
#ifdef WIN32
          //There is no setlinebuf(stdout) in windows.
          fflush(stdout);
#endif
        }
        if(gimx_params.curses)
        {
          display_run(adapter_get(0)->ctype, adapter_get(0)->axis);
        }

        adapter->send_command = 0;
      }

      if(adapter->ctype == C_TYPE_DS4)
      {
        adapter->axis[ds4a_finger1_x] = 0;
        adapter->axis[ds4a_finger1_y] = 0;
        adapter->axis[ds4a_finger2_x] = 0;
        adapter->axis[ds4a_finger2_y] = 0;
      }
    }
  }
  return ret;
}

void adapter_clean()
{
  int i;
  s_adapter* adapter;
  for(i=0; i<MAX_CONTROLLERS; ++i)
  {
    adapter = adapter_get(i);
    if(adapter->atype == E_ADAPTER_TYPE_REMOTE_GIMX)
    {
      if(adapter->dst_fd >= 0)
      {
        GE_RemoveSource(adapter->src_fd);
        udp_close(adapter->dst_fd);
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_BLUETOOTH)
    {
      if(adapter->bdaddr_dst)
      {
        if(adapter->ctype == C_TYPE_SIXAXIS)
        {
          sixaxis_close(i);
        }
#ifndef WIN32
        else if(adapter->ctype == C_TYPE_DS4)
        {
          btds4_close(i);
        }
#endif
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_DIY_USB)
    {
      if(adapter->serialdevice >= 0)
      {
        switch(adapter->ctype)
        {
          case C_TYPE_360_PAD:
          case C_TYPE_XONE_PAD:
            usb_close(i);
            break;
          case C_TYPE_DS4:
          case C_TYPE_T300RS_PS4:
          case C_TYPE_G29_PS4:
          case C_TYPE_G27_PS3:
            usb_close(i);
            adapter_send_reset(i);
            break;
          default:
            break;
        }
        serialasync_close(adapter->serialdevice);
      }
    }
    else if(adapter->atype == E_ADAPTER_TYPE_GPP)
    {
      gpp_disconnect(i);
    }
  }
}
