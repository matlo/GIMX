/*
  Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
  License: GPLv3

  This code is derived from the manymouse library: https://icculus.org/manymouse/
  Original licence:

  Copyright (c) 2005-2012 Ryan C. Gordon and others.

  This software is provided 'as-is', without any express or implied warranty.
  In no event will the authors be held liable for any damages arising from
  the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in a
  product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.

     Ryan C. Gordon <icculus@icculus.org>
 */

#include "rawinput.h"

#include "scancodes.h"

/* WinUser.h won't include rawinput stuff without this... */
#if (_WIN32_WINNT < 0x0501)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <setupapi.h>

#define MAX_DEVICES 256
#define MAX_KEYS 256

#define HID_USAGE_PAGE_GENERIC 1
#define HID_USAGE_GENERIC_MOUSE 2
#define HID_USAGE_GENERIC_KEYBOARD 6

#define RI_MOUSE_HWHEEL 0x0800

#define MAX_EVENTS 1024
static GE_Event raw_events[MAX_EVENTS];
static volatile int raw_events_read = 0;
static volatile int raw_events_write = 0;
#define INC_INDEX(INDEX) (INDEX) = (((INDEX) + 1) % MAX_EVENTS);

static HWND raw_hwnd = NULL;
static const char * class_name = RAWINPUT_CLASS_NAME;
static const char * win_name = RAWINPUT_WINDOW_NAME;
static ATOM class_atom = 0;

static unsigned int available_mice = 0;
static unsigned int available_keyboards = 0;

static struct {
    HANDLE handle;
    char * name;
} mice[MAX_DEVICES] = {};

static struct {
    HANDLE handle;
    char * name;
    unsigned char * keystates;
} keyboards[MAX_DEVICES] = {};

static int (*event_callback)(GE_Event*) = NULL;

static void queue_event(const GE_Event * event) {

  raw_events[raw_events_write] = *event;
  INC_INDEX(raw_events_write)
  if (raw_events_write == raw_events_read) {
    //TODO MLA: log error
    INC_INDEX(raw_events_read)
  }
}

static void queue_from_rawinput(const RAWINPUT * raw, UINT align) {

    unsigned int device;
    const RAWINPUTHEADER * header = &raw->header;
    const RAWMOUSE * mouse = (void*)&raw->data.mouse + align;
    const RAWKEYBOARD * keyboard = (void*)&raw->data.keyboard + align;
    GE_Event event = {};
    UINT scanCode;

    if (raw->header.dwType == RIM_TYPEMOUSE) {

      for (device = 0; device < available_mice; ++device) {
        if (mice[device].handle == header->hDevice) {
          break;
        }
      }

      if (device == available_mice) {
        return;
      }

      if (mouse->usFlags & MOUSE_MOVE_RELATIVE) {
          event.type = GE_MOUSEMOTION;
          event.motion.which = device;
          if (mouse->lLastX != 0) {
            event.motion.xrel = mouse->lLastX;
            event.motion.yrel = 0;
            queue_event(&event);
          }
          if (mouse->lLastY != 0) {
            event.motion.xrel = 0;
            event.motion.yrel = mouse->lLastY;
            queue_event(&event);
          }
      }

      #define QUEUE_BUTTON(ID,BUTTON) { \
        if (mouse->usButtonFlags & RI_MOUSE_BUTTON_##ID##_DOWN) { \
          event.type = GE_MOUSEBUTTONDOWN; \
          event.button.which = device; \
          event.button.button = BUTTON; \
          queue_event(&event); \
        } \
        if (mouse->usButtonFlags & RI_MOUSE_BUTTON_##ID##_UP) { \
          event.type = GE_MOUSEBUTTONUP; \
          event.button.which = device; \
          event.button.button = BUTTON; \
          queue_event(&event); \
        } \
      }

      QUEUE_BUTTON(1, GE_BTN_LEFT)
      QUEUE_BUTTON(2, GE_BTN_RIGHT)
      QUEUE_BUTTON(3, GE_BTN_MIDDLE)
      QUEUE_BUTTON(4, GE_BTN_BACK)
      QUEUE_BUTTON(5, GE_BTN_FORWARD)

      #undef QUEUE_BUTTON
      
      #define QUEUE_WHEEL_BUTTON(WHEEL,BUTTON_PLUS,BUTTON_MINUS) { \
        if (mouse->usButtonFlags & RI_MOUSE_##WHEEL) { \
          if (mouse->usButtonData != 0) { \
            event.type = GE_MOUSEBUTTONDOWN; \
            event.button.which = device; \
            event.button.button = ((SHORT) mouse->usButtonData) > 0 ? BUTTON_PLUS : BUTTON_MINUS; \
            queue_event(&event); \
          } \
        } \
      }\

      QUEUE_WHEEL_BUTTON(WHEEL, GE_BTN_WHEELUP, GE_BTN_WHEELDOWN)
      QUEUE_WHEEL_BUTTON(HWHEEL, GE_BTN_WHEELRIGHT, GE_BTN_WHEELLEFT)
      
      #undef QUEUE_WHEEL_BUTTON

    } else if(raw->header.dwType == RIM_TYPEKEYBOARD) {

      for (device = 0; device < available_keyboards; device++) {
        if (keyboards[device].handle == header->hDevice) {
          break;
        }
      }

      if (device == available_keyboards) {
        return;
      }
      
      scanCode = get_keycode(keyboard->Flags, keyboard->MakeCode);
            
      if(keyboard->Flags & RI_KEY_BREAK) {
        keyboards[device].keystates[scanCode] = 0;
      } else if(keyboards[device].keystates[scanCode] == 0) {
        keyboards[device].keystates[scanCode] = 1;
      } else {
        return;
      }
    
      event.key.which = device;
      event.key.keysym = scanCode;
      if(keyboard->Flags & RI_KEY_BREAK) {
        event.key.type = GE_KEYUP;
      } else {
        event.key.type = GE_KEYDOWN;
      }
      queue_event(&event);

    } else {
      return;
    }
}


static void wminput_handler(WPARAM wParam, LPARAM lParam)
{
  UINT dwSize = 0;
  LPBYTE lpb;

  GetRawInputData((HRAWINPUT) lParam, RID_INPUT, NULL, &dwSize, sizeof (RAWINPUTHEADER));

  if (dwSize == 0) {
    //TODO MLA: log error
    return;
  }

  lpb = (LPBYTE) malloc(dwSize);
  if (lpb == NULL) {
      //TODO MLA: log error
      return;
  }

  if (GetRawInputData((HRAWINPUT) lParam, RID_INPUT, lpb, &dwSize, sizeof (RAWINPUTHEADER)) != dwSize) {
      //TODO MLA: log error
      return;
  }

  queue_from_rawinput((RAWINPUT *) lpb, 0);
}

BOOL bIsWow64 = FALSE;

void wminput_handler_buff()
{
  UINT i;
  static RAWINPUT RawInputs[MAX_EVENTS];
  UINT cbSize = sizeof(RawInputs);
      
  UINT nInput = GetRawInputBuffer(RawInputs, &cbSize, sizeof(RAWINPUTHEADER));
  
  if (nInput == (UINT)-1) {
    return;
  }
  
  for (i = 0; i < nInput; ++i) {
    if(bIsWow64) {
      queue_from_rawinput((RAWINPUT *) RawInputs + i, 8);
    } else {
      queue_from_rawinput((RAWINPUT *) RawInputs + i, 0);
    }
  }
}

/*
 * For some reason GetRawInputBuffer does not work in Windows 8:
 * it seems there remain WM_INPUT messages in the queue, which
 * make the MsgWaitForMultipleInput always return immediately.
 */
int buff = 0;

static LRESULT CALLBACK RawWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

    if (Msg == WM_DESTROY) {
        return 0;
    }

    if (Msg == WM_INPUT) {
        if(!buff) {
            wminput_handler(wParam, lParam);
        }
    }

    return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static int init_event_queue(void)
{
    raw_events_read = raw_events_write = 0;

    HANDLE hInstance = GetModuleHandle(NULL);
    
    WNDCLASSEX wce = {
      .cbSize = sizeof(WNDCLASSEX),
      .lpfnWndProc = RawWndProc,
      .lpszClassName = class_name,
      .hInstance = hInstance,
    };
    class_atom = RegisterClassEx(&wce);
    if (class_atom == 0)
        return 0;
    
    //create the window at the position of the cursor
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    
    raw_hwnd = CreateWindow(class_name, win_name, WS_POPUP | WS_VISIBLE | WS_SYSMENU, cursor_pos.x, cursor_pos.y, 1, 1, NULL, NULL, hInstance, NULL);

    if (raw_hwnd == NULL) {
      //TODO MLA: log error
      return 0;
    }

    RAWINPUTDEVICE rid[2] = {
      {
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage = 2,
        .dwFlags = RIDEV_NOLEGACY,
        .hwndTarget = raw_hwnd
      },
      {
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage = 6,
        .dwFlags = RIDEV_NOLEGACY,
        .hwndTarget = raw_hwnd
      }
    };
    
    if (!RegisterRawInputDevices(rid, 2, sizeof (*rid))) {
        //TODO MLA: log error
        return 0;
    }
    
    ShowWindow(raw_hwnd, SW_SHOW);

    return 1;
}

static void cleanup_window(void) {

  if (raw_hwnd) {
      MSG Msg;
      DestroyWindow(raw_hwnd);
      while (PeekMessage(&Msg, raw_hwnd, 0, 0, PM_REMOVE)) {
          TranslateMessage(&Msg);
          DispatchMessage(&Msg);
      }
      raw_hwnd = 0;
  }

  if (class_atom) {
      UnregisterClass(class_name, GetModuleHandle(NULL));
      class_atom = 0;
  }
}

static struct {
  char * instanceId;
  SP_DEVINFO_DATA data;
} devinfos[MAX_DEVICES] = {};

static unsigned int nb_devinfos = 0;

static HDEVINFO hdevinfo = INVALID_HANDLE_VALUE;

static int get_devinfos() {
  
  const DWORD flags = DIGCF_ALLCLASSES | DIGCF_PRESENT;
  hdevinfo = SetupDiGetClassDevs(NULL, NULL, NULL, flags);
  if (hdevinfo == INVALID_HANDLE_VALUE) {
    //TODO MLA: log error
    return -1;
  }
  
  DWORD i = 0;
  while (1) {
    SP_DEVINFO_DATA data = { .cbSize = sizeof(SP_DEVINFO_DATA) };
    BOOL result = SetupDiEnumDeviceInfo(hdevinfo, i++, &data);
    if (result == TRUE) {
      DWORD bufsize;
      result = SetupDiGetDeviceInstanceId(hdevinfo, &data, NULL, 0, &bufsize);
      if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        char * buf = malloc(bufsize);
        if (buf == NULL) {
          //TODO MLA: log error
          continue;
        }
        result = SetupDiGetDeviceInstanceId(hdevinfo, &data, buf, bufsize, NULL);
        if (result == TRUE) {
          devinfos[nb_devinfos].instanceId = buf;
          devinfos[nb_devinfos].data = data;
          ++nb_devinfos;
        } else {
          //TODO MLA: log error
          free(buf);
        }
      } else {
        //TODO MLA: log error
      }
    } else if (GetLastError() == ERROR_NO_MORE_ITEMS) {
      break;
    }
  }

  return 0;
}

void free_dev_info() {
  if (hdevinfo != INVALID_HANDLE_VALUE) {
    SetupDiDestroyDeviceInfoList(hdevinfo);
  }
  unsigned int i;
  for (i = 0; i < nb_devinfos; ++i) {
    free(devinfos[i].instanceId);
    memset(devinfos + i, 0x00, sizeof(*devinfos));
  }
  nb_devinfos = 0;
}

static SP_DEVINFO_DATA * get_devinfo_data(const char * instanceId) {
  
  unsigned int i;
  for (i = 0; i < nb_devinfos; ++i) {
    if (strcasecmp(instanceId, devinfos[i].instanceId) == 0) {
      return &devinfos[i].data;
    }
  }
  return NULL;
}

static char * get_dev_name_by_instance(const char * devinstance) {

  char * name = NULL;

  SP_DEVINFO_DATA * devdata = get_devinfo_data(devinstance);
  if (devdata != NULL) {
    DWORD size;
    BOOL result = SetupDiGetDeviceRegistryProperty(hdevinfo, devdata, SPDRP_DEVICEDESC, NULL, NULL, 0, &size);
    if (result == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      name = malloc(size);
      if (name != NULL) {
        result = SetupDiGetDeviceRegistryProperty(hdevinfo, devdata, SPDRP_DEVICEDESC, NULL, (PBYTE) name, size, NULL);
        if (result == FALSE) {
          //TODO MLA: log error
          free(name);
          name = NULL;
        }
      } else {
        //TODO MLA: log error
      }
    } else {
      //TODO MLA: log error
    }
  } else {
    //TODO MLA: log error
  }
  
  return name;
}

static void init_device(const RAWINPUTDEVICELIST * dev) {

  if (dev->dwType != RIM_TYPEMOUSE && dev->dwType != RIM_TYPEKEYBOARD) {
    return;
  }

  UINT count = 0;
  if (GetRawInputDeviceInfo(dev->hDevice, RIDI_DEVICENAME, NULL, &count) == (UINT)-1) {
    //TODO MLA: log error
    return;
  }

  char * buf = (char *) calloc(count + 1, sizeof(char));
  if (buf == NULL) {
    //TODO MLA: log error
    return;
  }

  if (GetRawInputDeviceInfo(dev->hDevice, RIDI_DEVICENAME, buf, &count) == (UINT)-1) {
    //TODO MLA: log error
    return;
  }
  
  // skip remote desktop devices
  if (strstr(buf, "Root#RDP_")) {
    return;
  }

  // XP starts these strings with "\\??\\" ... Vista does "\\\\?\\".  :/
  while ((*buf == '?') || (*buf == '\\')) {
    buf++;
    count--;
  }

  // get the device instance id
  char * ptr;
  for (ptr = buf; *ptr; ptr++) {
    if (*ptr == '#') {
      *ptr = '\\'; // convert '#' to '\\'
    } else if (*ptr == '{') { // GUID part
      if (*(ptr-1) == '\\') {
        ptr--;
      }
      break;
    }
  }
  *ptr = '\0';
  
  if (dev->dwType == RIM_TYPEMOUSE) {
    mice[available_mice].name = get_dev_name_by_instance(buf);
    if (mice[available_mice].name != NULL) {
      mice[available_mice].handle = dev->hDevice;
      available_mice++;
    } else {
      //TODO MLA: log error
    }
  }
  else if(dev->dwType == RIM_TYPEKEYBOARD) {
    keyboards[available_keyboards].name = get_dev_name_by_instance(buf);
    if (keyboards[available_keyboards].name != NULL) {
      keyboards[available_keyboards].keystates = calloc(MAX_KEYS, sizeof(unsigned char));
      if (keyboards[available_keyboards].keystates != NULL) {
        keyboards[available_keyboards].handle = dev->hDevice;
        available_keyboards++;
      } else {
        //TODO MLA: log error
        free(keyboards[available_keyboards].name);
        keyboards[available_keyboards].name = NULL;
      }
    }
  }
}

int rawinput_init(int (*callback)(GE_Event*)) {

  event_callback = callback;

  available_mice = 0;
  available_keyboards = 0;
  
  if (get_devinfos() < 0) {
    return -1;
  }

  UINT count = 0;
  UINT result = GetRawInputDeviceList(NULL, &count, sizeof(RAWINPUTDEVICELIST));
  if (result == (UINT)-1) {
    //TODO MLA: log error
  } else if (count > 0) {
    RAWINPUTDEVICELIST * devlist = (PRAWINPUTDEVICELIST) malloc(count * sizeof(RAWINPUTDEVICELIST));
    result = GetRawInputDeviceList(devlist, &count, sizeof(RAWINPUTDEVICELIST));
    if (result != (UINT)-1) {
      unsigned int i;
      for (i = 0; i < count; i++) {
        init_device(&devlist[i]);
      }
    }
    free(devlist);
  }
  
  free_dev_info();
  
  if(result == (UINT)-1) {
    return -1;
  }
  
  if (!init_event_queue()) {
    cleanup_window();
    available_mice = 0;
    available_keyboards = 0;
  }
  
  IsWow64Process(GetCurrentProcess(), &bIsWow64);

  return 0;
}

void rawinput_quit(void) {

  RAWINPUTDEVICE rid[2] = {
    {
      .usUsagePage = HID_USAGE_PAGE_GENERIC,
      .usUsage = HID_USAGE_GENERIC_MOUSE,
      .dwFlags = RIDEV_REMOVE,
    },
    {
      .usUsagePage = HID_USAGE_PAGE_GENERIC,
      .usUsage = HID_USAGE_GENERIC_KEYBOARD,
      .dwFlags = RIDEV_REMOVE
    }
  };
  RegisterRawInputDevices(rid, sizeof(rid) / sizeof(*rid), sizeof(*rid));
  cleanup_window();
  unsigned int i;
  for(i = 0; i < available_keyboards; ++i) {
    free(keyboards[i].name);
    free(keyboards[i].keystates);
    memset(keyboards + i, 0x00, sizeof(*keyboards));
  }
  available_keyboards = 0;
  for(i = 0; i < available_mice; ++i) {
    free(mice[i].name);
    memset(mice + i, 0x00, sizeof(*mice));
  }
  available_mice = 0;
}

const char * rawinput_mouse_name(unsigned int index) {
  return (index < available_mice) ? mice[index].name : NULL;
}

const char * rawinput_keyboard_name(unsigned int index) {
  return (index < available_keyboards) ? keyboards[index].name : NULL;
}

void rawinput_poll() {

  MSG Msg;
  
  if(buff) {

    /* process WM_INPUT events */
    wminput_handler_buff();
    
    /* process all other events, otherwise the message queue quickly gets full */
    while (PeekMessage(&Msg, raw_hwnd, 0, WM_INPUT-1, PM_REMOVE)) {
        DefWindowProc(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
    }
    while (PeekMessage(&Msg, raw_hwnd, WM_INPUT+1, 0xFFFF, PM_REMOVE)) {
        DefWindowProc(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
    }
  } else {

    /* process all events */
    while (PeekMessage(&Msg, raw_hwnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&Msg);
      /* process messages including WM_INPUT ones */
      DispatchMessage(&Msg);
    }
  }

  while (raw_events_read != raw_events_write) {
    event_callback(raw_events + raw_events_read);
    INC_INDEX(raw_events_read)
  }
}
