/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include "config_reader.h"
#include <xml_defs.h>
#include "config.h"
#include <ginput.h>
#include "calibration.h"
#include <limits.h>
#include <dirent.h>
#include <libxml/xmlreader.h>
#include <iconv.h>
#include "gimx.h"
#include <adapter.h>
#include "../directories.h"
#include "macros.h"
#include <errno.h>

#ifdef WIN32
#include <sys/stat.h>
#define LINE_MAX 1024
#endif

/*
 * These variables are used to read the configuration.
 */
static s_config_entry entry;

static void reset_entry()
{
  memset(&entry.device, 0x00, sizeof(entry.device));
  memset(&entry.event, 0x00, sizeof(entry.event));
  memset(&entry.params, 0x00, sizeof(entry.params));
}

static char r_device_name[128];

const char* _UTF8_to_8BIT(const char* _utf8)
{
  iconv_t cd;
  char* input = (char*)_utf8;
  size_t in = strlen(input) + 1;
  static char output[256];
  char* poutput = output;
  size_t out = sizeof(output);
  cd = iconv_open ("ISO-8859-1//TRANSLIT", "UTF-8");
  iconv(cd, &input, &in, &poutput, &out);
  iconv_close(cd);
  return output;
}

/*
 * Get the device name and store it into r_device_name.
 * OK, return 0
 * error, return -1
 */
int GetDeviceName(xmlNode* a_node)
{
  int ret = 0;
  char* prop;

  prop = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
  if(prop)
  {
    strcpy(r_device_name, prop);
  }
  else
  {
    ret = -1;
  }
  xmlFree(prop);


  return ret;
}

static void warnDeviceNotFound()
{
  static unsigned char warned[E_DEVICE_TYPE_NB][MAX_DEVICES] = {};
  int type_index = entry.device.type - 1;
  if(type_index < 0 || type_index >= E_DEVICE_TYPE_NB || warned[type_index][entry.device.id] != 0)
  {
    return;
  }
  switch(entry.device.type)
  {
  case E_DEVICE_TYPE_JOYSTICK:
    gprintf(_("joystick not found: %s %d\n"), _UTF8_to_8BIT(r_device_name), entry.device.id);
    break;
  case E_DEVICE_TYPE_MOUSE:
    gprintf(_("mouse not found: %s %d\n"), _UTF8_to_8BIT(r_device_name), entry.device.id);
    break;
  case E_DEVICE_TYPE_KEYBOARD:
    gprintf(_("keyboard not found: %s %d\n"), _UTF8_to_8BIT(r_device_name), entry.device.id);
    break;
  case E_DEVICE_TYPE_UNKNOWN:
    return;
  }
  warned[type_index][entry.device.id] = 1;
}

/*
 * Get the device id and store it into binding.device.id.
 * OK, return 0
 * error, return -1
 * device not found, return 1
 */
static int GetDeviceId(xmlNode* a_node)
{
  int i;
  int ret;

  ret = GetIntProp(a_node, X_ATTR_ID, &entry.device.id);

  if(entry.device.id < 0 || entry.device.id >= MAX_DEVICES)
  {
    ret = -1;
  }

  if(ret != -1)
  {
    if(entry.device.type == E_DEVICE_TYPE_JOYSTICK)
    {
      for (i = 0; i < MAX_DEVICES && ginput_joystick_name(i); ++i)
      {
        if (!strcmp(r_device_name, ginput_joystick_name(i)))
        {
          if (entry.device.id == ginput_joystick_virtual_id(i))
          {
            entry.device.id = i;
            ginput_set_joystick_used(i);
#ifndef WIN32
            entry.device.hid = ginput_joystick_get_hid(i);
#else
            entry.device.usb_ids.vendor = 0;
            entry.device.usb_ids.product = 0;
            ginput_joystick_get_usb_ids(i, &entry.device.usb_ids.vendor, &entry.device.usb_ids.product);
#endif
            break;
          }
        }
      }
      if(i == MAX_DEVICES || !ginput_joystick_name(i))
      {
        warnDeviceNotFound();
        ret = 1;
      }
    }
    else if(ginput_get_mk_mode() == GE_MK_MODE_SINGLE_INPUT)
    {
      entry.device.id = 0;
    }
    else if(!strlen(r_device_name))
    {
      if(ginput_get_mk_mode() == GE_MK_MODE_MULTIPLE_INPUTS)
      {
        gprintf(_("A device name is empty. Multiple mice and keyboards are not managed.\n"));
      }
      ginput_set_mk_mode(GE_MK_MODE_SINGLE_INPUT);
    }
    else
    {
      if(entry.device.type == E_DEVICE_TYPE_MOUSE)
      {
        for (i = 0; i < MAX_DEVICES && ginput_mouse_name(i); ++i)
        {
          if (!strcmp(r_device_name, ginput_mouse_name(i)))
          {
            if (entry.device.id == ginput_mouse_virtual_id(i))
            {
              entry.device.id = i;
              break;
            }
          }
        }
        if(i == MAX_DEVICES || !ginput_mouse_name(i))
        {
          warnDeviceNotFound();
          ret = 1;
        }
      }
      else if(entry.device.type == E_DEVICE_TYPE_KEYBOARD)
      {
        for (i = 0; i < MAX_DEVICES && ginput_keyboard_name(i); ++i)
        {
          if (!strcmp(r_device_name, ginput_keyboard_name(i)))
          {
            if (entry.device.id == ginput_keyboard_virtual_id(i))
            {
              entry.device.id = i;
              break;
            }
          }
        }
        if(i == MAX_DEVICES || !ginput_keyboard_name(i))
        {
          warnDeviceNotFound();
          ret = 1;
        }
      }
    }
  }

  return ret;
}

/*
 * Get the device type and store it into binding.device.type.
 * OK, return 0
 * error, return -1
 */
int GetDeviceTypeProp(xmlNode * a_node)
{
  int ret = 0;
  char* type;

  type = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);

  if(type)
  {
    if (!strncmp(type, X_ATTR_VALUE_KEYBOARD, strlen(X_ATTR_VALUE_KEYBOARD)))
    {
      entry.device.type = E_DEVICE_TYPE_KEYBOARD;
    }
    else if (!strncmp(type, X_ATTR_VALUE_MOUSE, strlen(X_ATTR_VALUE_MOUSE)))
    {
      entry.device.type = E_DEVICE_TYPE_MOUSE;
    }
    else if (!strncmp(type, X_ATTR_VALUE_JOYSTICK, strlen(X_ATTR_VALUE_JOYSTICK)))
    {
      entry.device.type = E_DEVICE_TYPE_JOYSTICK;
    }
    else
    {
      entry.device.type = E_DEVICE_TYPE_UNKNOWN;
    }
  }
  else
  {
    ret = -1;
  }

  xmlFree(type);
  return ret;
}

/*
 * Get the event id and store it into entry.event.id.
 * OK, return 0
 * error, return -1
 */
static int GetEventId(xmlNode * a_node, char* attr_label)
{
  int ret = 0;
  char* event_id = (char*) xmlGetProp(a_node, (xmlChar*) attr_label);

  if(!event_id)
  {
    ret = -1;
  }
  else
  {
    switch(entry.device.type)
    {
      case E_DEVICE_TYPE_KEYBOARD:
        entry.event.id = ginput_key_id(event_id);
        break;
      case E_DEVICE_TYPE_JOYSTICK:
        entry.event.id = atoi(event_id);
        break;
      case E_DEVICE_TYPE_MOUSE:
        entry.event.id = ginput_mouse_button_id(event_id);
        break;
      default:
        ret = -1;
        break;
    }
  }

  xmlFree(event_id);
  return ret;
}

int GetIntProp(xmlNode * a_node, char* a_attr, int* a_int)
{
  char* val;
  int ret;

  val = (char*)xmlGetProp(a_node, (xmlChar*) a_attr);

  if(val && strlen(val))
  {
    *a_int = atoi(val);
    ret = 0;
  }
  else
  {
    ret = -1;
  }

  xmlFree(val);
  return ret;
}

int GetUnsignedIntProp(xmlNode * a_node, char* a_attr, unsigned int* a_uint)
{
  char* val;
  int ret;

  val = (char*)xmlGetProp(a_node, (xmlChar*) a_attr);

  if(val)
  {
    *a_uint = atoi(val);
    ret = 0;
  }
  else
  {
    ret = -1;
  }

  xmlFree(val);
  return ret;
}

int GetDoubleProp(xmlNode * a_node, char* a_attr, double* a_double)
{
  char* val;
  int ret;

  val = (char*)xmlGetProp(a_node, (xmlChar*) a_attr);

  if(val)
  {
    *a_double = strtod(val, NULL);
    ret = 0;
  }
  else
  {
    ret = -1;
  }

  xmlFree(val);
  return ret;
}

static int ProcessEventElement(xmlNode * a_node, unsigned char mapper)
{
  int ret = 0;
  char* type;
  char* shape;

  type = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
  if (!strncmp(type, X_ATTR_VALUE_BUTTON, strlen(X_ATTR_VALUE_BUTTON)))
  {
    entry.event.type = E_EVENT_TYPE_BUTTON;
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS_DOWN, strlen(X_ATTR_VALUE_AXIS_DOWN)))
  {
    entry.event.type = E_EVENT_TYPE_AXIS_DOWN;
    ret = GetIntProp(a_node, X_ATTR_THRESHOLD, &entry.params.mapper.threshold);
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS_UP, strlen(X_ATTR_VALUE_AXIS_UP)))
  {
    entry.event.type = E_EVENT_TYPE_AXIS_UP;
    ret = GetIntProp(a_node, X_ATTR_THRESHOLD, &entry.params.mapper.threshold);
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS, strlen(X_ATTR_VALUE_AXIS)))
  {
    entry.event.type = E_EVENT_TYPE_AXIS;

    if(mapper)
    {
      ret = GetIntProp(a_node, X_ATTR_DEADZONE, &entry.params.mapper.dead_zone);
      if(ret == -1)
      {
        entry.params.mapper.dead_zone = 0;
      }
      ret = GetDoubleProp(a_node, X_ATTR_MULTIPLIER, &entry.params.mapper.multiplier);
      if(ret == -1)
      {
        entry.params.mapper.multiplier = 1;
      }
      ret = GetDoubleProp(a_node, X_ATTR_EXPONENT, &entry.params.mapper.exponent);
      if(ret == -1)
      {
        entry.params.mapper.exponent = 1;
        ret = 0;
      }
      shape = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
      entry.params.mapper.shape = E_SHAPE_CIRCLE;//default value
      if(shape)
      {
        if (!strncmp(shape, X_ATTR_VALUE_RECTANGLE, strlen(X_ATTR_VALUE_RECTANGLE)))
        {
          entry.params.mapper.shape = E_SHAPE_RECTANGLE;
        }
      }
      xmlFree(shape);
      /* for compatibility with old configurations */
      GetUnsignedIntProp(a_node, X_ATTR_BUFFERSIZE, &entry.params.mouse_options.buffer_size);
      GetDoubleProp(a_node, X_ATTR_FILTER, &entry.params.mouse_options.filter);
    }
  }

  if(ret == 0)
  {
    ret = GetEventId(a_node, X_ATTR_ID);

    if(ret == 0 && mapper)
    {
      switch(entry.event.type)
      {
        case E_EVENT_TYPE_BUTTON:
          entry.params.mapper.button = entry.event.id;
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_DOWN:
        case E_EVENT_TYPE_AXIS_UP:
          entry.params.mapper.axis = entry.event.id;
          break;
        default:
          ret = -1;
          break;
      }

      if(ret == 0)
      {
        ret = cfg_add_binding(&entry);

        if(ret == 0)
        {
          switch(entry.event.type)
          {
            case E_EVENT_TYPE_BUTTON:
              adapter_set_device(entry.controller_id, entry.device.type, entry.device.id);
              break;
            case E_EVENT_TYPE_AXIS:
            case E_EVENT_TYPE_AXIS_DOWN:
            case E_EVENT_TYPE_AXIS_UP:
              if(entry.device.type == E_DEVICE_TYPE_MOUSE)
              {
                s_mouse_cal* mcal = cal_get_mouse(entry.device.id, entry.config_id);
                if(!mcal->options.buffer_size)
                {
                  entry.params.mouse_options.mode = E_MOUSE_MODE_AIMING;
                  if(!entry.params.mouse_options.buffer_size)
                  {
                    entry.params.mouse_options.buffer_size = 1;
                    entry.params.mouse_options.filter = 0;
                  }
                  cal_set_mouse(&entry);
                }
              }
              else if(entry.device.type == E_DEVICE_TYPE_JOYSTICK
                      && entry.params.mapper.axis_props.axis == rel_axis_0
                      && entry.params.mapper.axis_props.props == AXIS_PROP_CENTERED)
              {
                adapter_set_haptic_joystick(entry.controller_id, entry.device.id);
#ifndef WIN32
                if(entry.device.hid >= 0)
                {
                  adapter_set_hid(entry.controller_id, entry.device.hid);
                }
#else
                if(entry.device.usb_ids.vendor && entry.device.usb_ids.product)
                {
                  adapter_set_usb_ids(entry.controller_id, entry.device.id, entry.device.usb_ids.vendor, entry.device.usb_ids.product);
                }
#endif
              }
              break;
            default:
              break;
          }
        }
      }
    }
  }

  xmlFree(type);
  return ret;
}

static int ProcessDeviceElement(xmlNode * a_node)
{
  int ret = GetDeviceTypeProp(a_node);

  if(ret != -1)
  {
    ret = GetDeviceName(a_node);

    if(ret != -1)
    {
      ret = GetDeviceId(a_node);
    }
  }

  return ret;
}

static int ProcessAxisElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;
  char* aid;

  reset_entry();

  aid = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);

  entry.params.mapper.axis_props = controller_get_axis_index_from_name(aid);

  xmlFree(aid);

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
      {
        ret = ProcessDeviceElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing device element");
    ret = -1;
  }

  for (cur_node = cur_node->next; cur_node && ret == 0; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
      {
        ret = ProcessEventElement(cur_node, 1);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing event element");
    ret = -1;
  }

  return ret;
}

static int ProcessButtonElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;
  char* bid;

  reset_entry();

  bid = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);

  entry.params.mapper.axis_props = controller_get_axis_index_from_name(bid);
  entry.params.mapper.axis_props.props = AXIS_PROP_TOGGLE;

  xmlFree(bid);

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
      {
        ret = ProcessDeviceElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing device element");
    ret = -1;
  }

  for (cur_node = cur_node->next; cur_node && ret == 0; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
      {
        ret = ProcessEventElement(cur_node, 1);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing event element");
    ret = -1;
  }

  return ret;
}

static int ProcessAxisMapElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_AXIS))
      {
        ret = ProcessAxisElement(cur_node);
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }
  return ret;
}

static int ProcessButtonMapElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_BUTTON))
      {
        ret = ProcessButtonElement(cur_node);
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }
  return ret;
}

static int ProcessTriggerElement(xmlNode * a_node)
{
  int ret = 0;
  char* r_switch_back;

  reset_entry();

  ret = GetDeviceTypeProp(a_node);

  if(ret != -1 && entry.device.type != E_DEVICE_TYPE_UNKNOWN)
  {
    ret = GetDeviceName(a_node);

    if(ret != -1)
    {
      ret = GetDeviceId(a_node);

      if(ret != -1)
      {
        ret = GetEventId(a_node, X_ATTR_BUTTON_ID);
      }
    }

    //Optional
    entry.params.trigger.switch_back = 0;
    r_switch_back = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SWITCH_BACK);
    if(r_switch_back)
    {
      if(!strncmp(r_switch_back, X_ATTR_VALUE_YES, sizeof(X_ATTR_VALUE_YES)))
      {
        entry.params.trigger.switch_back = 1;
      }
    }
    xmlFree(r_switch_back);

    //Optional
    entry.params.trigger.delay = 0;
    GetIntProp(a_node, X_ATTR_DELAY, &entry.params.trigger.delay);

    if(ret != -1)
    {
      cfg_set_trigger(&entry);
    }
  }

  return ret;
}

static int ProcessUpDownElement(xmlNode * a_node, int* device_type, int* device_id, int* button)
{
  int ret = 0;

  ret = GetDeviceTypeProp(a_node);

  if(ret != -1 && entry.device.type != E_DEVICE_TYPE_UNKNOWN)
  {
    ret = GetDeviceName(a_node);

    if(ret != -1)
    {
      ret = GetDeviceId(a_node);

      if(ret != -1)
      {
        ret = GetEventId(a_node, X_ATTR_BUTTON_ID);

        *button = entry.event.id;
        *device_id = entry.device.id;
        *device_type = entry.device.type;
      }
    }
  }

  return ret;
}

static int ProcessIntensityElement(xmlNode * a_node, s_intensity* intensity)
{
  xmlNode* cur_node = NULL;
  int ret = 0;
  char* shape;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_UP))
      {
        ret = ProcessUpDownElement(cur_node, &intensity->up.device.type, &intensity->up.device.id, &intensity->up.button);
      }
      else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DOWN))
      {
        ret = ProcessUpDownElement(cur_node, &intensity->down.device.type, &intensity->down.device.id, &intensity->down.button);
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if(ret != -1 && (intensity->down.button != -1 || intensity->up.button != -1))
  {
    ret = GetUnsignedIntProp(a_node, X_ATTR_DEADZONE, &intensity->params.dead_zone);

    if(ret != -1)
    {
      shape = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
      if(shape)
      {
        if (!strncmp(shape, X_ATTR_VALUE_RECTANGLE, strlen(X_ATTR_VALUE_RECTANGLE)))
        {
          intensity->params.shape = E_SHAPE_RECTANGLE;
        }
        else
        {
          intensity->params.shape = E_SHAPE_CIRCLE;
        }
      }
      else
      {
        ret = -1;
      }
      xmlFree(shape);

      if(ret != -1)
      {
        ret = GetUnsignedIntProp(a_node, X_ATTR_STEPS, &intensity->params.steps);
      }
    }
  }

  return ret;
}

static int ProcessIntensityListElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;
  int ret = 0;
  char* control;
  int axis1, axis2;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY))
      {
        reset_entry();
        s_intensity intensity = { .down.button = -1, .up.button = -1 };

        axis1 = axis2 = -1;

        control = (char*) xmlGetProp(cur_node, (xmlChar*) X_ATTR_CONTROL);

        if(!strcmp(control, "left_stick") || !strcmp(control, "lstick"))
        {
          axis1 = rel_axis_lstick_x;
          axis2 = rel_axis_lstick_y;
        }
        else if(!strcmp(control, "right_stick") || !strcmp(control, "rstick"))
        {
          axis1 = rel_axis_rstick_x;
          axis2 = rel_axis_rstick_y;
        }
        else
        {
          axis1 = controller_get_axis_index(control);
        }
        if(axis1 >= 0)
        {
          ret = ProcessIntensityElement(cur_node, &intensity);
          cfg_set_axis_intensity(&entry, axis1, &intensity);
          if(axis2 >= 0)
          {
            cfg_set_axis_intensity(&entry, axis2, &intensity);
          }
        }
        xmlFree(control);
      }
      else
      {
        break;
      }
    }
  }

  return ret;
}

static int ProcessMouseOptionsListElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;
  int ret = 0;
  char* prop;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_MOUSE))
      {
        reset_entry();

        entry.device.type = E_DEVICE_TYPE_MOUSE;

        ret = GetDeviceName(cur_node);

        if(ret != -1)
        {
          ret = GetDeviceId(cur_node);

          prop = (char*) xmlGetProp(cur_node, (xmlChar*) X_ATTR_MODE);
          if(prop)
          {
            if (!strncmp(prop, X_ATTR_VALUE_AIMING, strlen(X_ATTR_VALUE_AIMING)))
            {
              entry.params.mouse_options.mode = E_MOUSE_MODE_AIMING;
            }
            else if (!strncmp(prop, X_ATTR_VALUE_DRIVING, strlen(X_ATTR_VALUE_DRIVING)))
            {
              entry.params.mouse_options.mode = E_MOUSE_MODE_DRIVING;
            }
            else
            {
              //ret = -1;
              //Work-around empty mode bug.
              entry.params.mouse_options.mode = E_MOUSE_MODE_AIMING;
            }

            if(ret != -1)
            {
              ret = GetUnsignedIntProp(cur_node, X_ATTR_BUFFERSIZE, &entry.params.mouse_options.buffer_size);

              if(ret != -1)
              {
                ret = GetDoubleProp(cur_node, X_ATTR_FILTER, &entry.params.mouse_options.filter);

                if(ret != -1)
                {
                  cal_set_mouse(&entry);
                }
              }
            }
          }
          else
          {
            ret = -1;
          }
          xmlFree(prop);
        }
      }
      else
      {
        break;
      }
    }
  }

  return ret;
}

static int ProcessCorrectionElement(xmlNode * a_node)
{
  int ret = 0;

  reset_entry();

  if(GetIntProp(a_node, X_ATTR_LOW_VALUE, &entry.params.joystick_correction.coef[0]) == -1)
  {
    printf("missing %s attribute\n", X_ATTR_LOW_VALUE);
    return -1;
  }

  if(GetIntProp(a_node, X_ATTR_HIGH_VALUE, &entry.params.joystick_correction.coef[1]) == -1)
  {
    printf("missing %s attribute\n", X_ATTR_HIGH_VALUE);
    return -1;
  }

  if(GetIntProp(a_node, X_ATTR_LOW_COEF, &entry.params.joystick_correction.coef[2]) == -1)
  {
    printf("missing %s attribute\n", X_ATTR_LOW_COEF);
    return -1;
  }

  if(GetIntProp(a_node, X_ATTR_HIGH_COEF, &entry.params.joystick_correction.coef[3]) == -1)
  {
    printf("missing %s attribute\n", X_ATTR_HIGH_COEF);
    return -1;
  }

  xmlNode* cur_node = NULL;
  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DEVICE))
      {
        ret = ProcessDeviceElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing device element");
    ret = -1;
  }

  for (cur_node = cur_node->next; cur_node && ret == 0; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_EVENT))
      {
        ret = ProcessEventElement(cur_node, 0);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing event element");
    ret = -1;
  }

  if(ret != -1)
  {
    entry.params.joystick_correction.axis = entry.event.id;
    cfg_add_js_corr(entry.device.id, &entry.params.joystick_correction);
  }

  return ret;
}

static int ProcessJoystickCorrectionsListElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CORRECTION))
      {
        ret = ProcessCorrectionElement(cur_node);
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }
  return ret;
}

static int ProcessConfigurationElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;

  ret = GetUnsignedIntProp(a_node, X_ATTR_ID, &entry.config_id);

  if(ret != -1)
  {
    entry.config_id--;

    if (entry.config_id >= MAX_CONFIGURATIONS)
    {
      printf("bad configuration id: %d\n", entry.config_id);
      ret = -1;
    }
  }

  cur_node = a_node->children;

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_TRIGGER))
      {
        ret = ProcessTriggerElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing trigger element");
    ret = -1;
  }
  
  for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_MOUSE_OPTIONS_LIST))
      {
        ret = ProcessMouseOptionsListElement(cur_node);
        break;
      }
      else
      {
        cur_node = cur_node->prev;
        break;
      }
    }
  }

  for (cur_node = cur_node->next; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY_LIST))
      {
        ret = ProcessIntensityListElement(cur_node);
        break;
      }
      else
      {
        cur_node = cur_node->prev;
        break;
      }
    }
  }

  for (cur_node = cur_node->next; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_BUTTON_MAP))
      {
        ret = ProcessButtonMapElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing button_map element");
    ret = -1;
  }

  for (cur_node = cur_node->next; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_AXIS_MAP))
      {
        ret = ProcessAxisMapElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if (!cur_node)
  {
    printf("missing axis_map element");
    ret = -1;
  }

  for (cur_node = cur_node->next; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_JOYSTICK_CORRECTIONS_LIST))
      {
        ret = ProcessJoystickCorrectionsListElement(cur_node);
        break;
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  return ret;
}

static int ProcessControllerElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;
  int ret = 0;

  ret = GetUnsignedIntProp(a_node, X_ATTR_ID, &entry.controller_id);

  if(ret != -1)
  {
    entry.controller_id--;

    if (entry.controller_id >= MAX_CONTROLLERS)
    {
      printf("bad controller id: %d\n", entry.controller_id);
      ret = -1;
    }
  }

  if(ret != -1)
  {
    unsigned int dpi;
    /* optional */
    if(GetUnsignedIntProp(a_node, X_ATTR_DPI, &dpi) != -1)
    {
      cfg_set_controller_dpi(entry.controller_id, dpi);
    }
  }

  for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CONFIGURATION))
      {
        ret = ProcessConfigurationElement(cur_node);
      }
      else
      {
        ret = -1;
        printf("bad element name: %s\n", cur_node->name);
      }
    }
  }
  return ret;
}

static int ProcessRootElement(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;
  int ret = 0;

  if (a_node->type == XML_ELEMENT_NODE)
  {
    if (xmlStrEqual(a_node->name, (xmlChar*) X_NODE_ROOT))
    {
      for (cur_node = a_node->children; cur_node && ret != -1; cur_node = cur_node->next)
      {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
          if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_CONTROLLER))
          {
            ret = ProcessControllerElement(cur_node);
          }
          else
          {
            ret = -1;
            printf("bad element name: %s\n", cur_node->name);
          }
        }
      }
    }
    else
    {
      ret = -1;
      printf("bad element name: %s\n", a_node->name);
    }
  }
  return ret;
}

/*
 * This function loads a config file.
 */
static int read_file(char* file_path)
{
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  int ret = 0;

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

  /*parse the file and get the DOM */
  doc = xmlReadFile(file_path, NULL, 0);

#ifdef WIN32
  if(!xmlFree) xmlMemGet(&xmlFree,&xmlMalloc,&xmlRealloc,NULL);
#endif

  if (doc != NULL)
  {
    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    if(root_element != NULL)
    {
      ret = ProcessRootElement(root_element);
    }
    else
    {
      ret = -1;
      fprintf(stderr, "xml error: no root element\n");
    }
  }
  else
  {
    ret = -1;
    fprintf(stderr, "could not parse file %s: %s\n", file_path, strerror(errno));
  }

  /*free the document */
  xmlFreeDoc(doc);

  return ret;
}

/*
 * This function loads a config file.
 */
int read_config_file(const char* file)
{
  char file_path[PATH_MAX];

  snprintf(file_path, sizeof(file_path), "%s%s%s%s", gimx_params.homedir, GIMX_DIR, CONFIG_DIR, file);

  if(read_file(file_path) == -1)
  {
    fprintf(stderr, "read_file failed\n");
    return -1;
  }

  return 0;
}
