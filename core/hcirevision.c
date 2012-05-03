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

  if(argc > 1 && sscanf(argv[1], "hci%d", &i) > 0)
  {
    sprintf(command, "hciconfig hci%d revision", i);

    system(command);
  }

  return 0;

}
