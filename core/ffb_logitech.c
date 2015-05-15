/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <stdio.h>

#ifdef WIN32
#define GIMX_PACKED __attribute__((gcc_struct, packed))
#else
#define GIMX_PACKED __attribute__((packed))
#endif

typedef enum
{
  E_DOWNLOAD = 0x00,
  E_DOWNLOAD_AND_PLAY = 0x01,
  E_PLAY = 0x02,
  E_STOP = 0x03,
  E_DEFAULT_SPRING_ON = 0x04,
  E_DEFAULT_SPRING_OFF = 0x05,
  E_RESERVED_1 = 0x06,
  E_RESERVED_2 = 0x07,
  E_NORMAL_MODE = 0x08,
  E_EXTENDED_COMMAND = 0xF8,
  E_SET_LED = 0x09,
  E_SET_WATCHDOG = 0x0A,
  E_RAW_MODE = 0x0B,
  E_REFRESH_FORCE = 0x0C,
  E_FIXED_TIME_LOOP = 0x0D,
  E_SET_DEFAULT_SPRING = 0x0E,
  E_SET_DEAD_BAND = 0x0F
} e_command;

typedef enum
{
  E_CONSTANT = 0x00,
  E_SPRING = 0x01,
  E_DAMPER = 0x02,
  E_AUTO_CENTER_SPRING = 0x03,
  E_SAWTOOTH_UP = 0x04,
  E_SAWTOOTH_DOWN = 0x05,
  E_TRAPEZOID = 0x06,
  E_RECTANGLE = 0x07,
  E_VARIABLE = 0x08,
  E_RAMP = 0x09,
  E_SQUARE_WAVE = 0x0A,
  E_HIGH_RESOLUTION_SPRING = 0x0B,
  E_HIGH_RESOLUTION_DAMPER = 0x0C,
  E_HIGH_RESOLUTION_AUTO_CENTER_SPRING = 0x0D,
  E_FRICTION = 0x0E,
} e_force;

typedef struct GIMX_PACKED
{
  unsigned char cmdAndForce;
} s_header;

typedef struct GIMX_PACKED
{
  s_header header;

} s_download;

static void print_command(unsigned char header)
{
  unsigned char command = header & 0x0F;
  switch(command)
  {
    case E_DOWNLOAD:
      printf("E_DOWNLOAD");
      break;
    case E_DOWNLOAD_AND_PLAY:
      printf("E_DOWNLOAD_AND_PLAY");
      break;
    case E_PLAY:
      printf("E_PLAY");
      break;
    case E_STOP:
      printf("E_STOP");
      break;
    case E_DEFAULT_SPRING_ON:
      printf("E_DEFAULT_SPRING_ON");
      break;
    case E_DEFAULT_SPRING_OFF:
      printf("E_DEFAULT_SPRING_OFF");
      break;
    case E_RESERVED_1:
      printf("E_RESERVED_1");
      break;
    case E_RESERVED_2:
      printf("E_RESERVED_2");
      break;
    case 0x08:
      if(header == 0xF8)
      {
        printf("E_EXTENDED_COMMAND");
      }
      else
      {
        printf("E_NORMAL_MODE");
      }
      break;
    case E_EXTENDED_COMMAND:
      printf("E_EXTENDED_COMMAND");
      break;
    case E_SET_LED:
      printf("E_SET_LED");
      break;
    case E_SET_WATCHDOG:
      printf("E_SET_WATCHDOG");
      break;
    case E_RAW_MODE:
      printf("E_RAW_MODE");
      break;
    case E_REFRESH_FORCE:
      printf("E_REFRESH_FORCE");
      break;
    case E_FIXED_TIME_LOOP:
      printf("E_FIXED_TIME_LOOP");
      break;
    case E_SET_DEFAULT_SPRING:
      printf("E_SET_DEFAULT_SPRING");
      break;
    case E_SET_DEAD_BAND:
      printf("E_SET_DEAD_BAND");
      break;
  }
}

static void print_force(unsigned char header)
{
  unsigned char force = (header & 0xF0) >> 4;
  switch(force)
  {
    case E_CONSTANT:
      printf("E_CONSTANT");
      break;
    case E_SPRING:
      printf("E_SPRING");
      break;
    case E_DAMPER:
      printf("E_DAMPER");
      break;
    case E_AUTO_CENTER_SPRING:
      printf("E_AUTO_CENTER_SPRING");
      break;
    case E_SAWTOOTH_UP:
      printf("E_SAWTOOTH_UP");
      break;
    case E_SAWTOOTH_DOWN:
      printf("E_SAWTOOTH_DOWN");
      break;
    case E_TRAPEZOID:
      printf("E_TRAPEZOID");
      break;
    case E_RECTANGLE:
      printf("E_RECTANGLE");
      break;
    case E_VARIABLE:
      printf("E_VARIABLE");
      break;
    case E_RAMP:
      printf("E_RAMP");
      break;
    case E_SQUARE_WAVE:
      printf("E_SQUARE_WAVE");
      break;
    case E_HIGH_RESOLUTION_SPRING:
      printf("E_HIGH_RESOLUTION_SPRING");
      break;
    case E_HIGH_RESOLUTION_DAMPER:
      printf("E_HIGH_RESOLUTION_DAMPER");
      break;
    case E_HIGH_RESOLUTION_AUTO_CENTER_SPRING:
      printf("E_HIGH_RESOLUTION_AUTO_CENTER_SPRING");
      break;
    case E_FRICTION:
      printf("E_FRICTION");
      break;
  }
}

void ffb_logitech_decode(unsigned char* report, unsigned char length)
{
  unsigned char header = report[0];

  print_command(header);
  printf(" ");
  print_force(header);
  printf("\n");

}
