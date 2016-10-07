/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GPOLL_H_
#define GPOLL_H_

typedef int (* GPOLL_READ_CALLBACK)(int user);
typedef int (* GPOLL_WRITE_CALLBACK)(int user);
typedef int (* GPOLL_CLOSE_CALLBACK)(int user);

typedef struct {
    GPOLL_READ_CALLBACK fp_read;   // called on data reception
    GPOLL_WRITE_CALLBACK fp_write; // called on write completion
    GPOLL_CLOSE_CALLBACK fp_close; // called on failure
} GPOLL_CALLBACKS;

typedef int (* GPOLL_REGISTER_FD)(int fd, int id, const GPOLL_CALLBACKS * callbacks);
typedef int (* GPOLL_REMOVE_FD)(int fd);

#ifdef WIN32
typedef void * HANDLE;
typedef int (* GPOLL_REGISTER_HANDLE)(HANDLE handle, int id, const GPOLL_CALLBACKS * callbacks);
typedef int (* GPOLL_REMOVE_HANDLE)(HANDLE handle);
#endif

#ifndef WIN32
typedef GPOLL_REGISTER_FD GPOLL_REGISTER_SOURCE;
typedef GPOLL_REMOVE_FD GPOLL_REMOVE_SOURCE;
#else
typedef GPOLL_REGISTER_HANDLE GPOLL_REGISTER_SOURCE;
typedef GPOLL_REMOVE_HANDLE GPOLL_REMOVE_SOURCE;
#endif

typedef struct {
    GPOLL_REGISTER_SOURCE fp_register; // to register the device to event sources
    GPOLL_REMOVE_SOURCE fp_remove;     // to remove the device from event sources
} GPOLL_INTERFACE;

#ifdef __cplusplus
extern "C" {
#endif

void gpoll();
int gpoll_register_fd(int fd, int user, const GPOLL_CALLBACKS * callbacks);
int gpoll_remove_fd(int fd);

#ifdef WIN32
int gpoll_register_handle(HANDLE handle, int user, const GPOLL_CALLBACKS * callbacks);
int gpoll_remove_handle(HANDLE handle);
void gpoll_set_rawinput_callback(void (*callback)());
#endif

#ifdef __cplusplus
}
#endif

#endif /* GPOLL_H_ */
