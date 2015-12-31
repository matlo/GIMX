/*
 Copyright (c) 2015 Mathieu Laurendeau
 License: GPLv3
 */

#ifndef GPOLL_H_
#define GPOLL_H_

typedef int (* GPOLL_READ_CALLBACK)(int user);
typedef int (* GPOLL_WRITE_CALLBACK)(int user);
typedef int (* GPOLL_CLOSE_CALLBACK)(int user);

typedef int (* GPOLL_REGISTER_FD)(int fd, int id, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write, GPOLL_CLOSE_CALLBACK fp_close);

#ifdef __cplusplus
extern "C" {
#endif

void gpoll();
int gpoll_register_fd(int fd, int user, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write, GPOLL_CLOSE_CALLBACK fp_close);
void gpoll_remove_fd(int fd);

#ifdef WIN32

typedef void * HANDLE;

typedef int (* GPOLL_REGISTER_HANDLE)(HANDLE handle, int id, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write, GPOLL_CLOSE_CALLBACK fp_close);

int gpoll_register_handle(HANDLE handle, int user, GPOLL_READ_CALLBACK fp_read, GPOLL_WRITE_CALLBACK fp_write, GPOLL_CLOSE_CALLBACK fp_close);
void gpoll_remove_handle(HANDLE handle);

#endif

#ifdef __cplusplus
}
#endif

#endif /* GPOLL_H_ */
