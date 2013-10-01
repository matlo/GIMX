/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <report.h>
#include <usb_spoof.h>

#ifndef WIN32
#include <arpa/inet.h> //htons
#else
#include <winsock2.h> //htons
#endif

static unsigned int _360pad_report_build(s_report* report)
{
  int axis_value;
  
  s_report_360* report_360 = &report->value.x360;

  report->packet_type = BYTE_SEND_REPORT;
  report->value_len = sizeof(s_report_360);

  report_360->type = 0x00;
  report_360->size = 0x14;

  report_360->buttons = 0x0000;
  
  if (state[0].user.axis[sa_up])
  {
    report_360->buttons |= 0x0001;
  }
  if (state[0].user.axis[sa_down])
  {
    report_360->buttons |= 0x0002;
  }
  if (state[0].user.axis[sa_left])
  {
    report_360->buttons |= 0x0004;
  }
  if (state[0].user.axis[sa_right])
  {
    report_360->buttons |= 0x0008;
  }

  if (state[0].user.axis[sa_start])
  {
    report_360->buttons |= 0x0010;
  }
  if (state[0].user.axis[sa_select])
  {
    report_360->buttons |= 0x0020;
  }
  if (state[0].user.axis[sa_l3])
  {
    report_360->buttons |= 0x0040;
  }
  if (state[0].user.axis[sa_r3])
  {
    report_360->buttons |= 0x0080;
  }

  if (state[0].user.axis[sa_l1])
  {
    report_360->buttons |= 0x0100;
  }
  if (state[0].user.axis[sa_r1])
  {
    report_360->buttons |= 0x0200;
  }
  if (state[0].user.axis[sa_ps])
  {
    report_360->buttons |= 0x0400;
  }

  if (state[0].user.axis[sa_cross])
  {
    report_360->buttons |= 0x1000;
  }
  if (state[0].user.axis[sa_circle])
  {
    report_360->buttons |= 0x2000;
  }
  if (state[0].user.axis[sa_square])
  {
    report_360->buttons |= 0x4000;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report_360->buttons |= 0x8000;
  }

  report_360->ltrigger = clamp(0, state[0].user.axis[sa_l2], 255);
  report_360->rtrigger = clamp(0, state[0].user.axis[sa_r2], 255);

  axis_value = state[0].user.axis[sa_lstick_x];
  report_360->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_360->xaxis |= 0xFF;
  }
  axis_value = - state[0].user.axis[sa_lstick_y];
  report_360->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_360->yaxis |= 0xFF;
  }
  axis_value = state[0].user.axis[sa_rstick_x];
  report_360->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_360->zaxis |= 0xFF;
  }
  axis_value = -state[0].user.axis[sa_rstick_y];
  report_360->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_360->taxis |= 0xFF;
  }
  
  return sizeof(*report_360);
}

static unsigned int XboxPad_report_build(s_report* report)
{
  int axis_value;

  s_report_xbox* report_xbox = &report->value.xbox;

  report->packet_type = BYTE_SEND_REPORT;
  report->value_len = sizeof(*report_xbox);

  report_xbox->type = 0x00;
  report_xbox->size = 0x14;

  report_xbox->buttons = 0x00;

  if (state[0].user.axis[sa_up])
  {
    report_xbox->buttons |= 0x01;
  }
  if (state[0].user.axis[sa_down])
  {
    report_xbox->buttons |= 0x02;
  }
  if (state[0].user.axis[sa_left])
  {
    report_xbox->buttons |= 0x04;
  }
  if (state[0].user.axis[sa_right])
  {
    report_xbox->buttons |= 0x08;
  }

  if (state[0].user.axis[sa_start])
  {
    report_xbox->buttons |= 0x10;
  }
  if (state[0].user.axis[sa_select])
  {
    report_xbox->buttons |= 0x20;
  }
  if (state[0].user.axis[sa_l3])
  {
    report_xbox->buttons |= 0x40;
  }
  if (state[0].user.axis[sa_r3])
  {
    report_xbox->buttons |= 0x80;
  }

  report_xbox->ltrigger = clamp(0, state[0].user.axis[sa_l2], 255);
  report_xbox->rtrigger = clamp(0, state[0].user.axis[sa_r2], 255);
  report_xbox->btnA = clamp(0, state[0].user.axis[sa_cross], 255);
  report_xbox->btnB = clamp(0, state[0].user.axis[sa_circle], 255);
  report_xbox->btnX = clamp(0, state[0].user.axis[sa_square], 255);
  report_xbox->btnY = clamp(0, state[0].user.axis[sa_triangle], 255);
  report_xbox->btnWhite = clamp(0, state[0].user.axis[sa_l1], 255);
  report_xbox->btnBlack = clamp(0, state[0].user.axis[sa_r1], 255);

  axis_value = state[0].user.axis[sa_lstick_x];
  report_xbox->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->xaxis |= 0xFF;
  }
  axis_value = - state[0].user.axis[sa_lstick_y];
  report_xbox->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->yaxis |= 0xFF;
  }
  axis_value = state[0].user.axis[sa_rstick_x];
  report_xbox->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->zaxis |= 0xFF;
  }
  axis_value = -state[0].user.axis[sa_rstick_y];
  report_xbox->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report_xbox->taxis |= 0xFF;
  }

  return sizeof(*report_xbox);
}

static unsigned int sixaxis_report_build(s_report* report)
{
  unsigned char* buf = report->value.sixaxis;

  int i;
  unsigned char tmp[49] =
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
  
  memcpy(buf, tmp, sizeof(buf));

  for (i = 0; i < 17; i++) {
      int byte = 2 + (i / 8);
      int offset = i % 8;
      if (state[0].user.axis[digital_order[i]])
          buf[byte] |= (1 << offset);
  }

  buf[6] = clamp(0, state[0].user.axis[sa_lstick_x] + 128, 255);
  buf[7] = clamp(0, state[0].user.axis[sa_lstick_y] + 128, 255);
  buf[8] = clamp(0, state[0].user.axis[sa_rstick_x] + 128, 255);
  buf[9] = clamp(0, state[0].user.axis[sa_rstick_y] + 128, 255);

  for (i = 0; i < 12; i++)
      buf[14 + i] = state[0].user.axis[analog_order[i]];

  *(uint16_t *)&buf[41] = htons(clamp(0, state[0].user.axis[sa_acc_x] + 512, 1023));
  *(uint16_t *)&buf[43] = htons(clamp(0, state[0].user.axis[sa_acc_y] + 512, 1023));
  *(uint16_t *)&buf[45] = htons(clamp(0, state[0].user.axis[sa_acc_z] + 512, 1023));
  *(uint16_t *)&buf[47] = htons(clamp(0, state[0].user.axis[sa_gyro] + 512, 1023));
  
  return sizeof(report->value.sixaxis);
}

static unsigned int joystick_report_build(s_report* report)
{
  s_report_joystick* report_js = &report->value.js;

  report->packet_type = BYTE_SEND_REPORT;
  report->value_len = sizeof(*report_js);

  report_js->X = clamp(0, state[0].user.axis[sa_lstick_x] + 32768, 65535);
  report_js->Y = clamp(0, state[0].user.axis[sa_lstick_y] + 32768, 65535);
  report_js->Z = clamp(0, state[0].user.axis[sa_rstick_x] + 32768, 65535);
  report_js->Rz = clamp(0, state[0].user.axis[sa_rstick_y] + 32768, 65535);

  report_js->Bt = 0x0000;
  
  if (state[0].user.axis[sa_square])
  {
    report_js->Bt |= 0x0001;
  }
  if (state[0].user.axis[sa_cross])
  {
    report_js->Bt |= 0x0002;
  }
  if (state[0].user.axis[sa_circle])
  {
    report_js->Bt |= 0x0004;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report_js->Bt |= 0x0008;
  }

  if (state[0].user.axis[sa_select])
  {
    report_js->Bt |= 0x0100;
  }
  if (state[0].user.axis[sa_start])
  {
    report_js->Bt |= 0x0200;
  }
  if (state[0].user.axis[sa_l3])
  {
    report_js->Bt |= 0x0400;
  }
  if (state[0].user.axis[sa_r3])
  {
    report_js->Bt |= 0x0800;
  }

  if (state[0].user.axis[sa_l1])
  {
    report_js->Bt |= 0x0010;
  }
  if (state[0].user.axis[sa_r1])
  {
    report_js->Bt |= 0x0020;
  }
  if (state[0].user.axis[sa_l2])
  {
    report_js->Bt |= 0x0040;
  }
  if (state[0].user.axis[sa_r2])
  {
    report_js->Bt |= 0x0080;
  }

  if (state[0].user.axis[sa_ps])
  {
    report_js->Bt |= 0x1000;
  }

  if (state[0].user.axis[sa_right])
  {
    if (state[0].user.axis[sa_down])
    {
      report_js->Hat = 0x0003;
    }
    else if (state[0].user.axis[sa_up])
    {
      report_js->Hat = 0x0001;
    }
    else
    {
      report_js->Hat = 0x0002;
    }
  }
  else if (state[0].user.axis[sa_left])
  {
    if (state[0].user.axis[sa_down])
    {
      report_js->Hat = 0x0005;
    }
    else if (state[0].user.axis[sa_up])
    {
      report_js->Hat = 0x0007;
    }
    else
    {
      report_js->Hat = 0x0006;
    }
  }
  else if (state[0].user.axis[sa_down])
  {
    report_js->Hat = 0x0004;
  }
  else if (state[0].user.axis[sa_up])
  {
    report_js->Hat = 0x0000;
  }
  else
  {
    report_js->Hat = 0x0008;
  }

  return sizeof(*report_js);
}

static unsigned int ps2_report_build(s_report* report)
{
  s_report_ps2* report_ps2 = &report->value.ps2;

  report->packet_type = BYTE_SEND_REPORT;
  report->value_len = sizeof(*report_ps2);

  report_ps2->head = 0x5A;
  report_ps2->Bt1 = 0xFF;
  report_ps2->Bt2 = 0xFF;

  report_ps2->X = clamp(0, state[0].user.axis[sa_lstick_x] + 128, 255);
  report_ps2->Y = clamp(0, state[0].user.axis[sa_lstick_y] + 128, 255);
  report_ps2->Z = clamp(0, state[0].user.axis[sa_rstick_x] + 128, 255);
  report_ps2->Rz = clamp(0, state[0].user.axis[sa_rstick_y] + 128, 255);

  if (state[0].user.axis[sa_square])
  {
    report_ps2->Bt2 &= ~0x80;
  }
  if (state[0].user.axis[sa_cross])
  {
    report_ps2->Bt2 &= ~0x40;
  }
  if (state[0].user.axis[sa_circle])
  {
    report_ps2->Bt2 &= ~0x20;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report_ps2->Bt2 &= ~0x10;
  }

  if (state[0].user.axis[sa_select])
  {
    report_ps2->Bt1 &= ~0x01;
  }
  if (state[0].user.axis[sa_start])
  {
    report_ps2->Bt1 &= ~0x08;
  }
  if (state[0].user.axis[sa_l3])
  {
    report_ps2->Bt1 &= ~0x02;
  }
  if (state[0].user.axis[sa_r3])
  {
    report_ps2->Bt1 &= ~0x04;
  }

  if (state[0].user.axis[sa_l1])
  {
    report_ps2->Bt2 &= ~0x04;
  }
  if (state[0].user.axis[sa_r1])
  {
    report_ps2->Bt2 &= ~0x08;
  }
  if (state[0].user.axis[sa_l2])
  {
    report_ps2->Bt2 &= ~0x01;
  }
  if (state[0].user.axis[sa_r2])
  {
    report_ps2->Bt2 &= ~0x02;
  }

  if (state[0].user.axis[sa_up])
  {
    report_ps2->Bt1 &= ~0x10;
  }
  if (state[0].user.axis[sa_right])
  {
    report_ps2->Bt1 &= ~0x20;
  }
  if (state[0].user.axis[sa_down])
  {
    report_ps2->Bt1 &= ~0x40;
  }
  if (state[0].user.axis[sa_left])
  {
    report_ps2->Bt1 &= ~0x80;
  }

  return sizeof(*report_ps2);
}

unsigned int (*func_ptr[C_TYPE_MAX])(s_report* report) =
{
    [C_TYPE_JOYSTICK] = joystick_report_build,
    [C_TYPE_360_PAD] = _360pad_report_build,
    [C_TYPE_XBOX_PAD] = XboxPad_report_build,
    [C_TYPE_SIXAXIS] = sixaxis_report_build,
    [C_TYPE_PS2_PAD] = ps2_report_build,
};

unsigned int report_build(s_report* report, e_controller_type type)
{
  unsigned int ret = 0;
  if(func_ptr[type])
  {
    ret = func_ptr[type](report);
  }
  return ret;
}
