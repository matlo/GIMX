/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gpoll.h>
#include <gimxcommon/include/gerror.h>

#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>

#define MAX_SOURCES 1024

static struct {
  int user;
  int (*fp_read)(int);
  int (*fp_write)(int);
  int (*fp_close)(int);
  short int event;
} sources[MAX_SOURCES] = { };

static int max_source = 0;

int gpoll_register_fd(int fd, int user, const GPOLL_CALLBACKS * callbacks) {

  if (!callbacks->fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!callbacks->fp_read && !callbacks->fp_write) {
    PRINT_ERROR_OTHER("fp_read and fp_write are NULL")
    return -1;
  }
  if (fd < 0 || fd >= MAX_SOURCES) {
    PRINT_ERROR_OTHER("fd is invalid")
    return -1;
  }
  sources[fd].user = user;
  if (callbacks->fp_read) {
    sources[fd].event |= POLLIN;
    sources[fd].fp_read = callbacks->fp_read;
  }
  if (callbacks->fp_write) {
    sources[fd].event |= POLLOUT;
    sources[fd].fp_write = callbacks->fp_write;
  }
  sources[fd].fp_close = callbacks->fp_close;
  if (fd > max_source) {
    max_source = fd;
  }
  return 0;
}

int gpoll_remove_fd(int fd) {

  if (fd >= 0 && fd < MAX_SOURCES) {
    memset(sources + fd, 0x00, sizeof(*sources));
    return 0;
  }
  return -1;
}

static unsigned int fill_fds(nfds_t nfds, struct pollfd fds[nfds]) {

  unsigned int pos = 0;

  unsigned int i;
  for (i = 0; i < nfds; ++i) {
    if (sources[i].event) {
      fds[pos].fd = i;
      fds[pos].events = sources[i].event;
      ++pos;
    }
  }

  return pos;
}

void gpoll(void) {

  unsigned int i;
  int res;

  while (1) {

    struct pollfd fds[max_source + 1];
    nfds_t nfds = fill_fds(max_source + 1, fds);

    if (poll(fds, nfds, -1) > 0) {
      for (i = 0; i < nfds; ++i) {
        if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
          res = sources[fds[i].fd].fp_close(sources[fds[i].fd].user);
          gpoll_remove_fd(fds[i].fd);
          if (res) {
            return;
          }
          continue;
        }
        if (fds[i].revents & POLLIN) {
          if (sources[fds[i].fd].fp_read(sources[fds[i].fd].user)) {
            return;
          }
        }
        if (fds[i].revents & POLLOUT) {
          if (sources[fds[i].fd].fp_write(sources[fds[i].fd].user)) {
            return;
          }
        }
      }
    } else {
      if(errno != EINTR) {
        PRINT_ERROR_ERRNO("poll")
      }
    }
  }
}
