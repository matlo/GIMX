/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include "gpp/pcprog.h"
#include "config.h"
#include "dump.h"
#include "emuclient.h"

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

  if (force_update || controller[0].send_command)
  {
    if(controller[0].send_command)
    {
      if (emuclient_params.status)
      {
        sixaxis_dump_state(state, 0);
        //mmap_dump(state);
      }

      controller[0].send_command = 0;
    }

    memset(output, 0x00, REP_IO_COUNT);

    output[PS3_UP] = state[0].user.axis[sa_up] * 100 / 255;
    output[PS3_DOWN] = state[0].user.axis[sa_down] * 100 / 255;
    output[PS3_LEFT] = state[0].user.axis[sa_left] * 100 / 255;
    output[PS3_RIGHT] = state[0].user.axis[sa_right] * 100 / 255;
    output[PS3_START] = state[0].user.axis[sa_start] * 100 / 255;
    output[PS3_SELECT] = state[0].user.axis[sa_select] * 100 / 255;
    output[PS3_L3] = state[0].user.axis[sa_l3] * 100 / 255;
    output[PS3_R3] = state[0].user.axis[sa_r3] * 100 / 255;
    output[PS3_L1] = state[0].user.axis[sa_l1] * 100 / 255;
    output[PS3_R1] = state[0].user.axis[sa_r1] * 100 / 255;
    output[PS3_PS] = state[0].user.axis[sa_ps] * 100 / 255;
    output[PS3_CROSS] = state[0].user.axis[sa_cross] * 100 / 255;
    output[PS3_CIRCLE] = state[0].user.axis[sa_circle] * 100 / 255;
    output[PS3_SQUARE] = state[0].user.axis[sa_square] * 100 / 255;
    output[PS3_TRIANGLE] = state[0].user.axis[sa_triangle] * 100 / 255;
    output[PS3_L2] = state[0].user.axis[sa_l2] * 100 / 255;
    output[PS3_R2] = state[0].user.axis[sa_r2] * 100 / 255;
    
    axis_value = state[0].user.axis[sa_lstick_x] * 100 / 127;
    output[PS3_LX] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_lstick_y] * 100 / 127;
    output[PS3_LY] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_rstick_x] * 100 / 127;
    output[PS3_RX] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_rstick_y] * 100 / 127;
    output[PS3_RY] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_acc_x] * 100 / 511;
    output[PS3_ACCX] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_acc_y] * 100 / 511;
    output[PS3_ACCY] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_acc_z] * 100 / 511;
    output[PS3_ACCZ] = clamp(-100, axis_value, 100);

    axis_value = state[0].user.axis[sa_gyro] * 100 / 511;
    output[PS3_GYRO] = clamp(-100, axis_value, 100);

    gpppcprog_output(output);
  }
}

void gpp_disconnect()
{
  gppcprog_disconnect();
}
