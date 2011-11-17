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

static int debug;

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

/*
 * Send a usb report to the serial port.
 */
void serial_send(int force_update)
{
  s_report_data data =
  { };
  int i;
  struct timeval tv;
  uint8_t* pdata = (uint8_t*) &data;

  if (force_update || controller[0].send_command)
  {
    /*
     * Make sure the value is not out of range.
     */
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
    win_serial_send(&data);
#else
    lin_serial_send(&data);
#endif

    if(controller[0].send_command)
    {
      /* Dump contents */
      if (debug)
      {
        gettimeofday(&tv, NULL);
        printf("%ld.%06ld ", tv.tv_sec, tv.tv_usec);
        for (i = 0; i < sizeof(s_report_data); i++)
        {
          printf(" %02x", pdata[i]);
        }
        printf("\n");
      }

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
