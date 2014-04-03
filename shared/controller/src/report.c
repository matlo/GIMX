/*
 Copyright (c) 2014 Mathieu Laurendeau
 License: GPLv3
 */

#include <report.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned int (*func_ptr[C_TYPE_MAX])(int axis[AXIS_MAX], s_report* report) = {};

void report_init(void) __attribute__((constructor (102)));
void report_init(void)
{
  int type;
  for(type=0; type<C_TYPE_MAX; ++type)
  {
    if(!func_ptr[type])
    {
      fprintf(stderr, "Controller type %d is missing a report builder!\n", type);
      exit(-1);
    }
  }
}

void report_register_builder(e_controller_type type, unsigned int (*fp)(int axis[AXIS_MAX], s_report* report)) {
  func_ptr[type] = fp;
}

unsigned int report_build(e_controller_type type, int axis[AXIS_MAX], s_report* report)
{
  unsigned int ret = 0;
  if(func_ptr[type])
  {
    ret = func_ptr[type](axis, report);
  }
  return ret;
}
