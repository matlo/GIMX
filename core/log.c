/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimx.h>
#include "../info.h"
#include <stdlib.h>
#if defined(_WIN32)
#include <windows.h>
#include <VersionHelpers.h>
#endif

void print_version() {
  printf("GIMX %s %s\n", INFO_VERSION, INFO_ARCH);
}

void print_os() {
#if defined(__linux__)
  if (system("lsb_release -d -s") != 0) {
    fprintf(stderr, "cannot get distribution information\n");
  }
#elif defined(_WIN32)
  printf("Windows ");
  const char * version = "(unknown version)";
  if (IsWindowsVersionOrGreater(10, 0, 0)) {
    version = "10 (or later)";
  } else if (IsWindows8Point1OrGreater()) {
    version = "8.1";
  } else if (IsWindows8OrGreater()) {
    version = "8";
  } else if (IsWindows7SP1OrGreater()) {
    version = "7 SP1";
  } else if (IsWindows7OrGreater()) {
    version = "7";
  } else if (IsWindowsVistaSP2OrGreater()) {
    version = "Vista SP2";
  } else if (IsWindowsVistaSP1OrGreater()) {
    version = "Vista SP1";
  } else if (IsWindowsVistaOrGreater()) {
    version = "Vista";
  } else if (IsWindowsXPSP3OrGreater()) {
    version = "XP SP3";
  } else if (IsWindowsXPSP2OrGreater()) {
    version = "XP SP2";
  } else if (IsWindowsXPSP1OrGreater()) {
    version = "XP SP1";
  } else if (IsWindowsXPOrGreater()) {
    version = "XP";
  } else {
    version = "older than XP";
  }
  printf("%s", version);
  SYSTEM_INFO info;
  GetNativeSystemInfo(&info);
  switch (info.wProcessorArchitecture) {
  case PROCESSOR_ARCHITECTURE_AMD64:
    printf(" x64");
    break;
  case PROCESSOR_ARCHITECTURE_ARM:
    printf(" ARM");
    break;
  case PROCESSOR_ARCHITECTURE_IA64:
    printf(" ia64");
    break;
  case PROCESSOR_ARCHITECTURE_INTEL:
    printf(" x86");
    break;
  case PROCESSOR_ARCHITECTURE_UNKNOWN:
    printf(" unknown architecture");
    break;
  }
  printf("\n");
#endif
}

void print_kernel() {
#ifdef __linux__
  if (system("uname -srvp") != 0) {
    fprintf(stderr, "cannot get kernel information\n");
  }
#elif defined(_WIN32)
  printf("Kernel version: ");
  DWORD dwVersion = GetVersion();
  DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));;
  DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));;
  DWORD dwBuild = 0;
  if (dwVersion < 0x80000000) {
    dwBuild = (DWORD)(HIWORD(dwVersion));
  }
  printf("%d.%d (%d)\n", dwMajorVersion, dwMinorVersion, dwBuild);
#endif
}

void print_devices() {
#ifdef __linux__
  if (system("lsusb") != 0) {
    fprintf(stderr, "cannot get USB device list\n");
  }
  if (system("lsusb -t") != 0) {
    fprintf(stderr, "cannot get USB device tree\n");
  }
#elif defined(_WIN32)
#endif
}

void log_info() {
  print_version();
  print_os();
  print_kernel();
  print_devices();
}
