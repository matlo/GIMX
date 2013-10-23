/*
 * ManyMouse main header. Include this from your app.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#ifndef _INCLUDE_MANYMOUSE_H_
#define _INCLUDE_MANYMOUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MANYMOUSE_VERSION "0.0.3"

typedef enum
{
    MANYMOUSE_EVENT_ABSMOTION = 0,
    MANYMOUSE_EVENT_RELMOTION,
    MANYMOUSE_EVENT_BUTTON,
    MANYMOUSE_EVENT_SCROLL,
    MANYMOUSE_EVENT_DISCONNECT,
    MANYMOUSE_EVENT_KEY,
    MANYMOUSE_EVENT_MAX
} ManyMouseEventType;

typedef struct
{
    ManyMouseEventType type;
    unsigned int device;
    unsigned int item;
    int value;
    unsigned int scancode;
    int minval;
    int maxval;
} ManyMouseEvent;


/* internal use only. */
typedef struct
{
    const char *driver_name;
    int (*init)(void);
    void (*quit)(void);
    const char *(*mouse_name)(unsigned int index);
    int (*poll)(ManyMouseEvent *event);
    const char *(*keyboard_name)(unsigned int index);
} ManyMouseDriver;


int ManyMouse_Init(void);
const char *ManyMouse_DriverName(void);
void ManyMouse_Quit(void);
const char *ManyMouse_MouseName(unsigned int index);
int ManyMouse_PollEvent(ManyMouseEvent *event);
const char *ManyMouse_KeyboardName(unsigned int index);

#ifdef __cplusplus
}
#endif

#endif  /* !defined _INCLUDE_MANYMOUSE_H_ */

/* end of manymouse.h ... */

