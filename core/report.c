/*
 Copyright (c) 2010 Mathieu Laurendeau
 License: GPLv3
 */

#include <string.h>
#include <report.h>

#ifndef WIN32
#include <arpa/inet.h> //htons
#else
#include <winsock2.h> //htons
#endif

static unsigned int _360pad_report_build(s_report_360* report)
{
  int axis_value;
  
  report->type = 0x00;
  report->size = 0x14;

  report->buttons = 0x0000;
  
  if (state[0].user.axis[sa_up])
  {
    report->buttons |= 0x0001;
  }
  if (state[0].user.axis[sa_down])
  {
    report->buttons |= 0x0002;
  }
  if (state[0].user.axis[sa_left])
  {
    report->buttons |= 0x0004;
  }
  if (state[0].user.axis[sa_right])
  {
    report->buttons |= 0x0008;
  }

  if (state[0].user.axis[sa_start])
  {
    report->buttons |= 0x0010;
  }
  if (state[0].user.axis[sa_select])
  {
    report->buttons |= 0x0020;
  }
  if (state[0].user.axis[sa_l3])
  {
    report->buttons |= 0x0040;
  }
  if (state[0].user.axis[sa_r3])
  {
    report->buttons |= 0x0080;
  }

  if (state[0].user.axis[sa_l1])
  {
    report->buttons |= 0x0100;
  }
  if (state[0].user.axis[sa_r1])
  {
    report->buttons |= 0x0200;
  }
  if (state[0].user.axis[sa_ps])
  {
    report->buttons |= 0x0400;
  }

  if (state[0].user.axis[sa_cross])
  {
    report->buttons |= 0x1000;
  }
  if (state[0].user.axis[sa_circle])
  {
    report->buttons |= 0x2000;
  }
  if (state[0].user.axis[sa_square])
  {
    report->buttons |= 0x4000;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report->buttons |= 0x8000;
  }

  if (state[0].user.axis[sa_l2])
  {
    report->ltrigger = 0xFF;
  }
  if (state[0].user.axis[sa_r2])
  {
    report->rtrigger = 0xFF;
  }

  axis_value = state[0].user.axis[sa_lstick_x];
  report->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->xaxis |= 0xFF;
  }
  axis_value = - state[0].user.axis[sa_lstick_y];
  report->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->yaxis |= 0xFF;
  }
  axis_value = state[0].user.axis[sa_rstick_x];
  report->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->zaxis |= 0xFF;
  }
  axis_value = -state[0].user.axis[sa_rstick_y];
  report->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->taxis |= 0xFF;
  }
  
  return sizeof(*report);
}

static unsigned int XboxPad_report_build(s_report_xbox* report)
{
  int axis_value;

  report->type = 0x00;
  report->size = 0x14;

  report->buttons = 0x00;

  if (state[0].user.axis[sa_up])
  {
    report->buttons |= 0x01;
  }
  if (state[0].user.axis[sa_down])
  {
    report->buttons |= 0x02;
  }
  if (state[0].user.axis[sa_left])
  {
    report->buttons |= 0x04;
  }
  if (state[0].user.axis[sa_right])
  {
    report->buttons |= 0x08;
  }

  if (state[0].user.axis[sa_start])
  {
    report->buttons |= 0x10;
  }
  if (state[0].user.axis[sa_select])
  {
    report->buttons |= 0x20;
  }
  if (state[0].user.axis[sa_l3])
  {
    report->buttons |= 0x40;
  }
  if (state[0].user.axis[sa_r3])
  {
    report->buttons |= 0x80;
  }

  report->ltrigger = clamp(0, state[0].user.axis[sa_l2], 255);
  report->rtrigger = clamp(0, state[0].user.axis[sa_r2], 255);
  report->btnA = clamp(0, state[0].user.axis[sa_cross], 255);
  report->btnB = clamp(0, state[0].user.axis[sa_circle], 255);
  report->btnX = clamp(0, state[0].user.axis[sa_square], 255);
  report->btnY = clamp(0, state[0].user.axis[sa_triangle], 255);
  report->btnWhite = clamp(0, state[0].user.axis[sa_l1], 255);
  report->btnBlack = clamp(0, state[0].user.axis[sa_r1], 255);

  axis_value = state[0].user.axis[sa_lstick_x];
  report->xaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->xaxis |= 0xFF;
  }
  axis_value = - state[0].user.axis[sa_lstick_y];
  report->yaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->yaxis |= 0xFF;
  }
  axis_value = state[0].user.axis[sa_rstick_x];
  report->zaxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->zaxis |= 0xFF;
  }
  axis_value = -state[0].user.axis[sa_rstick_y];
  report->taxis = clamp(-128, axis_value, 127) << 8;
  if(axis_value > 127)
  {
    report->taxis |= 0xFF;
  }

  return sizeof(*report);
}

static unsigned int sixaxis_report_build(unsigned char buf[49])
{
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
  
  return sizeof(tmp);
}

static unsigned int joystick_report_build(s_report_joystick* report)
{
  report->X = clamp(0, state[0].user.axis[sa_lstick_x] + 32768, 65535);
  report->Y = clamp(0, state[0].user.axis[sa_lstick_y] + 32768, 65535);
  report->Z = clamp(0, state[0].user.axis[sa_rstick_x] + 32768, 65535);
  report->Rz = clamp(0, state[0].user.axis[sa_rstick_y] + 32768, 65535);

  report->Bt = 0x0000;
  
  if (state[0].user.axis[sa_square])
  {
    report->Bt |= 0x0001;
  }
  if (state[0].user.axis[sa_cross])
  {
    report->Bt |= 0x0002;
  }
  if (state[0].user.axis[sa_circle])
  {
    report->Bt |= 0x0004;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report->Bt |= 0x0008;
  }

  if (state[0].user.axis[sa_select])
  {
    report->Bt |= 0x0100;
  }
  if (state[0].user.axis[sa_start])
  {
    report->Bt |= 0x0200;
  }
  if (state[0].user.axis[sa_l3])
  {
    report->Bt |= 0x0400;
  }
  if (state[0].user.axis[sa_r3])
  {
    report->Bt |= 0x0800;
  }

  if (state[0].user.axis[sa_l1])
  {
    report->Bt |= 0x0010;
  }
  if (state[0].user.axis[sa_r1])
  {
    report->Bt |= 0x0020;
  }
  if (state[0].user.axis[sa_l2])
  {
    report->Bt |= 0x0040;
  }
  if (state[0].user.axis[sa_r2])
  {
    report->Bt |= 0x0080;
  }

  if (state[0].user.axis[sa_right])
  {
    if (state[0].user.axis[sa_down])
    {
      report->Hat = 0x0003;
    }
    else if (state[0].user.axis[sa_up])
    {
      report->Hat = 0x0001;
    }
    else
    {
      report->Hat = 0x0002;
    }
  }
  else if (state[0].user.axis[sa_left])
  {
    if (state[0].user.axis[sa_down])
    {
      report->Hat = 0x0005;
    }
    else if (state[0].user.axis[sa_up])
    {
      report->Hat = 0x0007;
    }
    else
    {
      report->Hat = 0x0006;
    }
  }
  else if (state[0].user.axis[sa_down])
  {
    report->Hat = 0x0004;
  }
  else if (state[0].user.axis[sa_up])
  {
    report->Hat = 0x0000;
  }
  else
  {
    report->Hat = 0x0008;
  }

  return sizeof(*report);
}

static unsigned int ps2_report_build(s_report_ps2* report)
{
  report->head = 0x5A;
  report->Bt1 = 0xFF;
  report->Bt2 = 0xFF;

  report->X = clamp(0, state[0].user.axis[sa_lstick_x] + 128, 255);
  report->Y = clamp(0, state[0].user.axis[sa_lstick_y] + 128, 255);
  report->Z = clamp(0, state[0].user.axis[sa_rstick_x] + 128, 255);
  report->Rz = clamp(0, state[0].user.axis[sa_rstick_y] + 128, 255);

  if (state[0].user.axis[sa_square])
  {
    report->Bt2 &= ~0x80;
  }
  if (state[0].user.axis[sa_cross])
  {
    report->Bt2 &= ~0x40;
  }
  if (state[0].user.axis[sa_circle])
  {
    report->Bt2 &= ~0x20;
  }
  if (state[0].user.axis[sa_triangle])
  {
    report->Bt2 &= ~0x10;
  }

  if (state[0].user.axis[sa_select])
  {
    report->Bt1 &= ~0x01;
  }
  if (state[0].user.axis[sa_start])
  {
    report->Bt1 &= ~0x08;
  }
  if (state[0].user.axis[sa_l3])
  {
    report->Bt1 &= ~0x02;
  }
  if (state[0].user.axis[sa_r3])
  {
    report->Bt1 &= ~0x04;
  }

  if (state[0].user.axis[sa_l1])
  {
    report->Bt2 &= ~0x04;
  }
  if (state[0].user.axis[sa_r1])
  {
    report->Bt2 &= ~0x08;
  }
  if (state[0].user.axis[sa_l2])
  {
    report->Bt2 &= ~0x01;
  }
  if (state[0].user.axis[sa_r2])
  {
    report->Bt2 &= ~0x02;
  }

  if (state[0].user.axis[sa_up])
  {
    report->Bt1 &= ~0x10;
  }
  if (state[0].user.axis[sa_right])
  {
    report->Bt1 &= ~0x20;
  }
  if (state[0].user.axis[sa_down])
  {
    report->Bt1 &= ~0x40;
  }
  if (state[0].user.axis[sa_left])
  {
    report->Bt1 &= ~0x80;
  }

  return sizeof(*report);
}

unsigned int report_build(s_report* report, e_controller_type type)
{
  unsigned int ret = 0;
  switch(type)
	{
	  case C_TYPE_JOYSTICK:
		ret = joystick_report_build(&report->js);
		break;
	  case C_TYPE_360_PAD:
		ret = _360pad_report_build(&report->x360);
		break;
    case C_TYPE_XBOX_PAD:
    ret = XboxPad_report_build(&report->xbox);
    break;
	  case C_TYPE_SIXAXIS:
		ret = sixaxis_report_build(report->sixaxis);
		break;
	  case C_TYPE_PS2_PAD:
		ret = ps2_report_build(&report->ps2);
		break;
	  default:
		break;
	}
  return ret;
}
