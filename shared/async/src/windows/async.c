/*
 Copyright (c) 2015 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <async.h>

#include <windows.h>
#include <stdio.h>

s_device devices[ASYNC_MAX_DEVICES] = { };

static void reset_handles(int device) {
    devices[device].handle = INVALID_HANDLE_VALUE;
    devices[device].read.overlapped.hEvent = INVALID_HANDLE_VALUE;
    devices[device].write.overlapped.hEvent = INVALID_HANDLE_VALUE;
}

void init(void) __attribute__((constructor (101)));
void init(void) {
    int i;
    for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
        reset_handles(i);
    }
}

void clean(void) __attribute__((destructor (101)));
void clean(void) {
    int i;
    for (i = 0; i < ASYNC_MAX_DEVICES; ++i) {
        if(devices[i].handle != INVALID_HANDLE_VALUE) {
            async_close(i);
        }
    }
}

inline void async_print_error(const char * file, int line, const char * msg) {
  
  DWORD error = GetLastError();
  LPTSTR pBuffer = NULL;

  if(!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL,
          error,
          0,
          (LPTSTR)&pBuffer,
          0,
          NULL))
  {
    fprintf(stderr, "%s:%d %s failed with error: %lu\n", file, line, msg, error);
  }
  else
  {
    fprintf(stderr, "%s:%d %s failed with error: %s\n", file, line, msg, pBuffer);
    LocalFree(pBuffer);
  }
}

inline int async_check_device(int device, const char * file, unsigned int line, const char * func) {
  if(device < 0 || device >= ASYNC_MAX_DEVICES) {
      fprintf(stderr, "%s:%d %s: invalid device (%d)\n", file, line, func, device);
      return -1;
  }
  if(devices[device].handle == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "%s:%d %s: no such device (%d)\n", file, line, func, device);
      return -1;
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
      fprintf(stderr, "%s:%d %s: no space left in write queue for device (%d)\n", __FILE__, __LINE__, __func__, device);
      return -1;
  }
  if(count < devices[device].write.size) {
      count = devices[device].write.size;
  }
  void * dup = calloc(count, sizeof(char));
  if(!dup) {
      fprintf(stderr, "%s:%d %s: malloc failed\n", __FILE__, __LINE__, __func__);
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
        ASYNC_PRINT_ERROR("CreateEvent")
        return -1;
    }
    devices[device].write.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(devices[device].write.overlapped.hEvent == INVALID_HANDLE_VALUE) {
        ASYNC_PRINT_ERROR("CreateEvent")
        return -1;
    }
    return 0;
}

int async_open_path(const char * path, int print) {
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
                ASYNC_PRINT_ERROR("CreateFile")
            }
        }
    }
    return ret;
}

int async_close(int device) {

    ASYNC_CHECK_DEVICE(device)

    DWORD dwBytesTransfered;

    if(devices[device].read.overlapped.hEvent != INVALID_HANDLE_VALUE) {
      if(CancelIoEx(devices[device].handle, &devices[device].read.overlapped) != ERROR_NOT_FOUND) {
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesTransfered, TRUE)) { //block until completion
          ASYNC_PRINT_ERROR("GetOverlappedResult")
        }
      }
    }
    if(devices[device].write.overlapped.hEvent != INVALID_HANDLE_VALUE) {
      if(CancelIoEx(devices[device].handle, &devices[device].write.overlapped) != ERROR_NOT_FOUND) {
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesTransfered, TRUE)) { //block until completion
          ASYNC_PRINT_ERROR("GetOverlappedResult")
        }
      }
    }

    while(dequeue_write(device) != -1) ;

    free(devices[device].read.buf);
    free(devices[device].path);

    CloseHandle(devices[device].read.overlapped.hEvent);
    CloseHandle(devices[device].write.overlapped.hEvent);
    CloseHandle(devices[device].handle);

    memset(devices + device, 0x00, sizeof(*devices));

    reset_handles(device);

    return 0;
}

int async_read_timeout(int device, void * buf, unsigned int count, unsigned int timeout) {

  DWORD dwBytesRead = 0;

  memset(buf, 0x00, count);

  if(!ReadFile(devices[device].handle, buf, count, NULL, &devices[device].read.overlapped)) {
    if(GetLastError() != ERROR_IO_PENDING) {
      ASYNC_PRINT_ERROR("ReadFile")
      return -1;
    }
    int ret = WaitForSingleObject(devices[device].read.overlapped.hEvent, timeout * 1000);
    switch (ret)
    {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE)) {
          ASYNC_PRINT_ERROR("GetOverlappedResult")
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        if(!CancelIoEx(devices[device].handle, &devices[device].read.overlapped)) {
            ASYNC_PRINT_ERROR("CancelIoEx")
            return -1;
        }
        if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, TRUE)) { //block until completion
          ASYNC_PRINT_ERROR("GetOverlappedResult")
          return -1;
        }
        if(dwBytesRead != count) { // the read operation may still have succeed
          fprintf(stderr, "ReadFile failed: timeout expired.\n");
        }
        break;
      default:
        ASYNC_PRINT_ERROR("WaitForSingleObject")
        return -1;
    }
  }
  else
  {
    dwBytesRead = count;
  }

  // skip the eventual leading null byte for hid devices
  if (devices[device].hid.vendor != 0x0000 && dwBytesRead > 0) {
    if (((unsigned char*)buf)[0] == 0x00) {
      --dwBytesRead;
      memmove(buf, buf + 1, dwBytesRead);
    }
  }

  return dwBytesRead;
}

int async_write_timeout(int device, const void * buf, unsigned int count, unsigned int timeout) {

  DWORD dwBytesWritten = 0;

  if(!WriteFile(devices[device].handle, buf, count, NULL, &devices[device].write.overlapped)) {
    if(GetLastError() != ERROR_IO_PENDING) {
      ASYNC_PRINT_ERROR("WriteFile")
      return -1;
    }
    int ret = WaitForSingleObject(devices[device].write.overlapped.hEvent, timeout*1000);
    switch (ret) {
      case WAIT_OBJECT_0:
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE)) {
          ASYNC_PRINT_ERROR("GetOverlappedResult")
          return -1;
        }
        break;
      case WAIT_TIMEOUT:
        if(!CancelIoEx(devices[device].handle, &devices[device].write.overlapped)) {
          ASYNC_PRINT_ERROR("CancelIoEx")
          return -1;
        }
        if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, TRUE)) { //block until completion
          ASYNC_PRINT_ERROR("GetOverlappedResult")
          return -1;
        }
        if(dwBytesWritten != count) { // the write operation may still have succeed
          fprintf(stderr, "WriteFile failed: timeout expired.\n");
        }
        break;
      default:
        ASYNC_PRINT_ERROR("WaitForSingleObject")
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
  
  int ret = -1;

  memset(devices[device].read.buf, 0x00, devices[device].read.count);
  
  if(!ReadFile(devices[device].handle, devices[device].read.buf, devices[device].read.count, NULL, &devices[device].read.overlapped))
  {
    if(GetLastError() != ERROR_IO_PENDING)
    {
      ASYNC_PRINT_ERROR("ReadFile")
      ret = -1;
    }
  }
  else
  {
    DWORD dwBytesRead;
    if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE)) {
      ASYNC_PRINT_ERROR("GetOverlappedResult")
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
      if (devices[device].hid.vendor != 0x0000 && ((unsigned char*)devices[device].read.buf)[0] == 0x00) {
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

    ASYNC_CHECK_DEVICE(device)
    
    DWORD dwBytesRead = 0;

    if (!GetOverlappedResult(devices[device].handle, &devices[device].read.overlapped, &dwBytesRead, FALSE))
    {
      ASYNC_PRINT_ERROR("GetOverlappedResult")
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
        ASYNC_PRINT_ERROR("WriteFile")
        return -1;
      }
    }
    else {
      if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE)) {
        ASYNC_PRINT_ERROR("GetOverlappedResult")
        return -1;
      }
    }

    return dwBytesWritten;
}

static int write_callback(int device) {

    ASYNC_CHECK_DEVICE(device)

    int ret = 0;

    DWORD dwBytesWritten = 0;

    if (!GetOverlappedResult(devices[device].handle, &devices[device].write.overlapped, &dwBytesWritten, FALSE))
    {
      ASYNC_PRINT_ERROR("GetOverlappedResult")
      ret = -1;
    }

    while(dequeue_write(device) != -1) {
        dwBytesWritten = write_internal(device);
        if(dwBytesWritten == 0) {
            break; // IO is pending, completion will execute write_callback
        }
        else if(dwBytesWritten < 0) {
            ret = -1; // IO failed, report this error
        }
    }

    if(devices[device].callback.fp_write) {
        devices[device].callback.fp_write(devices[device].callback.user);
    }

    return ret;
}

static int close_callback(int device) {

    ASYNC_CHECK_DEVICE(device)

    return devices[device].callback.fp_close(devices[device].callback.user);
}

int async_set_read_size(int device, unsigned int size) {

    ASYNC_CHECK_DEVICE(device)
    
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

int async_register(int device, int user, ASYNC_READ_CALLBACK fp_read, ASYNC_WRITE_CALLBACK fp_write, ASYNC_CLOSE_CALLBACK fp_close, ASYNC_REGISTER_SOURCE fp_register) {

    ASYNC_CHECK_DEVICE(device)
    
    while(read_packet(device) >= 0) ;

    devices[device].callback.user = user;
    devices[device].callback.fp_read = fp_read;
    devices[device].callback.fp_write = fp_write;
    devices[device].callback.fp_close = fp_close;

    if(fp_read) {
        fp_register(devices[device].read.overlapped.hEvent, device, read_callback, NULL, close_callback);
    }
    if(fp_write) {
        fp_register(devices[device].write.overlapped.hEvent, device, NULL, write_callback, close_callback);
    }

    return 0;
}

int async_write(int device, const void * buf, unsigned int count) {

    ASYNC_CHECK_DEVICE(device)

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
    }

    return ret;
}
