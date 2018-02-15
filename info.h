#ifndef INFO_H_
#define INFO_H_

#define INFO_VERSION "7.2"
#define INFO_YEAR "2010-2018"
#define INFO_DEV "Matlo"
#define INFO_WEB "http://gimx.fr"
#define INFO_LICENCE "GNU GPL v3"

#ifdef __x86_64__
  #define INFO_ARCH "x86_64"
#endif

#ifdef __i386__
  #define INFO_ARCH "i386"
#endif

#ifdef __ARM_ARCH_6__
  #define INFO_ARCH "ARMv6"
#endif

#ifndef INFO_ARCH
  #define INFO_ARCH ""
#endif

#endif /* INFO_H_ */
