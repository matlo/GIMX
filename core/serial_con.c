/*
 * win_serial.c
 *
 *  Created on: 28 mai 2011
 *      Author: Matlo
 *
 *  License: GPLv3
 */

#include "win_serial_con.h"
#include "lin_serial_con.h"
#include "serial_con.h"
#include <sys/time.h>
#include <stdio.h>
#include "config.h"
#include "dump.h"

/*
 * These defines are exclusive.
 */
#define JS 1
#define PS2 0
#define PS3 0
#define X360 0

/*
 * Imported from emuclient.c
 */
extern int max_axis_value;
extern int mean_axis_value;

extern s_controller controller[MAX_CONTROLLERS];
extern struct sixaxis_state state[MAX_CONTROLLERS];

extern int display;

/*
 * Connect to a serial port.
 */
int serial_connect(char* portname)
{
#ifdef WIN32
  return win_serial_connect(portname);
#else
  return lin_serial_connect(portname);
#endif
}

static void _360pad_serial_send()
{
  s_report_360 report =
  { .type = 0x00, .size = 0x14 };
  int axis_value;

  if (state[0].user.button[sb_up].pressed)
  {
    report.buttons |= 0x0001;
  }
  if (state[0].user.button[sb_down].pressed)
  {
    report.buttons |= 0x0002;
  }
  if (state[0].user.button[sb_left].pressed)
  {
    report.buttons |= 0x0004;
  }
  if (state[0].user.button[sb_right].pressed)
  {
    report.buttons |= 0x0008;
  }

  if (state[0].user.button[sb_start].pressed)
  {
    report.buttons |= 0x0010;
  }
  if (state[0].user.button[sb_select].pressed)
  {
    report.buttons |= 0x0020;
  }
  if (state[0].user.button[sb_l3].pressed)
  {
    report.buttons |= 0x0040;
  }
  if (state[0].user.button[sb_r3].pressed)
  {
    report.buttons |= 0x0080;
  }

  if (state[0].user.button[sb_l1].pressed)
  {
    report.buttons |= 0x0100;
  }
  if (state[0].user.button[sb_r1].pressed)
  {
    report.buttons |= 0x0200;
  }
  if (state[0].user.button[sb_ps].pressed)
  {
    report.buttons |= 0x0400;
  }

  if (state[0].user.button[sb_cross].pressed)
  {
    report.buttons |= 0x1000;
  }
  if (state[0].user.button[sb_circle].pressed)
  {
    report.buttons |= 0x2000;
  }
  if (state[0].user.button[sb_square].pressed)
  {
    report.buttons |= 0x4000;
  }
  if (state[0].user.button[sb_triangle].pressed)
  {
    report.buttons |= 0x8000;
  }

  if (state[0].user.button[sb_l2].pressed)
  {
    report.ltrigger = 0xFF;
  }
  if (state[0].user.button[sb_r2].pressed)
  {
    report.rtrigger = 0xFF;
  }

  axis_value = state[0].user.axis[0][0];
  report.xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report.xaxis |= 0xFF;
  }
  axis_value = - state[0].user.axis[0][1];
  report.yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report.yaxis |= 0xFF;
  }
  axis_value = state[0].user.axis[1][0];
  report.zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report.zaxis |= 0xFF;
  }
  axis_value = -state[0].user.axis[1][1];
  report.taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report.taxis |= 0xFF;
  }

#ifdef WIN32
    win_serial_send(&report, sizeof(report));
#else
    lin_serial_send(&report, sizeof(report));
#endif
}

extern const int digital_order[17];
extern const int analog_order[12];

static void sixaxis_serial_send()
{
  int i;
  unsigned char buf[50] =
  {
      0x01,0x00,0x00,0x00,
      0x00, //0x01 = PS3 button
      0x00,0x7a,0x80,0x82,
      0x7d,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,
      0x02,0xee,0x10,0x00,
      0x00,0x00,0x00,0x02,
      0xff,0x77,0x01,0x80,
      0x02,0x1f,0x02,0x02,
      0x01,0x9c,0x00,0x05
  };

  for (i = 0; i < 17; i++) {
      int byte = 2 + (i / 8);
      int offset = i % 8;
      if (state[0].user.button[digital_order[i]].pressed)
          buf[byte] |= (1 << offset);
  }

  buf[6] = clamp(0, state[0].user.axis[0][0] + mean_axis_value, max_axis_value);
  buf[7] = clamp(0, state[0].user.axis[0][1] + mean_axis_value, max_axis_value);
  buf[8] = clamp(0, state[0].user.axis[1][0] + mean_axis_value, max_axis_value);
  buf[9] = clamp(0, state[0].user.axis[1][1] + mean_axis_value, max_axis_value);

  for (i = 0; i < 12; i++)
      buf[14 + i] = state[0].user.button[analog_order[i]].value;

#ifdef WIN32
  win_serial_send(buf, sizeof(buf));
#else
  lin_serial_send(buf, sizeof(buf));
#endif
}

static void joystick_serial_send()
{
  s_report_data data =
  { };

  data.X = clamp(0, state[0].user.axis[0][0] + mean_axis_value, max_axis_value);
  data.Y = clamp(0, state[0].user.axis[0][1] + mean_axis_value, max_axis_value);
  data.Z = clamp(0, state[0].user.axis[1][0] + mean_axis_value, max_axis_value);
  data.Rz = clamp(0, state[0].user.axis[1][1] + mean_axis_value, max_axis_value);

  if (state[0].user.button[sb_square].pressed)
  {
    data.Bt |= 0x0001;
  }
  if (state[0].user.button[sb_cross].pressed)
  {
    data.Bt |= 0x0002;
  }
  if (state[0].user.button[sb_circle].pressed)
  {
    data.Bt |= 0x0004;
  }
  if (state[0].user.button[sb_triangle].pressed)
  {
    data.Bt |= 0x0008;
  }

  if (state[0].user.button[sb_select].pressed)
  {
    data.Bt |= 0x0100;
  }
  if (state[0].user.button[sb_start].pressed)
  {
    data.Bt |= 0x0200;
  }
  if (state[0].user.button[sb_l3].pressed)
  {
    data.Bt |= 0x0400;
  }
  if (state[0].user.button[sb_r3].pressed)
  {
    data.Bt |= 0x0800;
  }

  if (state[0].user.button[sb_l1].pressed)
  {
    data.Bt |= 0x0010;
  }
  if (state[0].user.button[sb_r1].pressed)
  {
    data.Bt |= 0x0020;
  }
  if (state[0].user.button[sb_l2].pressed)
  {
    data.Bt |= 0x0040;
  }
  if (state[0].user.button[sb_r2].pressed)
  {
    data.Bt |= 0x0080;
  }

  if (state[0].user.button[sb_right].pressed)
  {
    if (state[0].user.button[sb_down].pressed)
    {
      data.Hat = 0x0003;
    }
    else if (state[0].user.button[sb_up].pressed)
    {
      data.Hat = 0x0001;
    }
    else
    {
      data.Hat = 0x0002;
    }
  }
  else if (state[0].user.button[sb_left].pressed)
  {
    if (state[0].user.button[sb_down].pressed)
    {
      data.Hat = 0x0005;
    }
    else if (state[0].user.button[sb_up].pressed)
    {
      data.Hat = 0x0007;
    }
    else
    {
      data.Hat = 0x0006;
    }
  }
  else if (state[0].user.button[sb_down].pressed)
  {
    data.Hat = 0x0004;
  }
  else if (state[0].user.button[sb_up].pressed)
  {
    data.Hat = 0x0000;
  }
  else
  {
    data.Hat = 0x0008;
  }

#ifdef WIN32
  win_serial_send(&data, sizeof(data));
#else
  lin_serial_send(&data, sizeof(data));
#endif
}

static void ps2_serial_send()
{
  s_report_data2 data =
  { .head = 0x5A, .Bt1 = 0xFF, .Bt2 = 0xFF };

  data.X = clamp(0, state[0].user.axis[0][0] + mean_axis_value, max_axis_value);
  data.Y = clamp(0, state[0].user.axis[0][1] + mean_axis_value, max_axis_value);
  data.Z = clamp(0, state[0].user.axis[1][0] + mean_axis_value, max_axis_value);
  data.Rz = clamp(0, state[0].user.axis[1][1] + mean_axis_value, max_axis_value);

  if (state[0].user.button[sb_square].pressed)
  {
    data.Bt2 &= ~0x80;
  }
  if (state[0].user.button[sb_cross].pressed)
  {
    data.Bt2 &= ~0x40;
  }
  if (state[0].user.button[sb_circle].pressed)
  {
    data.Bt2 &= ~0x20;
  }
  if (state[0].user.button[sb_triangle].pressed)
  {
    data.Bt2 &= ~0x10;
  }

  if (state[0].user.button[sb_select].pressed)
  {
    data.Bt1 &= ~0x01;
  }
  if (state[0].user.button[sb_start].pressed)
  {
    data.Bt1 &= ~0x08;
  }
  if (state[0].user.button[sb_l3].pressed)
  {
    data.Bt1 &= ~0x02;
  }
  if (state[0].user.button[sb_r3].pressed)
  {
    data.Bt1 &= ~0x04;
  }

  if (state[0].user.button[sb_l1].pressed)
  {
    data.Bt2 &= ~0x04;
  }
  if (state[0].user.button[sb_r1].pressed)
  {
    data.Bt2 &= ~0x08;
  }
  if (state[0].user.button[sb_l2].pressed)
  {
    data.Bt2 &= ~0x01;
  }
  if (state[0].user.button[sb_r2].pressed)
  {
    data.Bt2 &= ~0x02;
  }

  if (state[0].user.button[sb_up].pressed)
  {
    data.Bt1 &= ~0x10;
  }
  if (state[0].user.button[sb_right].pressed)
  {
    data.Bt1 &= ~0x20;
  }
  if (state[0].user.button[sb_down].pressed)
  {
    data.Bt1 &= ~0x40;
  }
  if (state[0].user.button[sb_left].pressed)
  {
    data.Bt1 &= ~0x80;
  }

#ifdef WIN32
    win_serial_send(&data, sizeof(data));
#else
    lin_serial_send(&data, sizeof(data));
#endif

}

/*
 * Send a usb report to the serial port.
 */
void serial_send(e_controller_type ctype, int force_update)
{
  if (force_update || controller[0].send_command)
  {
    switch(ctype)
    {
      case C_TYPE_JOYSTICK:
        joystick_serial_send();
        break;
      case C_TYPE_360_PAD:
        _360pad_serial_send();
        break;
      case C_TYPE_SIXAXIS:
        sixaxis_serial_send();
        break;
      case C_TYPE_PS2_PAD:
        ps2_serial_send();
        break;
      default:
        break;
    }

    if(controller[0].send_command)
    {
      if (display)
      {
        sixaxis_dump_state(state, 0);
        //mmap_dump(state);
      }

      controller[0].send_command = 0;
    }
  }
}

void serial_close()
{
#ifdef WIN32
    win_serial_close();
#else
    lin_serial_close();
#endif
}
