/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <gimxgpp/pcprog.h>
#include <gimxcontroller/include/controller.h>
#include "config.h"
#include "gimx.h"

int gpp_connect(int id, const char* device)
{
  int ret = -1;
  GCAPI_REPORT report;

  if(gppcprog_connect(id, device) != 1)
  {
    return -1;
  }

  if(gpppcprog_input(id, &report, 1000) != 1)
  {
      gppcprog_disconnect(id);
      return -1;
  }
  
  switch(report.console)
  {
    case CONSOLE_DISCONNECTED:
      ret = -1;
      break;
    case CONSOLE_PS3:
      ret = C_TYPE_SIXAXIS;
      break;
    case CONSOLE_XB360:
      ret = C_TYPE_360_PAD;
      break;
    case CONSOLE_PS4:
      ret = C_TYPE_DS4;
      break;
    case CONSOLE_XB1:
      ret = C_TYPE_XONE_PAD;
      break;
  }

  return ret;
}

static inline int scale_axis(e_controller_type type, int index, int axis[AXIS_MAX])
{
  return axis[index] * 100 / controller_get_max_signed(type, index);
}

int gpp_send(int id, e_controller_type type, int axis[AXIS_MAX])
{
  static int8_t output[MAX_CONTROLLERS][GCAPI_INPUT_TOTAL] = {};
  int axis_value;
  int ret = 0;

  output[id][PS3_UP] = scale_axis(type, sa_up, axis);
  output[id][PS3_DOWN] = scale_axis(type, sa_down, axis);
  output[id][PS3_LEFT] = scale_axis(type, sa_left, axis);
  output[id][PS3_RIGHT] = scale_axis(type, sa_right, axis);
  output[id][PS3_START] = axis[sa_start] ? 100 : 0;
  output[id][PS3_SELECT] = axis[sa_select] ? 100 : 0;
  output[id][PS3_L3] = scale_axis(type, sa_l3, axis);
  output[id][PS3_R3] = scale_axis(type, sa_r3, axis);
  output[id][PS3_L1] = scale_axis(type, sa_l1, axis);
  output[id][PS3_R1] = scale_axis(type, sa_r1, axis);
  output[id][PS3_PS] = axis[sa_ps] ? 100 : 0;
  output[id][PS3_CROSS] = scale_axis(type, sa_cross, axis);
  output[id][PS3_CIRCLE] = scale_axis(type, sa_circle, axis);
  output[id][PS3_SQUARE] = scale_axis(type, sa_square, axis);
  output[id][PS3_TRIANGLE] = scale_axis(type, sa_triangle, axis);
  output[id][PS3_L2] = scale_axis(type, sa_l2, axis);
  output[id][PS3_R2] = scale_axis(type, sa_r2, axis);

  axis_value = scale_axis(type, sa_lstick_x, axis);
  output[id][PS3_LX] = clamp(-100, axis_value, 100);

  axis_value = scale_axis(type, sa_lstick_y, axis);
  output[id][PS3_LY] = clamp(-100, axis_value, 100);

  axis_value = scale_axis(type, sa_rstick_x, axis);
  output[id][PS3_RX] = clamp(-100, axis_value, 100);

  axis_value = scale_axis(type, sa_rstick_y, axis);
  output[id][PS3_RY] = clamp(-100, axis_value, 100);

  if(type == C_TYPE_SIXAXIS)
  {
    axis_value = scale_axis(type, sa_acc_x, axis);
    output[id][PS3_ACCX] = clamp(-100, axis_value, 100);

    axis_value = scale_axis(type, sa_acc_y, axis);
    output[id][PS3_ACCY] = clamp(-100, axis_value, 100);

    axis_value = scale_axis(type, sa_acc_z, axis);
    output[id][PS3_ACCZ] = clamp(-100, axis_value, 100);

    axis_value = scale_axis(type, sa_gyro, axis);
    output[id][PS3_GYRO] = clamp(-100, axis_value, 100);
  }
  else if(type == C_TYPE_DS4)
  {
    output[id][PS4_TOUCH] = axis[ds4a_touchpad] ? 100 : 0;

    if(axis[ds4a_finger1])
    {
      axis_value = output[id][PS4_TOUCHX] + scale_axis(type, ds4a_finger1_x, axis);
      output[id][PS4_TOUCHX] = clamp(-100, axis_value, 100);

      axis_value = output[id][PS4_TOUCHY] + scale_axis(type, ds4a_finger1_y, axis);
      output[id][PS4_TOUCHY] = clamp(-100, axis_value, 100);
    }
    else
    {
      output[id][PS4_TOUCHX] = 101;
      output[id][PS4_TOUCHY] = 101;
    }
  }

  int res = gpppcprog_output(id, output[id]);
  if(res < 0)
  {
    ret = -1;
  }
  else if (res == 0)
  {
    gwarn("device is busy\n");
  }

  return ret;
}

int8_t gpp_start_async(int id, const GHID_CALLBACKS * callbacks)
{
  return gpppcprog_start_async(id, callbacks);
}

void gpp_disconnect(int id)
{
  gppcprog_disconnect(id);
}
