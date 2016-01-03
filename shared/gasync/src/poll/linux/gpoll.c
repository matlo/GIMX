/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gpoll.h>

#include <stdio.h>
#include <poll.h>
#include <string.h>

#define MAX_SOURCES 1024

#define PRINT_ERROR_OTHER(msg) fprintf(stderr, "%s:%d %s: %s\n", __FILE__, __LINE__, __func__, msg);

static struct {
  int user;
  int (*fp_read)(int);
  int (*fp_write)(int);
  int (*fp_close)(int);
  short int event;
} sources[MAX_SOURCES] = { };

static int max_source = 0;

int gpoll_register_fd(int fd, int user, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write,
    GPOLL_CLOSE_CALLBACK fp_close) {

  if (!fp_close) {
    PRINT_ERROR_OTHER("fp_close is mandatory")
    return -1;
  }
  if (!fp_read && !fp_write) {
    PRINT_ERROR_OTHER("fp_read and fp_write are NULL")
    return -1;
  }
  if (fd < 0 || fd >= MAX_SOURCES) {
    PRINT_ERROR_OTHER("fd is invalid")
    return -1;
  }
  sources[fd].user = user;
  if (fp_read) {
    sources[fd].event |= POLLIN;
    sources[fd].fp_read = fp_read;
  }
  if (fp_write) {
    sources[fd].event |= POLLOUT;
    sources[fd].fp_write = fp_write;
  }
  sources[fd].fp_close = fp_close;
  if (fd > max_source) {
    max_source = fd;
  }
  return 0;
}

void gpoll_remove_fd(int fd) {

  if (fd >= 0 && fd < MAX_SOURCES) {
    memset(sources + fd, 0x00, sizeof(*sources));
  }
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
    }
  }
}
