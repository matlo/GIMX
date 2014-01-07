#pragma once

/* -------------------------------------------------------------------------- */
/*   DEFINES                                                                  */
/* -------------------------------------------------------------------------- */
#define     CONTROLLER_DISCONNECTED     0x00
#define     CONTROLLER_PS3              0x10
#define     CONTROLLER_XB360            0x20
#define     CONTROLLER_WII              0x30
#define     NUNCHUK_EXTENSION           0x01
#define     CLASSIC_EXTENSION           0x02

#define     CONSOLE_DISCONNECTED           0
#define     CONSOLE_PS3                    1
#define     CONSOLE_XB360                  2
#define     CONSOLE_PS4                    3
#define     CONSOLE_XB1                    4

#define     GPP_LED_OFF                     0
#define     GPP_LED_ON                      1
#define     GPP_LED_BLINK_FAST              2
#define     GPP_LED_BLINK_SLOW              3

#define     PS3_PS                      0
#define     PS3_SELECT                  1
#define     PS3_START                   2
#define     PS3_R1                      3
#define     PS3_R2                      4
#define     PS3_R3                      5
#define     PS3_L1                      6
#define     PS3_L2                      7
#define     PS3_L3                      8
#define     PS3_RX                      9
#define     PS3_RY                      10
#define     PS3_LX                      11
#define     PS3_LY                      12
#define     PS3_UP                      13
#define     PS3_DOWN                    14
#define     PS3_LEFT                    15
#define     PS3_RIGHT                   16
#define     PS3_TRIANGLE                17
#define     PS3_CIRCLE                  18
#define     PS3_CROSS                   19
#define     PS3_SQUARE                  20
#define     PS3_ACCX                    21
#define     PS3_ACCY                    22
#define     PS3_ACCZ                    23
#define     PS3_GYRO                    24

#define     XB360_XBOX                  0
#define     XB360_BACK                  1
#define     XB360_START                 2
#define     XB360_RB                    3
#define     XB360_RT                    4
#define     XB360_RA                    5
#define     XB360_LB                    6
#define     XB360_LT                    7
#define     XB360_LA                    8
#define     XB360_RX                    9
#define     XB360_RY                    10
#define     XB360_LX                    11
#define     XB360_LY                    12
#define     XB360_UP                    13
#define     XB360_DOWN                  14
#define     XB360_LEFT                  15
#define     XB360_RIGHT                 16
#define     XB360_Y                     17
#define     XB360_B                     18
#define     XB360_A                     19
#define     XB360_X                     20

#define     WII_HOME                     0
#define     WII_MINUS                    1
#define     WII_PLUS                     2
#define     WII_ONE                      5
#define     WII_TWO                      8
#define     WII_UP                      13
#define     WII_DOWN                    14
#define     WII_LEFT                    15
#define     WII_RIGHT                   16
#define     WII_B                       18
#define     WII_A                       19
#define     WII_ACCX                    21
#define     WII_ACCY                    22
#define     WII_ACCZ                    23
#define     WII_IRX                     28
#define     WII_IRY                     29

#define     WII_C                        6
#define     WII_Z                        7
#define     WII_NX                      11
#define     WII_NY                      12
#define     WII_ACCNX                   25
#define     WII_ACCNY                   26
#define     WII_ACCNZ                   27

#define     WII_RT                       3
#define     WII_ZR                       4
#define     WII_LT                       6
#define     WII_ZL                       7
#define     WII_RX                       9
#define     WII_RY                      10
#define     WII_LX                      11
#define     WII_LY                      12
#define     WII_X                       17
#define     WII_Y                       20

#define     REP_IO_COUNT                30

/* -------------------------------------------------------------------------- */
/*   TYPEDEFS                                                                 */
/* -------------------------------------------------------------------------- */
typedef     char                        int8;
typedef     unsigned char               uint8;
typedef     unsigned short              uint16;

/* -------------------------------------------------------------------------- */
/*   STRUCTS                                                                  */
/* -------------------------------------------------------------------------- */
struct gppReport {
    uint8 console;               // Range: [0 ~ 2] CONSOLE_DISCONNECTED | CONSOLE_PS3 | CONSOLE_XB360
    uint8 controller;            // Logic AND (& 0xF0) == Controller; Logic AND (& 0x0F) == Extension
    uint8 led[4];                // Range: [0 ~ 3] GPP_LED_OFF | GPP_LED_ON | GPP_LED_BLINK_FAST | GPP_LED_BLINK_SLOW
    uint8 rumble[2];             // Range: [0 ~ 100] %
    uint8 battery_level;         // Range: [0 ~ 10] 0 = empty, 10 = full
    int8 input[REP_IO_COUNT];    // Input Entries (from controller)
};

/* -------------------------------------------------------------------------- */
/*   FUNCTIONS PROTOTYPES                                                     */
/* -------------------------------------------------------------------------- */
int8 gppcprog_connect();
int8 gppcprog_connected();
void gppcprog_disconnect();
int8 gpppcprog_input(struct gppReport *report, int timeout);
int8 gpppcprog_output(int8 *output);

