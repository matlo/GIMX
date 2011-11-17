/*
 * hcirevision.c
 *
 *  Created on: 14 août 2011
 *      Author: matlo
 */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  char command[64];
  int i;

  if(argc > 1)
  {
    i = atoi(argv[1]);

    if(i > -1 && i < 128)
    {
      sprintf(command, "hciconfig hci%d revision", i);
    }

    system(command);
  }

  return 0;

}
