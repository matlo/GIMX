/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include "gpp/pcprog.h"
#include "config.h"
#include <controller2.h>

int gpp_connect(int id, const char* device)
{
  int ret = -1;
  GCAPI_REPORT report;

  if(gppcprog_connect(id, device) != 1)
  {
    return -1;
  }

  while(gpppcprog_input(id, &report, 100) != 1) {}
  
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

int gpp_send(int id, int axis[AXIS_MAX])
{
  int8_t output[GCAPI_INPUT_TOTAL] = {};
  int axis_value;
  int ret = 0;

  //TODO: normalize values vs the controller type...

  output[PS3_UP] = axis[sa_up] * 100 / 255;
  output[PS3_DOWN] = axis[sa_down] * 100 / 255;
  output[PS3_LEFT] = axis[sa_left] * 100 / 255;
  output[PS3_RIGHT] = axis[sa_right] * 100 / 255;
  output[PS3_START] = axis[sa_start] ? 100 : 0;
  output[PS3_SELECT] = axis[sa_select] ? 100 : 0;
  output[PS3_L3] = axis[sa_l3] * 100 / 255;
  output[PS3_R3] = axis[sa_r3] * 100 / 255;
  output[PS3_L1] = axis[sa_l1] * 100 / 255;
  output[PS3_R1] = axis[sa_r1] * 100 / 255;
  output[PS3_PS] = axis[sa_ps] ? 100 : 0;
  output[PS3_CROSS] = axis[sa_cross] * 100 / 255;
  output[PS3_CIRCLE] = axis[sa_circle] * 100 / 255;
  output[PS3_SQUARE] = axis[sa_square] * 100 / 255;
  output[PS3_TRIANGLE] = axis[sa_triangle] * 100 / 255;
  output[PS3_L2] = axis[sa_l2] * 100 / 255;
  output[PS3_R2] = axis[sa_r2] * 100 / 255;

  axis_value = axis[sa_lstick_x] * 100 / 127;
  output[PS3_LX] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_lstick_y] * 100 / 127;
  output[PS3_LY] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_rstick_x] * 100 / 127;
  output[PS3_RX] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_rstick_y] * 100 / 127;
  output[PS3_RY] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_acc_x] * 100 / 511;
  output[PS3_ACCX] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_acc_y] * 100 / 511;
  output[PS3_ACCY] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_acc_z] * 100 / 511;
  output[PS3_ACCZ] = clamp(-100, axis_value, 100);

  axis_value = axis[sa_gyro] * 100 / 511;
  output[PS3_GYRO] = clamp(-100, axis_value, 100);

  output[PS4_TOUCH] = axis[ds4a_finger1] ? 100 : 0;
  output[PS4_TOUCHX] = axis[ds4a_finger1_x] * 100 / 1919;
  output[PS4_TOUCHY] = axis[ds4a_finger1_y] * 100 / 919;

  if(!gpppcprog_output(id, output))
  {
    ret = -1;
  }

  return ret;
}

void gpp_disconnect(int id)
{
  gppcprog_disconnect(id);
}
