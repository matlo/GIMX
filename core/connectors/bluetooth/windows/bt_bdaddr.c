/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
   License: GPLv3
*/

#include <connectors/bluetooth/windows/bt_bdaddr.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Copy, swap, convert BD Address */
inline int bacmp(const bdaddr_t *ba1, const bdaddr_t *ba2)
{
  return memcmp(ba1, ba2, sizeof(bdaddr_t));
}

inline void bacpy(bdaddr_t *dst, const bdaddr_t *src)
{
  memcpy(dst, src, sizeof(bdaddr_t));
}

int bachk(const char *str)
{
  if (!str)
    return -1;

  if (strlen(str) != 17)
    return -1;

  while (*str) {
    if (!isxdigit(*str++))
      return -1;

    if (!isxdigit(*str++))
      return -1;

    if (*str == 0)
      break;

    if (*str++ != ':')
      return -1;
  }

  return 0;
}

void baswap(bdaddr_t *dst, bdaddr_t *src)
{
  unsigned char *d = (unsigned char *) dst;
  unsigned char *s = (unsigned char *) src;
  unsigned int i;

  for (i = 0; i < 6; i++)
  {
    d[i] = s[5 - i];
  }
}

int ba2str(const bdaddr_t *ba, char *str)
{
  return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
    ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int str2ba(const char *str, bdaddr_t *ba)
{
  bdaddr_t b;
  int i;

  if (bachk(str) < 0) {
    memset(ba, 0, sizeof(*ba));
    return -1;
  }

  for (i = 0; i < 6; i++, str += 3)
    b.b[i] = strtol(str, NULL, 16);

  baswap(ba, &b);

  return 0;
}
