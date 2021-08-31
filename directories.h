/*
 * directories.h
 *
 *  Created on: 28 févr. 2012
 *      Author: matlo
 */

#ifndef DIRECTORIES_H_
#define DIRECTORIES_H_

#ifndef WIN32
#define OLD_GIMX_DIR "/.emuclient/"
#else
#define OLD_GIMX_DIR "/emuclient/"
#endif

#ifndef WIN32
#define GIMX_DIR "/.gimx/"
#else
#define GIMX_DIR "/gimx/"
#endif

#define CONFIG_DIR "config/"
#define MACRO_DIR "macros/"
#define BT_DIR "bluetooth/"
#define LOG_DIR "log/"

#ifndef WIN32
#define FIRMWARE_DIR "/usr/share/gimx/firmware"
#else
#define FIRMWARE_DIR "firmware"
#endif

#ifndef WIN32
#define FIRMWARE_DIR "/usr/share/gimx/firmware"
#else
#define FIRMWARE_DIR "firmware"
#endif

#ifndef WIN32
#define IMAGE_DIR "/usr/share/gimx/image"
#else
#define IMAGE_DIR "image"
#endif

#endif /* DIRECTORIES_H_ */
