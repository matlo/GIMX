/*
 * config_reader.c
 *
 *  Created on: 6 mai 2011
 *      Author: matlo
 */

#include "config_reader.h"
#include "config.h"
#include "conversion.h"
#include "sdl_tools.h"
#include "calibration.h"
#include <limits.h>
#include <dirent.h>
#include <libxml/xmlreader.h>
#include <iconv.h>
#include "emuclient.h"
#include "../directories.h"
#include "macros.h"

#ifdef WIN32
#include <sys/stat.h>
#define LINE_MAX 1024
#endif

/*
 * These variables are used to read the configuration.
 */
static unsigned int r_controller_id;
static unsigned int r_config_id;
static e_event_type r_event_type;
static unsigned int r_event_id;
static int r_threshold;
static unsigned int r_deadZone;
static double r_multiplier;
static double r_exponent;
static e_shape r_shape;
static e_device_type r_device_type;
static int r_device_id;
static char r_device_name[128];
static unsigned int r_config_dpi[MAX_CONTROLLERS];
static unsigned int r_buffer_size;
static double r_filter;

/*
 * Get the device name and store it into r_device_name.
 * OK, return 0
 * error, return -1
 */
int GetDeviceName(xmlNode* a_node)
{
  int ret = 0;
  char* prop;
  iconv_t cd;
  char* input;
  size_t in;
  char* output = r_device_name;
  size_t out = sizeof(r_device_name);

  prop = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_NAME);
  if(prop)
  {
    cd = iconv_open("ISO-8859-1//TRANSLIT", "UTF-8");
    input = prop;
    in = strlen(prop) + 1;
#ifndef WIN32
    iconv(cd, (char**)&input, &in, (char**)&output, &out);
#else
    iconv(cd, (const char**)&input, &in, (char**)&output, &out);
#endif
    iconv_close(cd);
  }
  else
  {
    ret = -1;
  }
  xmlFree(prop);

  return ret;
}

/*
 * Get the device id and store it into r_device_id.
 * OK, return 0
 * error, return -1
 * device not found, return 1
 */
static int GetDeviceId(xmlNode* a_node)
{
  int i;
  int ret;

  ret = GetIntProp(a_node, X_ATTR_ID, &r_device_id);

  if(ret != -1)
  {
    if(r_device_type == E_DEVICE_TYPE_JOYSTICK)
    {
      for (i = 0; i < MAX_DEVICES && sdl_get_joystick_name(i); ++i)
      {
        if (!strcmp(r_device_name, sdl_get_joystick_name(i)))
        {
          if (r_device_id == sdl_get_joystick_virtual_id(i))
          {
            r_device_id = i;
            break;
          }
        }
      }
      if(i == MAX_DEVICES || !sdl_get_joystick_name(i))
      {
        gprintf("joystick not found: %s %d\n", r_device_name, r_device_id);
        ret = 1;
      }
    }
    else if(merge_all_devices && !check_config)
    {
      r_device_id = 0;
    }
    else if(!strlen(r_device_name))
    {
      if(!merge_all_devices)
      {
        gprintf("A device name is empty. Multiple mice and keyboards are not managed.\n");
      }
      merge_all_devices = 1;
    }
    else
    {
      if(r_device_type == E_DEVICE_TYPE_MOUSE)
      {
        for (i = 0; i < MAX_DEVICES && sdl_get_mouse_name(i); ++i)
        {
          if (!strcmp(r_device_name, sdl_get_mouse_name(i)))
          {
            if (r_device_id == sdl_get_mouse_virtual_id(i))
            {
              r_device_id = i;
              break;
            }
          }
        }
        if(i == MAX_DEVICES || !sdl_get_mouse_name(i))
        {
          gprintf("mouse not found: %s %d\n", r_device_name, r_device_id);
          ret = 1;
        }
      }
      else if(r_device_type == E_DEVICE_TYPE_KEYBOARD)
      {
        for (i = 0; i < MAX_DEVICES && sdl_get_keyboard_name(i); ++i)
        {
          if (!strcmp(r_device_name, sdl_get_keyboard_name(i)))
          {
            if (r_device_id == sdl_get_keyboard_virtual_id(i))
            {
              r_device_id = i;
              break;
            }
          }
        }
        if(i == MAX_DEVICES || !sdl_get_keyboard_name(i))
        {
          gprintf("keyboard not found: %s %d\n", r_device_name, r_device_id);
          ret = 1;
        }
      }
    }
  }

  return ret;
}

/*
 * Get the device type and store it into r_device_type.
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
      r_device_type = E_DEVICE_TYPE_KEYBOARD;
    }
    else if (!strncmp(type, X_ATTR_VALUE_MOUSE, strlen(X_ATTR_VALUE_MOUSE)))
    {
      r_device_type = E_DEVICE_TYPE_MOUSE;
    }
    else if (!strncmp(type, X_ATTR_VALUE_JOYSTICK, strlen(X_ATTR_VALUE_JOYSTICK)))
    {
      r_device_type = E_DEVICE_TYPE_JOYSTICK;
    }
    else
    {
      r_device_type = E_DEVICE_TYPE_UNKNOWN;
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
 * Get the event id and store it into r_event_id.
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
    switch(r_device_type)
    {
      case E_DEVICE_TYPE_KEYBOARD:
        r_event_id = get_key_from_buffer(event_id);
        break;
      case E_DEVICE_TYPE_JOYSTICK:
        r_event_id = atoi(event_id);
        break;
      case E_DEVICE_TYPE_MOUSE:
        r_event_id = get_mouse_event_id_from_buffer(event_id);
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

static int ProcessEventElement(xmlNode * a_node)
{
  int ret = 0;
  char* type;
  char* shape;

  type = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_TYPE);
  if (!strncmp(type, X_ATTR_VALUE_BUTTON, strlen(X_ATTR_VALUE_BUTTON)))
  {
    r_event_type = E_EVENT_TYPE_BUTTON;
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS_DOWN, strlen(X_ATTR_VALUE_AXIS_DOWN)))
  {
    r_event_type = E_EVENT_TYPE_AXIS_DOWN;
    ret = GetIntProp(a_node, X_ATTR_THRESHOLD, &r_threshold);
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS_UP, strlen(X_ATTR_VALUE_AXIS_UP)))
  {
    r_event_type = E_EVENT_TYPE_AXIS_UP;
    ret = GetIntProp(a_node, X_ATTR_THRESHOLD, &r_threshold);
  }
  else if (!strncmp(type, X_ATTR_VALUE_AXIS, strlen(X_ATTR_VALUE_AXIS)))
  {
    r_event_type = E_EVENT_TYPE_AXIS;

    ret = GetUnsignedIntProp(a_node, X_ATTR_DEADZONE, &r_deadZone);
    if(ret == -1)
    {
      r_deadZone = 0;
    }
    ret = GetDoubleProp(a_node, X_ATTR_MULTIPLIER, &r_multiplier);
    if(ret == -1)
    {
      r_multiplier = 1;
    }
    ret = GetDoubleProp(a_node, X_ATTR_EXPONENT, &r_exponent);
    if(ret == -1)
    {
      r_exponent = 1;
      ret = 0;
    }
    shape = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
    r_shape = E_SHAPE_CIRCLE;//default value
    if(shape)
    {
      if (!strncmp(shape, X_ATTR_VALUE_RECTANGLE, strlen(X_ATTR_VALUE_RECTANGLE)))
      {
        r_shape = E_SHAPE_RECTANGLE;
      }
    }
    xmlFree(shape);
    r_buffer_size = 1;//default value
    GetUnsignedIntProp(a_node, X_ATTR_BUFFERSIZE, &r_buffer_size);
    r_filter = 0;//default value
    GetDoubleProp(a_node, X_ATTR_FILTER, &r_filter);
  }

  ret = GetEventId(a_node, X_ATTR_ID);

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

static s_mapper** get_mapper_table()
{
  s_mapper** pp_mapper = NULL;

  if(r_device_id < 0) return NULL;

  switch(r_device_type)
  {
    case E_DEVICE_TYPE_KEYBOARD:
      pp_mapper = cfg_get_keyboard_buttons(r_device_id, r_controller_id, r_config_id);
      break;
    case E_DEVICE_TYPE_MOUSE:
      switch(r_event_type)
      {
        case E_EVENT_TYPE_BUTTON:
          pp_mapper = cfg_get_mouse_buttons(r_device_id, r_controller_id, r_config_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          pp_mapper = cfg_get_mouse_axes(r_device_id, r_controller_id, r_config_id);
          break;
        default:
          break;
      }
      break;
    case E_DEVICE_TYPE_JOYSTICK:
      switch(r_event_type)
      {
        case E_EVENT_TYPE_BUTTON:
          pp_mapper = cfg_get_joystick_buttons(r_device_id, r_controller_id, r_config_id);
          break;
        case E_EVENT_TYPE_AXIS:
        case E_EVENT_TYPE_AXIS_UP:
        case E_EVENT_TYPE_AXIS_DOWN:
          pp_mapper = cfg_get_joystick_axes(r_device_id, r_controller_id, r_config_id);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return pp_mapper;
}

static void allocate_mapper(s_mapper** pp_mapper)
{
  if(*pp_mapper)
  {
    *pp_mapper = realloc(*pp_mapper, ((*pp_mapper)->nb_mappers+1)*sizeof(s_mapper));
    memset(*pp_mapper+(*pp_mapper)->nb_mappers, 0x00, sizeof(s_mapper));
  }
  else
  {
    *pp_mapper = calloc(1, sizeof(s_mapper));
  }
  (*pp_mapper)->nb_mappers++;
}

static int ProcessAxisElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;
  s_mapper** pp_mapper = NULL;
  s_mapper* p_mapper = NULL;
  s_axis_index aindex;
  char* aid;

  aid = (char*)xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);

  aindex = get_axis_index_from_name(aid);

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
        ret = ProcessEventElement(cur_node);
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

  if(ret == 0)
  {
    pp_mapper = get_mapper_table();

    if(pp_mapper)
    {
      allocate_mapper(pp_mapper);

      p_mapper = *pp_mapper;

      p_mapper = p_mapper+p_mapper->nb_mappers-1;

      p_mapper->controller_axis_value = aindex.value;
      p_mapper->controller_axis = aindex.index;

      switch(r_event_type)
      {
        case E_EVENT_TYPE_BUTTON:
          p_mapper->button = r_event_id;
          macro_set_controller_device(r_controller_id, r_device_type-1, r_device_id);
          break;
        case E_EVENT_TYPE_AXIS:
          p_mapper->axis = r_event_id;
          p_mapper->dead_zone = r_deadZone;
          p_mapper->multiplier = r_multiplier;
          p_mapper->exponent = r_exponent;
          p_mapper->shape = r_shape;
          p_mapper->buffer_size = r_buffer_size;
          p_mapper->filter = r_filter;
          break;
        default:
          break;
      }
    }
  }

  return ret;
}

static int ProcessButtonElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;
  char* bid;
  e_sixaxis_axis_index bindex = 0;
  s_mapper* p_mapper = NULL;
  s_mapper** pp_mapper = NULL;

  bid = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_ID);

  ret = get_button_index_from_name(bid);

  xmlFree(bid);

  if(ret != -1)
  {
    bindex = ret;
  }

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
        ret = ProcessEventElement(cur_node);
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

  if(ret == 0)
  {
    pp_mapper = get_mapper_table();

    if(pp_mapper)
    {
      allocate_mapper(pp_mapper);

      p_mapper = *pp_mapper;

      p_mapper = p_mapper + p_mapper->nb_mappers - 1;

      p_mapper->controller_axis_value = 0;
      p_mapper->controller_axis = bindex;

      switch (r_event_type)
      {
        case E_EVENT_TYPE_BUTTON:
          p_mapper->button = r_event_id;
          macro_set_controller_device(r_controller_id, r_device_type-1, r_device_id);
          break;
        case E_EVENT_TYPE_AXIS_DOWN:
        case E_EVENT_TYPE_AXIS_UP:
          p_mapper->axis = r_event_id;
          p_mapper->threshold = r_threshold;
          break;
        default:
          break;
      }
    }
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
  int switch_back = 0;
  int delay = 0;
  s_trigger* trigger;

  ret = GetDeviceTypeProp(a_node);

  if(ret != -1 && r_device_type != E_DEVICE_TYPE_UNKNOWN)
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
    r_switch_back = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SWITCH_BACK);
    if(r_switch_back)
    {
      if(!strncmp(r_switch_back, X_ATTR_VALUE_YES, sizeof(X_ATTR_VALUE_YES)))
      {
        switch_back = 1;
      }
    }
    xmlFree(r_switch_back);

    //Optional
    GetIntProp(a_node, X_ATTR_DELAY, &delay);

    if(ret != -1)
    {
      trigger = cfg_get_trigger(r_controller_id, r_config_id);
      trigger->button = r_event_id;
      trigger->device_id = r_device_id;
      trigger->device_type = r_device_type;
      trigger->switch_back = switch_back;
      trigger->delay = delay;

    }
  }

  return ret;
}

static int ProcessUpDownElement(xmlNode * a_node, int* device_type, int* device_id, int* button)
{
  int ret = 0;

  ret = GetDeviceTypeProp(a_node);

  if(ret != -1 && r_device_type != E_DEVICE_TYPE_UNKNOWN)
  {
    ret = GetDeviceName(a_node);

    if(ret != -1)
    {
      ret = GetDeviceId(a_node);

      if(ret != -1)
      {
        ret = GetEventId(a_node, X_ATTR_BUTTON_ID);

        *button = r_event_id;
        *device_id = r_device_id;
        *device_type = r_device_type;
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
  unsigned int steps;
  unsigned int dz;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_UP))
      {
        ret = ProcessUpDownElement(cur_node, &intensity->device_up_type, &intensity->device_up_id, &intensity->up_button);
      }
      else if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_DOWN))
      {
        ret = ProcessUpDownElement(cur_node, &intensity->device_down_type, &intensity->device_down_id, &intensity->down_button);
      }
      else
      {
        printf("bad element name: %s", cur_node->name);
        ret = -1;
      }
    }
  }

  if(ret != -1 && (intensity->device_down_id != -1 || intensity->device_up_id != -1))
  {
    ret = GetUnsignedIntProp(a_node, X_ATTR_DEADZONE, &dz);

    if(ret != -1)
    {
      intensity->dead_zone = dz * axis_scale;

      shape = (char*) xmlGetProp(a_node, (xmlChar*) X_ATTR_SHAPE);
      if(shape)
      {
        if (!strncmp(shape, X_ATTR_VALUE_RECTANGLE, strlen(X_ATTR_VALUE_RECTANGLE)))
        {
          r_shape = E_SHAPE_RECTANGLE;
        }
        else
        {
          intensity->shape = E_SHAPE_CIRCLE;
        }
      }
      else
      {
        ret = -1;
      }
      xmlFree(shape);

      if(ret != -1)
      {
        ret = GetUnsignedIntProp(a_node, X_ATTR_STEPS, &steps);

        if(ret != -1)
        {
          intensity->step = (double)(mean_axis_value - intensity->dead_zone) / steps;
        }
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
  s_intensity* intensity;

  for (cur_node = a_node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrEqual(cur_node->name, (xmlChar*) X_NODE_INTENSITY))
      {
        control = (char*) xmlGetProp(cur_node, (xmlChar*) X_ATTR_CONTROL);
        if(!strcmp(control, "left_stick"))
        {
          intensity = cfg_get_axis_intensity(r_controller_id, r_config_id, sa_lstick_x);
          ret = ProcessIntensityElement(cur_node, intensity);
          memcpy(cfg_get_axis_intensity(r_controller_id, r_config_id, sa_lstick_y), intensity, sizeof(s_intensity));
        }
        else if(!strcmp(control, "right_stick"))
        {
          intensity = cfg_get_axis_intensity(r_controller_id, r_config_id, sa_rstick_x);
          ret = ProcessIntensityElement(cur_node, intensity);
          memcpy(cfg_get_axis_intensity(r_controller_id, r_config_id, sa_rstick_y), intensity, sizeof(s_intensity));          
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

static int ProcessConfigurationElement(xmlNode * a_node)
{
  int ret = 0;
  xmlNode* cur_node = NULL;
  s_intensity* intensity;
  int i;

  ret = GetUnsignedIntProp(a_node, X_ATTR_ID, &r_config_id);

  if(ret != -1)
  {
    r_config_id--;

    if (r_config_id >= MAX_CONFIGURATIONS)
    {
      printf("bad configuration id: %d\n", r_config_id);
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
  
  for(i=sa_lstick_x; i<=sa_rstick_y; ++i)
  {
    intensity = cfg_get_axis_intensity(r_controller_id, r_config_id, i);
    intensity->device_up_id = -1;
    intensity->device_down_id = -1;
    intensity->up_button = -1;
    intensity->down_button = -1;
    intensity->value = mean_axis_value;
    intensity->shape = E_SHAPE_RECTANGLE;
  }
  for(i=sa_acc_x; i<=sa_gyro; ++i)
  {
    intensity = cfg_get_axis_intensity(r_controller_id, r_config_id, i);
    intensity->device_up_id = -1;
    intensity->device_down_id = -1;
    intensity->up_button = -1;
    intensity->down_button = -1;
    intensity->value = 512;
    intensity->shape = E_SHAPE_RECTANGLE;
  }  
  for(i=sa_select; i<SA_MAX; ++i)
  {
    intensity = cfg_get_axis_intensity(r_controller_id, r_config_id, i);
    intensity->device_up_id = -1;
    intensity->device_down_id = -1;
    intensity->up_button = -1;
    intensity->down_button = -1;
    intensity->value = 255;
    intensity->shape = E_SHAPE_RECTANGLE;
  }
  
  for (cur_node = cur_node->next; cur_node; cur_node = cur_node->next)
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

  return ret;
}

static int ProcessControllerElement(xmlNode * a_node)
{
  xmlNode* cur_node = NULL;
  int ret = 0;

  ret = GetUnsignedIntProp(a_node, X_ATTR_ID, &r_controller_id);

  if(ret != -1)
  {
    r_controller_id--;

    if (r_controller_id >= MAX_CONTROLLERS)
    {
      printf("bad controller id: %d\n", r_controller_id);
      ret = -1;
    }
  }

  if(ret != -1)
  {
    /* optional */
    GetUnsignedIntProp(a_node, X_ATTR_DPI, r_config_dpi+r_controller_id);
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
      printf("error: no root element\n");
    }
  }
  else
  {
    ret = -1;
    printf("error: could not parse file %s\n", file_path);
  }

  /*free the document */
  xmlFreeDoc(doc);

  return ret;
}

static void read_calibration()
{
  int i, j, k;
  s_mapper** pp_mapper;
  s_mapper* p_mapper;
  s_mouse_cal* mcal;
  int found;

  current_mouse = -1;

  for(i=0; i<MAX_DEVICES; ++i)
  {
    found = 0;
    for(j=0; j<MAX_CONTROLLERS && !found; ++j)
    {
      for(k=0; k<MAX_CONFIGURATIONS; ++k)
      {
        pp_mapper = cfg_get_mouse_axes(i, j, k);
        mcal = cal_get_mouse(i, k);
        for(p_mapper = *pp_mapper; p_mapper && p_mapper<*pp_mapper+(*pp_mapper)->nb_mappers; p_mapper++)
        {
          if(current_mouse < 0)
          {
            current_mouse = i;
          }
          if(p_mapper->axis == 0)
          {
            found = 1;
            cal_set_controller(i, j);
            mcal->mx = &p_mapper->multiplier;
            mcal->ex = &p_mapper->exponent;
            mcal->dzx = &p_mapper->dead_zone;
            mcal->dzs = &p_mapper->shape;
            mcal->rd = DEFAULT_RADIUS;
            mcal->vel = DEFAULT_VELOCITY;
            mcal->dpi = r_config_dpi[j];
            mcal->bsx = &p_mapper->buffer_size;
            mcal->fix = &p_mapper->filter;
          }
          else
          {
            found = 1;
            cal_set_controller(i, j);
            mcal->my = &p_mapper->multiplier;
            mcal->ey = &p_mapper->exponent;
            mcal->dzy = &p_mapper->dead_zone;
            mcal->bsy = &p_mapper->buffer_size;
            mcal->fiy = &p_mapper->filter;
          }
        }
      }
    }
  }

  if(current_mouse < 0)
  {
    current_mouse = 0;
  }
}

void free_config()
{
  s_mapper** mapper;
  int i, j, k;
  for(i=0; i<MAX_DEVICES; ++i)
  {
    for(j=0; j<MAX_CONTROLLERS; ++j)
    {
      for(k=0; k<MAX_CONFIGURATIONS; ++k)
      {
        mapper = cfg_get_keyboard_buttons(i, j, k);
        free(*mapper);
        *mapper = NULL;
        mapper = cfg_get_mouse_buttons(i, j, k);
        free(*mapper);
        *mapper = NULL;
        mapper = cfg_get_mouse_axes(i, j, k);
        free(*mapper);
        *mapper = NULL;
        mapper = cfg_get_joystick_buttons(i, j, k);
        free(*mapper);
        *mapper = NULL;
        mapper = cfg_get_joystick_axes(i, j, k);
        free(*mapper);
        *mapper = NULL;
      }
    }
  }
  cal_init();
}

/*
 * This function loads a config file.
 */
void read_config_file(const char* file)
{
  char file_path[PATH_MAX];

#ifndef WIN32
  snprintf(file_path, sizeof(file_path), "%s%s%s%s", homedir, APP_DIR, CONFIG_DIR, file);
#else
  snprintf(file_path, sizeof(file_path), "%s%s", CONFIG_DIR, file);
#endif

  if(read_file(file_path) == -1)
  {
    printf("Bad config file: %s\n", file_path);
    exit(-1);
  }

  read_calibration();
}
