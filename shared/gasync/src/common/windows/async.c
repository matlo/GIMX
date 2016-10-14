/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <common/async.h>
#include <common/gerror.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define ASYNC_MAX_WRITE_QUEUE_SIZE 2

static unsigned int clients = 0;

#define CHECK_INITIALIZED(PRINT,RETVALUE) \
    if (clients == 0) { \
        if (PRINT != 0) { \
            PRINT_ERROR_OTHER("async_init should be called first") \
        } \
        return RETVALUE; \
    }

typedef struct
{
  struct
  {
    char * buf;
    unsigned int count;
  } data[ASYNC_MAX_WRITE_QUEUE_SIZE];
  unsigned int nb;
} s_queue;

static struct {
    HANDLE handle;
    char * path;
    struct
    {
      OVERLAPPED overlapped;
      char * buf;
      unsigned int count;
      unsigned int bread;
      unsigned int size;
    } read;
    struct
    {
      OVERLAPPED overlapped;
      s_queue queue;
      unsigned int size;
    } write;
    struct {
        int user;
        ASYNC_READ_CALLBACK fp_read;
        ASYNC_WRITE_CALLBACK fp_write;
        ASYNC_CLOSE_CALLBACK fp_close;
        ASYNC_REMOVE_SOURCE fp_remove;
    } callback;
    void * priv;
    e_async_device_type device_type;
} devices[ASYNC_MAX_DEVICES] = { };

#define ASYNC_CHECK_DEVICE(device,retValue) \
    if(device < 0 || device >= ASYNC_MAX_DEVICES) { \
        fprintf(stderr, "%s:%d %s: invalid device (%d)\n", __FILE__, __LINE__, __func__, device); \
        return retValue; \
    } \
    if(devices[device].handle == INVALID_HANDLE_VALUE) { \
        fprintf(stderr, "%s:%d %s: no such device (%d)\n", __FILE__, __LINE__, __func__, device); \
        return retValue; \
    }

static void reset_handles(int device) {
    devices[device].handle = INVALID_HANDLE_VALUE;
    devices[device].read.overlapped.hEvent = INVALID_HANDLE_VALUE;
    devices[device].write.overlapped.hEvent = INVALID_HANDLE_VALUE;
}

static BOOL (__stdcall *pCancelIoEx)(HANDLE, LPOVERLAPPED) = NULL;

static inline int setup_cancel_io(void)
{
  HMODULE hKernel32 = GetModuleHandleA("KERNEL32");
  if (hKernel32 != NULL) {
    pCancelIoEx = (BOOL (__stdcall *)(HANDLE,LPOVERLAPPED)) GetProcAddress(hKernel32, "CancelIoEx");
  }
  if(pCancelIoEx == NULL) {
    PRINT_ERROR_GETLASTERROR("GetProcAddress")
    return -1;
  }
  return 0;
}

void async_init_static(void) __attribute__((constructor));
void async_init_static(void) {
    int i;
    for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
        reset_handles(i);
    }
}

int async_init() {

    if (clients == UINT_MAX) {
        PRINT_ERROR_OTHER("too many clients")
        return -1;
    }
    
    if (clients == 0 && pCancelIoEx == NULL) {
        if (setup_cancel_io() < 0) {
            return -1;
        }
    }

    ++clients;

    return 0;
}

int async_exit() {

    if (clients > 0) {
        --clients;
        if (clients == 0) {
            int i;
            for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
                if(devices[i].handle != INVALID_HANDLE_VALUE) {
                    async_close(i);
                }
            }
        }
    }
    return 0;
}

static int add_device(const char * path, HANDLE handle, int print) {
    int i;
    for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
        if(devices[i].path && !strcmp(devices[i].path, path)) {
            if(print) {
                fprintf(stderr, "%s:%d add_device %s: device already opened\n", __FILE__, __LINE__, path);
            }
            return -1;
        }
    }
    for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
        if(devices[i].handle == INVALID_HANDLE_VALUE) {
            devices[i].path = strdup(path);
            if(devices[i].path != NULL) {
                devices[i].handle = handle;
                return i;
            }
            else {
                fprintf(stderr, "%s:%d add_device %s: can't duplicate path\n", __FILE__, __LINE__, path);
                return -1;
            }
        }
    }
    return -1;
}

static int queue_write(int device, const char * buf, unsigned int count) {
  if(devices[device].write.queue.nb == ASYNC_MAX_WRITE_QUEUE_SIZE) {
      fprintf(stderr, "%s:%d %s: no space left in write queue for device (%s)\n", __FILE__, __LINE__, __func__, devices[device].path);
      return -1;
  }
  if(count < devices[device].write.size) {
      count = devices[device].write.size;
  }
  void * dup = calloc(count, sizeof(char));
  if(!dup) {
      PRINT_ERROR_ALLOC_FAILED("malloc")
      return -1;
  }
  memcpy(dup, buf, count);
  devices[device].write.queue.data[devices[device].write.queue.nb].buf = dup;
  devices[device].write.queue.data[devices[device].write.queue.nb].count = count;
  ++devices[device].write.queue.nb;
  return devices[device].write.queue.nb - 1;
}

static int dequeue_write(int device) {
  if(devices[device].write.queue.nb > 0) {
      --devices[device].write.queue.nb;
      free(devices[device].write.queue.data[0].buf);
      memmove(devices[device].write.queue.data, devices[device].write.queue.data + 1, devices[device].write.queue.nb * sizeof(*devices[device].write.queue.data));
  }
  return devices[device].write.queue.nb - 1;
}

static int set_overlapped(int device) {
    /*
     * create event objects for overlapped I/O
     */
    devices[device].read.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(devices[device].read.overlapped.hEvent == INVALID_HANDLE_VALUE) {
        PRINT_ERROR_GETLASTERROR("CreateEvent")
        return -1;
    }
    devices[device].write.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(devices[device].write.overlapped.hEvent == INVALID_HANDLE_VALUE) {
        PRINT_ERROR_GETLASTERROR("CreateEvent")
        return -1;
    }
    return 0;
}

int async_open_path(const char * path, int print) {

    CHECK_INITIALIZED(print, -1)

    DWORD accessdirection = GENERIC_READ | GENERIC_WRITE;
    DWORD sharemode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    int ret = -1;
    if(path != NULL) {
        HANDLE handle = CreateFile(path, accessdirection, sharemode, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
        if(handle != INVALID_HANDLE_VALUE) {
            ret = add_device(path, handle, print);
            if(ret == -1) {
                CloseHandle(handle);
            }
            else if(set_overlapped(ret) == -1) {
                async_close(ret);
                ret = -1;
            }
        }
        else {
            if(print) {
                PRINT_ERROR_GETLASTERROR("CreateFile")
            }
        }
    }
    return ret;
}

int async_close(int device) {

    ASYNC_CHECK_DEVICE(device, -1)

    DWORD dwBytesTransfered;

    if(devices[device].read.overlapped.hEvent != INVALID_HANDLE_VALUE) {
      if(pCancelIoEx(devices[device].handle, &devices[device].read.overlapped)) {
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesTransfered, TRUE)) { //block until completion
          if(GetLastError() != ERROR_OPERATION_ABORTED) {
            PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
          }
        }
      }
      else if(GetLastError() != ERROR_NOT_FOUND)
			{
				PRINT_ERROR_GETLASTERROR("CancelIoEx")
			}
    }
    if(devices[device].write.overlapped.hEvent != INVALID_HANDLE_VALUE) {
      if(pCancelIoEx(devices[device].handle, &devices[device].write.overlapped)) {
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesTransfered, TRUE)) { //block until completion
          if(GetLastError() != ERROR_OPERATION_ABORTED) {
            PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
          }
        }
      }
      else if(GetLastError() != ERROR_NOT_FOUND)
			{
				PRINT_ERROR_GETLASTERROR("CancelIoEx")
			}
    }

    while(dequeue_write(device) != -1) ;

    free(devices[device].read.buf);
    free(devices[device].path);

    if (devices[device].callback.fp_remove != NULL) {
        devices[device].callback.fp_remove(devices[device].read.overlapped.hEvent);
        devices[device].callback.fp_remove(devices[device].write.overlapped.hEvent);
    }

    CloseHandle(devices[device].read.overlapped.hEvent);
    CloseHandle(devices[device].write.overlapped.hEvent);
    CloseHandle(devices[device].handle);

    memset(devices + device, 0x00, sizeof(*devices));

    reset_handles(device);

    return 0;
}

int async_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  ASYNC_CHECK_DEVICE(device, -1)

  if(devices[device].device_type == E_ASYNC_DEVICE_TYPE_HID && devices[device].read.size == 0) {
    PRINT_ERROR_OTHER("the HID device has no IN endpoint")
    return -1;
  }

  DWORD dwBytesRead = 0;

  memset(buf, 0x00, count);

  if(!ReadFile(devices[device].handle, buf, count, NULL, &devices[device].read.overlapped)) {
    if(GetLastError() != ERROR_IO_PENDING) {
      PRINT_ERROR_GETLASTERROR("ReadFile")
      return -1;
    }
    int ret = WaitForSingleObject(devices[device].read.overlapped.hEvent, timeout);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE)) {
          PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        if(!pCancelIoEx(devices[device].handle, &devices[device].read.overlapped)) {
          PRINT_ERROR_GETLASTERROR("CancelIoEx")
          return -1;
        }
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, TRUE)) { //block until completion
        	if(GetLastError() != ERROR_OPERATION_ABORTED) {
						PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
						return -1;
					}
        }
        if(dwBytesRead != count) { // the read operation may still have succeed
          PRINT_ERROR_OTHER("ReadFile failed: timeout expired.")
        }
        break;
      default:
        PRINT_ERROR_GETLASTERROR("WaitForSingleObject")
        return -1;
    }
  }
  else
  {
    dwBytesRead = count;
  }

  // skip the eventual leading null byte for hid devices
  if (devices[device].device_type == E_ASYNC_DEVICE_TYPE_HID && dwBytesRead > 0) {
    if (((unsigned char*)buf)[0] == 0x00) {
      --dwBytesRead;
      memmove(buf, buf + 1, dwBytesRead);
    }
  }

  return dwBytesRead;
}

int async_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

  ASYNC_CHECK_DEVICE(device, -1)
    
  if(devices[device].device_type == E_ASYNC_DEVICE_TYPE_HID && devices[device].write.size == 0) {
    PRINT_ERROR_OTHER("the HID device has no OUT endpoint")
    return -1;
  }

  DWORD dwBytesWritten = 0;

  if(count < devices[device].write.size) {
      count = devices[device].write.size;
  }

  if(!WriteFile(devices[device].handle, buf, count, NULL, &devices[device].write.overlapped)) {
    if(GetLastError() != ERROR_IO_PENDING) {
      PRINT_ERROR_GETLASTERROR("WriteFile")
      return -1;
    }
    int ret = WaitForSingleObject(devices[device].write.overlapped.hEvent, timeout);
    switch (ret) {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE)) {
          PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        if(!pCancelIoEx(devices[device].handle, &devices[device].write.overlapped)) {
          PRINT_ERROR_GETLASTERROR("CancelIoEx")
          return -1;
        }
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, TRUE)) { //block until completion
        	if(GetLastError() != ERROR_OPERATION_ABORTED) {
						PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
						return -1;
					}
        }
        if(dwBytesWritten != count) { // the write operation may still have succeed
          PRINT_ERROR_OTHER("WriteFile failed: timeout expired.")
        }
        break;
      default:
        PRINT_ERROR_GETLASTERROR("WaitForSingleObject")
        return -1;
    }
  }
  else {
    dwBytesWritten = count;
  }

  return dwBytesWritten;
}

/*
 * This function starts an overlapped read.
 * If the read completes immediately, it returns the number of transferred bytes, which is the number of requested bytes.
 * If the read is pending, it returns -1.
 */
static int start_overlapped_read(int device) {

  if(devices[device].read.buf == NULL) {

    PRINT_ERROR_OTHER("the read buffer is NULL")
    return -1;
  }

  int ret = -1;

  memset(devices[device].read.buf, 0x00, devices[device].read.count);
  
  if(!ReadFile(devices[device].handle, devices[device].read.buf, devices[device].read.count, NULL, &devices[device].read.overlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      PRINT_ERROR_GETLASTERROR("ReadFile")
      ret = -1;
    }
  }
  else
  {
    DWORD dwBytesRead;
    if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE)) {
      PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
      return -1;
    }
    // the read immediately completed
    ret = devices[device].read.bread = dwBytesRead;
  }
  
  return ret;
}

static int read_packet(int device) {
  
  if(devices[device].read.bread) {
    if(devices[device].callback.fp_read != NULL) {
      // skip the eventual leading null byte for hid devices
      if (devices[device].device_type == E_ASYNC_DEVICE_TYPE_HID && ((unsigned char*)devices[device].read.buf)[0] == 0x00) {
        --devices[device].read.bread;
        memmove(devices[device].read.buf, devices[device].read.buf + 1, devices[device].read.bread);
      }
      devices[device].callback.fp_read(devices[device].callback.user, (const char *)devices[device].read.buf, devices[device].read.bread);
    }
    devices[device].read.bread = 0;
  }

  return start_overlapped_read(device);
}

static int read_callback(int device) {

    ASYNC_CHECK_DEVICE(device, -1)
    
    DWORD dwBytesRead = 0;

    if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE))
    {
      PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
      return -1;
    }

    devices[device].read.bread = dwBytesRead;
    
    while(read_packet(device) >= 0) ;

    return 0;
}

static int write_internal(int device) {

    DWORD dwBytesWritten = 0;

    if(!WriteFile(devices[device].handle, devices[device].write.queue.data[0].buf, devices[device].write.queue.data[0].count, NULL, &devices[device].write.overlapped)) {
      if(GetLastError() != ERROR_IO_PENDING) {
        PRINT_ERROR_GETLASTERROR("WriteFile")
        return -1;
      }
    }
    else {
      if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE)) {
        PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
        return -1;
      }
    }

    return dwBytesWritten;
}

static int write_callback(int device) {

    ASYNC_CHECK_DEVICE(device, -1)

    int ret = 0;

    DWORD dwBytesWritten = 0;

    if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE))
    {
      PRINT_ERROR_GETLASTERROR("GetOverlappedResult")
      ret = -1;
    }

    while(dequeue_write(device) != -1) {
        int result = write_internal(device);
        if(result == 0) {
            break; // IO is pending, completion will execute write_callback
        }
        else if(result < 0) {
            ret = -1; // IO failed, report this error
        }
    }

    if(devices[device].callback.fp_write) {
        devices[device].callback.fp_write(devices[device].callback.user, ret == 0 ? (int)dwBytesWritten : -1);
    }

    return ret;
}

static int close_callback(int device) {

    ASYNC_CHECK_DEVICE(device, -1)

    return devices[device].callback.fp_close(devices[device].callback.user);
}

int async_set_read_size(int device, unsigned int size) {

    ASYNC_CHECK_DEVICE(device, -1)
    
    if(size > devices[device].read.size) {
        void * ptr = realloc(devices[device].read.buf, size);
        if(ptr == NULL) {
            fprintf(stderr, "%s:%d %s: can't allocate a buffer\n", __FILE__, __LINE__, __func__);
            return -1;
        }
        devices[device].read.buf = ptr;
        devices[device].read.size = size;
    }
  
    devices[device].read.count = size;

    return 0;
}

int async_register(int device, int user, const ASYNC_CALLBACKS * callbacks) {

    ASYNC_CHECK_DEVICE(device, -1)
    
    while(read_packet(device) >= 0) ;

    int ret = 0;

    if (callbacks->fp_read) {
        GPOLL_CALLBACKS gpoll_callbacks = {
          .fp_read = read_callback,
          .fp_write = NULL,
          .fp_close = close_callback,
        };
        ret = callbacks->fp_register(devices[device].read.overlapped.hEvent, device, &gpoll_callbacks);
    }
    if (ret != -1) {
        GPOLL_CALLBACKS gpoll_callbacks = {
          .fp_read = NULL,
          .fp_write = write_callback,
          .fp_close = close_callback,
        };
        ret = callbacks->fp_register(devices[device].write.overlapped.hEvent, device, &gpoll_callbacks);
    }

    if (ret != -1) {
      devices[device].callback.user = user;
      devices[device].callback.fp_read = callbacks->fp_read;
      devices[device].callback.fp_write = callbacks->fp_write;
      devices[device].callback.fp_close = callbacks->fp_close;
      devices[device].callback.fp_remove = callbacks->fp_remove;
    }

    return ret;
}

int async_write(int device, const void * buf, unsigned int count) {

    ASYNC_CHECK_DEVICE(device, -1)
    
    if(devices[device].device_type == E_ASYNC_DEVICE_TYPE_HID && devices[device].write.size == 0) {
        PRINT_ERROR_OTHER("the HID device has no OUT endpoint")
        return -1;
    }

    int res = queue_write(device, buf, count);
    if(res < 0) {
        return -1; // cannot be queued
    }
    else if(res > 0) {
        return 0; // another IO is pending
    }

    int ret = write_internal(device);
    if(ret != 0) {
        dequeue_write(device); // IO failed or completed
        //TODO MLA: manage disconnection
    }

    return ret;
}

void async_set_private(int device, void * priv) {

    ASYNC_CHECK_DEVICE(device,)

    devices[device].priv = priv;
}

void * async_get_private(int device) {

    ASYNC_CHECK_DEVICE(device, NULL)

    return devices[device].priv;
}

HANDLE * async_get_handle(int device) {

    ASYNC_CHECK_DEVICE(device, INVALID_HANDLE_VALUE)

    return devices[device].handle;
}

int async_set_write_size(int device, unsigned int size) {

    ASYNC_CHECK_DEVICE(device, -1)

    devices[device].write.size = size;

    return 0;
}

void async_set_device_type(int device, e_async_device_type device_type) {

    ASYNC_CHECK_DEVICE(device,)

    devices[device].device_type = device_type;
}

const char * async_get_path(int device) {

    ASYNC_CHECK_DEVICE(device, NULL)

    return devices[device].path;
}
