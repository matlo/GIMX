/*
 * xml_defs.h
 *
 *  Created on: 13 nov. 2013
 *      Author: matlo
 */

#ifndef XML_DEFS_H_
#define XML_DEFS_H_

#define X_NODE_ROOT "root"
#define X_NODE_CONTROLLER "controller"
#define X_NODE_CONFIGURATION "configuration"
#define X_NODE_TRIGGER "trigger"
#define X_NODE_INTENSITY_LIST "intensity_list"
#define X_NODE_INTENSITY "intensity"
#define X_NODE_BUTTON_MAP "button_map"
#define X_NODE_AXIS_MAP "axis_map"
#define X_NODE_DEVICE "device"
#define X_NODE_EVENT "event"
#define X_NODE_AXIS "axis"
#define X_NODE_BUTTON "button"
#define X_NODE_UP "up"
#define X_NODE_DOWN "down"

#define X_ATTR_ID "id"
#define X_ATTR_DPI "dpi"
#define X_ATTR_TYPE "type"
#define X_ATTR_NAME "name"
#define X_ATTR_BUTTON_ID "button_id"
#define X_ATTR_THRESHOLD "threshold"
#define X_ATTR_DEADZONE "dead_zone"
#define X_ATTR_MULTIPLIER "multiplier"
#define X_ATTR_EXPONENT "exponent"
#define X_ATTR_SHAPE "shape"
#define X_ATTR_BUFFERSIZE "buffer_size"
#define X_ATTR_FILTER "filter"
#define X_ATTR_SWITCH_BACK "switch_back"
#define X_ATTR_DELAY "delay"
#define X_ATTR_STEPS "steps"
#define X_ATTR_CONTROL "control"

#define X_ATTR_VALUE_KEYBOARD "keyboard"
#define X_ATTR_VALUE_MOUSE "mouse"
#define X_ATTR_VALUE_JOYSTICK "joystick"
#define X_ATTR_VALUE_BUTTON "button"
#define X_ATTR_VALUE_AXIS "axis"
#define X_ATTR_VALUE_AXIS_DOWN "axis down"
#define X_ATTR_VALUE_AXIS_UP "axis up"
#define X_ATTR_VALUE_CIRCLE "Circle"
#define X_ATTR_VALUE_RECTANGLE "Rectangle"
#define X_ATTR_VALUE_YES "yes"
#define X_ATTR_VALUE_NO "no"

#define X_NODE_MOUSE_OPTIONS_LIST "mouse_options_list"
#define X_NODE_MOUSE "mouse"
#define X_ATTR_MODE "mode"
#define X_ATTR_VALUE_AIMING "Aiming"
#define X_ATTR_VALUE_DRIVING "Driving"

#define X_NODE_JOYSTICK_CORRECTIONS_LIST "joystick_corrections_list"
#define X_NODE_CORRECTION "correction"
#define X_ATTR_LOW_VALUE "low_value"
#define X_ATTR_LOW_COEF "low_coef"
#define X_ATTR_HIGH_VALUE "high_value"
#define X_ATTR_HIGH_COEF "high_coef"

#define X_NODE_FORCE_FEEDBACK "force_feedback"
#define X_NODE_INVERSION "inversion"
#define X_ATTR_ENABLE "enable"
#define X_NODE_GAIN "gain"
#define X_ATTR_RUMBLE "rumble"
#define X_ATTR_CONSTANT "constant"
#define X_ATTR_SPRING "spring"
#define X_ATTR_DAMPER "damper"

#define X_NODE_MACROS "macros"

#endif /* XML_DEFS_H_ */
