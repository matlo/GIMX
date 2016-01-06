/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <windows.h>
#include <stdio.h>

/*
 * This is the Windows equivalent for perror.
 */
void gerror_print_last(const char * msg) {

  DWORD error = GetLastError();
  LPTSTR pBuffer = NULL;

  if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPTSTR) & pBuffer, 0,
      NULL)) {
    fprintf(stderr, "%s: code = %lu\n", msg, error);
  } else {
    fprintf(stderr, "%s: %s\n", msg, pBuffer);
    LocalFree(pBuffer);
  }
}
