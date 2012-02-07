/*
 * gpp_con.c
 *
 *  Created on: 28 janv. 2012
 *      Author: matlo
 */

#include "gpp/pcprog.h"
#include "config.h"
#include "dump.h"

extern int display;

extern s_controller controller[MAX_CONTROLLERS];
extern struct sixaxis_state state[MAX_CONTROLLERS];

int gpp_connect()
{
  struct gppReport report;

  if(gppcprog_connect() != 1)
  {
    return -1;
  }

  while(gpppcprog_input(&report, 100) != 1) {}

  return 0;
}

void gpp_send(int force_update)
{
  char output[REP_IO_COUNT];
  int axis_value;
  int8 r;

  if (force_update || controller[0].send_command)
  {
    if(controller[0].send_command)
    {
      if (display)
      {
        sixaxis_dump_state(state, 0);
        //mmap_dump(state);
      }

      controller[0].send_command = 0;
    }

    memset(output, 0x00, REP_IO_COUNT);

    if (state[0].user.button[sb_up].pressed)
    {
      output[PS3_UP] = 100;
    }
    if (state[0].user.button[sb_down].pressed)
    {
      output[PS3_DOWN] = 100;
    }
    if (state[0].user.button[sb_left].pressed)
    {
      output[PS3_LEFT] = 100;
    }
    if (state[0].user.button[sb_right].pressed)
    {
      output[PS3_RIGHT] = 100;
    }

    if (state[0].user.button[sb_start].pressed)
    {
      output[PS3_START] = 100;
    }
    if (state[0].user.button[sb_select].pressed)
    {
      output[PS3_SELECT] = 100;
    }
    if (state[0].user.button[sb_l3].pressed)
    {
      output[PS3_L3] = 100;
    }
    if (state[0].user.button[sb_r3].pressed)
    {
      output[PS3_R3] = 100;
    }

    if (state[0].user.button[sb_l1].pressed)
    {
      output[PS3_L1] = 100;
    }
    if (state[0].user.button[sb_r1].pressed)
    {
      output[PS3_R1] = 100;
    }
    if (state[0].user.button[sb_ps].pressed)
    {
      output[PS3_PS] = 100;
    }

    if (state[0].user.button[sb_cross].pressed)
    {
      output[PS3_CROSS] = 100;
    }
    if (state[0].user.button[sb_circle].pressed)
    {
      output[PS3_CIRCLE] = 100;
    }
    if (state[0].user.button[sb_square].pressed)
    {
      output[PS3_SQUARE] = 100;
    }
    if (state[0].user.button[sb_triangle].pressed)
    {
      output[PS3_TRIANGLE] = 100;
    }

    if (state[0].user.button[sb_l2].pressed)
    {
      output[PS3_L2] = 100;
    }
    if (state[0].user.button[sb_r2].pressed)
    {
      output[PS3_R2] = 100;
    }

    axis_value = state[0].user.axis[0][0] * 100 / 127;
    output[PS3_LX] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[0][1] * 100 / 127;
    output[PS3_LY] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[1][0] * 100 / 127;
    output[PS3_RX] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[1][1] * 100 / 127;
    output[PS3_RY] = clamp(-100, axis_value, 100);

    gpppcprog_output(output);
  }
}

void gpp_disconnect()
{
  gppcprog_disconnect();
}
