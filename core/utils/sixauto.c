
/*
 * Simple program that gets the sixaxis and ps3 addresses, changes the dongle bdaddr, and connects to the ps3.
 * Compile with: gcc -o sixauto sixauto.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

int main()
{
  FILE* fp;
  char line[LINE_MAX];
  char command1[256] = "bdaddr -r -i hci0 ";
  char command2[256] = "emu ";
  
  while(1)
  {
    system("sixaddr > addr");
    
    fp = fopen("addr", "r");
    
    if(fp)
    {
      if(fgets(line, LINE_MAX, fp))
      {
        snprintf(command2+4, 18, "%s", line+26);
      }
      if(fgets(line, LINE_MAX, fp))
      {
        snprintf(command1+18, 18, "%s", line+34);
        system(command1);
        sleep(2);
        system(command2);
      }
    }
    
    sleep(1);
  }
}

