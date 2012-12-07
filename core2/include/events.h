
#ifndef WIN32
#include <linux/input.h>
#endif
#include <SDL/SDL.h>

#define MAX_DEVICES 256

#define DEVTYPE_KEYBOARD 0x01
#define DEVTYPE_MOUSE    0x02
#define DEVTYPE_JOYSTICK 0x04
#define DEVTYPE_NB       3

#define MAX_KEYNAMES 249

extern char* keynames[MAX_KEYNAMES];

void ev_init();
void ev_quit();
char* ev_get_name(unsigned char, int);
