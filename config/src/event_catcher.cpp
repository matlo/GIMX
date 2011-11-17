#include "event_catcher.h"
#include <unistd.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define EVENT_BUFFER_SIZE 32

#define MAX_ARG_LENGTH 32

typedef struct
{
    char arg[MAX_ARG_LENGTH];
    SDLKey key;
} s_arg_key;

s_arg_key arg_key_table[] =
{
    {"UNKNOWN",         SDLK_UNKNOWN         },
    {"FIRST",             SDLK_FIRST             },
    {"BACKSPACE",         SDLK_BACKSPACE      },
    {"TAB",             SDLK_TAB             },
    {"CLEAR",             SDLK_CLEAR             },
    {"RETURN",             SDLK_RETURN         },
    {"PAUSE",             SDLK_PAUSE          },
    {"ESCAPE",             SDLK_ESCAPE         },
    {"SPACE",                SDLK_SPACE          },
    {"EXCLAIM",         SDLK_EXCLAIM         },
    {"QUOTEDBL",         SDLK_QUOTEDBL       },
    {"HASH",             SDLK_HASH           },
    {"DOLLAR",             SDLK_DOLLAR         },
    {"AMPERSAND",         SDLK_AMPERSAND      },
    {"QUOTE",             SDLK_QUOTE          },
    {"LEFTPAREN",         SDLK_LEFTPAREN      },
    {"RIGHTPAREN",         SDLK_RIGHTPAREN     },
    {"ASTERISK",         SDLK_ASTERISK       },
    {"PLUS",             SDLK_PLUS           },
    {"COMMA",             SDLK_COMMA          },
    {"MINUS",             SDLK_MINUS          },
    {"PERIOD",             SDLK_PERIOD         },
    {"SLASH",             SDLK_SLASH          },
    {"0",                 SDLK_0                 },
    {"1",                 SDLK_1                 },
    {"2",                 SDLK_2                 },
    {"3",                 SDLK_3                 },
    {"4",                 SDLK_4                 },
    {"5",                 SDLK_5                 },
    {"6",                 SDLK_6                 },
    {"7",                 SDLK_7                 },
    {"8",                 SDLK_8                 },
    {"9",                 SDLK_9                 },
    {"COLON",             SDLK_COLON             },
    {"SEMICOLON",         SDLK_SEMICOLON      },
    {"LESS",             SDLK_LESS           },
    {"EQUALS",             SDLK_EQUALS         },
    {"GREATER",         SDLK_GREATER        },
    {"QUESTION",         SDLK_QUESTION       },
    {"AT",                 SDLK_AT             },
    /*
       Skip uppercase letters
     */
    {"LEFTBRACKET",        SDLK_LEFTBRACKET     },
    {"BACKSLASH",         SDLK_BACKSLASH         },
    {"RIGHTBRACKET",     SDLK_RIGHTBRACKET     },
    {"CARET",              SDLK_CARET             },
    {"UNDERSCORE",         SDLK_UNDERSCORE     },
    {"BACKQUOTE",         SDLK_BACKQUOTE         },
    {"a",                 SDLK_a                 },
    {"b",                 SDLK_b                 },
    {"c",                 SDLK_c                 },
    {"d",                 SDLK_d                 },
    {"e",                 SDLK_e                 },
    {"f",                 SDLK_f                 },
    {"g",                 SDLK_g                 },
    {"h",                 SDLK_h                 },
    {"i",                 SDLK_i                 },
    {"j",                 SDLK_j                 },
    {"k",                 SDLK_k                 },
    {"l",                 SDLK_l                 },
    {"m",                 SDLK_m                 },
    {"n",                 SDLK_n                 },
    {"o",                 SDLK_o                 },
    {"p",                 SDLK_p                 },
    {"q",                 SDLK_q                 },
    {"r",                 SDLK_r                 },
    {"s",                 SDLK_s                 },
    {"t",                 SDLK_t                 },
    {"u",                 SDLK_u                 },
    {"v",                 SDLK_v                 },
    {"w",                 SDLK_w                 },
    {"x",                 SDLK_x                 },
    {"y",                 SDLK_y                 },
    {"z",                 SDLK_z                 },
    {"DELETE",             SDLK_DELETE         },
    /* End of ASCII mapped keysyms */

    /* International keyboard syms */
    {"WORLD_0",         SDLK_WORLD_0         },
    {"WORLD_1",         SDLK_WORLD_1         },
    {"WORLD_2",         SDLK_WORLD_2         },
    {"WORLD_3",         SDLK_WORLD_3         },
    {"WORLD_4",         SDLK_WORLD_4         },
    {"WORLD_5",         SDLK_WORLD_5         },
    {"WORLD_6",         SDLK_WORLD_6         },
    {"WORLD_7",         SDLK_WORLD_7         },
    {"WORLD_8",         SDLK_WORLD_8         },
    {"WORLD_9",         SDLK_WORLD_9         },
    {"WORLD_10",         SDLK_WORLD_10         },
    {"WORLD_11",         SDLK_WORLD_11         },
    {"WORLD_12",         SDLK_WORLD_12         },
    {"WORLD_13",         SDLK_WORLD_13         },
    {"WORLD_14",         SDLK_WORLD_14         },
    {"WORLD_15",         SDLK_WORLD_15         },
    {"WORLD_16",         SDLK_WORLD_16         },
    {"WORLD_17",         SDLK_WORLD_17         },
    {"WORLD_18",         SDLK_WORLD_18         },
    {"WORLD_19",         SDLK_WORLD_19         },
    {"WORLD_20",         SDLK_WORLD_20         },
    {"WORLD_21",         SDLK_WORLD_21         },
    {"WORLD_22",         SDLK_WORLD_22         },
    {"WORLD_23",         SDLK_WORLD_23         },
    {"WORLD_24",         SDLK_WORLD_24         },
    {"WORLD_25",         SDLK_WORLD_25         },
    {"WORLD_26",         SDLK_WORLD_26         },
    {"WORLD_27",         SDLK_WORLD_27         },
    {"WORLD_28",         SDLK_WORLD_28         },
    {"WORLD_29",         SDLK_WORLD_29         },
    {"WORLD_30",         SDLK_WORLD_30         },
    {"WORLD_31",         SDLK_WORLD_31         },
    {"WORLD_32",         SDLK_WORLD_32         },
    {"WORLD_33",         SDLK_WORLD_33         },
    {"WORLD_34",         SDLK_WORLD_34         },
    {"WORLD_35",         SDLK_WORLD_35         },
    {"WORLD_36",         SDLK_WORLD_36         },
    {"WORLD_37",         SDLK_WORLD_37         },
    {"WORLD_38",         SDLK_WORLD_38         },
    {"WORLD_39",         SDLK_WORLD_39         },
    {"WORLD_40",         SDLK_WORLD_40         },
    {"WORLD_41",         SDLK_WORLD_41         },
    {"WORLD_42",         SDLK_WORLD_42         },
    {"WORLD_43",         SDLK_WORLD_43         },
    {"WORLD_44",         SDLK_WORLD_44         },
    {"WORLD_45",         SDLK_WORLD_45         },
    {"WORLD_46",         SDLK_WORLD_46         },
    {"WORLD_47",         SDLK_WORLD_47         },
    {"WORLD_48",         SDLK_WORLD_48         },
    {"WORLD_49",         SDLK_WORLD_49         },
    {"WORLD_50",         SDLK_WORLD_50         },
    {"WORLD_51",         SDLK_WORLD_51         },
    {"WORLD_52",         SDLK_WORLD_52         },
    {"WORLD_53",         SDLK_WORLD_53         },
    {"WORLD_54",         SDLK_WORLD_54         },
    {"WORLD_55",         SDLK_WORLD_55         },
    {"WORLD_56",         SDLK_WORLD_56         },
    {"WORLD_57",         SDLK_WORLD_57         },
    {"WORLD_58",         SDLK_WORLD_58         },
    {"WORLD_59",         SDLK_WORLD_59         },
    {"WORLD_60",         SDLK_WORLD_60         },
    {"WORLD_61",         SDLK_WORLD_61         },
    {"WORLD_62",         SDLK_WORLD_62         },
    {"WORLD_63",         SDLK_WORLD_63         },
    {"WORLD_64",         SDLK_WORLD_64         },
    {"WORLD_65",         SDLK_WORLD_65         },
    {"WORLD_66",         SDLK_WORLD_66         },
    {"WORLD_67",         SDLK_WORLD_67         },
    {"WORLD_68",         SDLK_WORLD_68         },
    {"WORLD_69",         SDLK_WORLD_69         },
    {"WORLD_70",         SDLK_WORLD_70         },
    {"WORLD_71",         SDLK_WORLD_71         },
    {"WORLD_72",         SDLK_WORLD_72         },
    {"WORLD_73",         SDLK_WORLD_73         },
    {"WORLD_74",         SDLK_WORLD_74         },
    {"WORLD_75",         SDLK_WORLD_75         },
    {"WORLD_76",         SDLK_WORLD_76         },
    {"WORLD_77",         SDLK_WORLD_77         },
    {"WORLD_78",         SDLK_WORLD_78         },
    {"WORLD_79",         SDLK_WORLD_79         },
    {"WORLD_80",         SDLK_WORLD_80         },
    {"WORLD_81",         SDLK_WORLD_81         },
    {"WORLD_82",         SDLK_WORLD_82         },
    {"WORLD_83",         SDLK_WORLD_83         },
    {"WORLD_84",         SDLK_WORLD_84         },
    {"WORLD_85",         SDLK_WORLD_85         },
    {"WORLD_86",         SDLK_WORLD_86         },
    {"WORLD_87",         SDLK_WORLD_87         },
    {"WORLD_88",         SDLK_WORLD_88         },
    {"WORLD_89",         SDLK_WORLD_89         },
    {"WORLD_90",         SDLK_WORLD_90         },
    {"WORLD_91",         SDLK_WORLD_91         },
    {"WORLD_92",         SDLK_WORLD_92         },
    {"WORLD_93",         SDLK_WORLD_93         },
    {"WORLD_94",         SDLK_WORLD_94         },
    {"WORLD_95",         SDLK_WORLD_95         },

    /* Numeric keypad */
    {"KP0",             SDLK_KP0             },
    {"KP1",             SDLK_KP1             },
    {"KP2",             SDLK_KP2             },
    {"KP3",             SDLK_KP3             },
    {"KP4",             SDLK_KP4             },
    {"KP5",             SDLK_KP5             },
    {"KP6",             SDLK_KP6             },
    {"KP7",             SDLK_KP7             },
    {"KP8",             SDLK_KP8             },
    {"KP9",             SDLK_KP9             },
    {"KP_PERIOD",         SDLK_KP_PERIOD        },
    {"KP_DIVIDE",         SDLK_KP_DIVIDE        },
    {"KP_MULTIPLY",        SDLK_KP_MULTIPLY    },
    {"KP_MINUS",         SDLK_KP_MINUS        },
    {"KP_PLUS",         SDLK_KP_PLUS        },
    {"KP_ENTER",         SDLK_KP_ENTER        },
    {"KP_EQUALS",         SDLK_KP_EQUALS        },

    /* Arrows + Home/End pad */
    {"UP",                 SDLK_UP             },
    {"DOWN",             SDLK_DOWN            },
    {"RIGHT",             SDLK_RIGHT             },
    {"LEFT",             SDLK_LEFT             },
    {"INSERT",             SDLK_INSERT            },
    {"HOME",             SDLK_HOME             },
    {"END",             SDLK_END             },
    {"PAGEUP",             SDLK_PAGEUP            },
    {"PAGEDOWN",         SDLK_PAGEDOWN        },

    /* Function keys */
    {"F1",                 SDLK_F1             },
    {"F2",                 SDLK_F2             },
    {"F3",                 SDLK_F3             },
    {"F4",                 SDLK_F4             },
    {"F5",                 SDLK_F5             },
    {"F6",                 SDLK_F6             },
    {"F7",                 SDLK_F7             },
    {"F8",                 SDLK_F8             },
    {"F9",                 SDLK_F9             },
    {"F10",             SDLK_F10             },
    {"F11",             SDLK_F11             },
    {"F12",             SDLK_F12             },
    {"F13",             SDLK_F13             },
    {"F14",             SDLK_F14             },
    {"F15",             SDLK_F15             },

    /* Key state modifier keys */
    {"NUMLOCK",         SDLK_NUMLOCK        },
    {"CAPSLOCK",         SDLK_CAPSLOCK        },
    {"SCROLLOCK",         SDLK_SCROLLOCK        },
    {"RSHIFT",             SDLK_RSHIFT            },
    {"LSHIFT",             SDLK_LSHIFT            },
    {"RCTRL",             SDLK_RCTRL             },
    {"LCTRL",             SDLK_LCTRL             },
    {"RALT",             SDLK_RALT             },
    {"LALT",             SDLK_LALT             },
    {"RMETA",             SDLK_RMETA             },
    {"LMETA",             SDLK_LMETA             },
    {"LSUPER",             SDLK_LSUPER         },        /* Left "Windows" key */
    {"RSUPER",             SDLK_RSUPER            },        /* Right "Windows" key */
    {"MODE",             SDLK_MODE           },        /* "Alt Gr" key */
    {"COMPOSE",         SDLK_COMPOSE        },        /* Multi-key compose key */

    /* Miscellaneous function keys */
    {"HELP",             SDLK_HELP             },
    {"PRINT",             SDLK_PRINT             },
    {"SYSREQ",             SDLK_SYSREQ            },
    {"BREAK",             SDLK_BREAK             },
    {"MENU",             SDLK_MENU             },
    {"POWER",             SDLK_POWER          },        /* Power Macintosh power key */
    {"EURO",             SDLK_EURO             },        /* Some european keyboards */
    {"UNDO",             SDLK_UNDO             },        /* Atari keyboard has Undo */

    /* Add any other keys here */

    {"LAST",            SDLK_LAST             },
};

/*
 * This function gives a char string from a key code.
 */
static const char* get_chars_from_key(SDLKey key)
{
    int i;

    for (i = 0; i < SDLK_LAST; i++)
    {
        if (arg_key_table[i].key == key)
        {
            return arg_key_table[i].arg;
        }
    }

    return arg_key_table[0].arg;
}

const char* buttons[] =
{
    "UNDEFINED",
    "BUTTON_LEFT",
    "BUTTON_MIDDLE",
    "BUTTON_RIGHT",
    "BUTTON_WHEELUP",
    "BUTTON_WHEELDOWN",
    "BUTTON_X1",
    "BUTTON_X2",
    "BUTTON_X3",
    "BUTTON_X4",
    "BUTTON_X5",
    "BUTTON_X6",
    "BUTTON_X7",
    "BUTTON_X8",
    "BUTTON_X9"
};

static const char* get_chars_from_mouse_button(Uint8 button)
{
    if(button > 0 && button < sizeof(buttons)/sizeof(buttons[0]))
    {
        return buttons[button];
    }

    return buttons[0];
}

event_catcher::event_catcher()
{
    //ctor
}

event_catcher::~event_catcher()
{
    //dtor
}

#define BT_SIXAXIS_NAME "PLAYSTATION(R)3 Controller"

void event_catcher::init()
{
    int i, j;
    const char* name;
    SDL_Surface *screen = NULL;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_WM_SetCaption("Event Catcher", "Event Catcher");

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_HWSURFACE | SDL_ANYFORMAT);
    if (screen == NULL)
    {
        fprintf(stderr, "Unable to create video surface: %s\n", SDL_GetError());
        return;
    }

    i = 0;
    while(i < MAX_DEVICES && (jstick[i] = SDL_JoystickOpen(i)))
    {
        joystickName[i] = wxString(SDL_JoystickName(i), wxConvISO8859_1);
        if(joystickName[i].StartsWith(_(BT_SIXAXIS_NAME)))
        {
          joystickName[i] = _(BT_SIXAXIS_NAME);
        }
        for(j=i-1; j>=0; --j)
        {
          if(joystickName[i] == joystickName[j])
          {
            joystickVirtualIndex[i] = joystickVirtualIndex[j]+1;
            break;
          }
        }
        if(j < 0)
        {
          joystickVirtualIndex[i] = 0;
        }
        joystickNbButton[i] = SDL_JoystickNumButtons(jstick[i]);
        if(joystickName[i] == _("Sony PLAYSTATION(R)3 Controller"))
        {
          joystickSixaxis[i] = 1;
        }
        i++;
    }
    if(i < MAX_DEVICES)
    {
        jstick[i] = NULL;
    }

    i = 0;
    while ((name = SDL_GetMouseName(i)))
    {
      mouseName[i] = wxString(name, wxConvISO8859_1);

      for (j = i - 1; j >= 0; --j)
      {
        if (mouseName[i] == mouseName[j])
        {
          mouseVirtualIndex[i] = mouseVirtualIndex[j] + 1;
          break;
        }
      }
      if (j < 0)
      {
        mouseVirtualIndex[i] = 0;
      }
      i++;
    }
    if(i < MAX_DEVICES)
    {
      mouseName[i].Empty();
    }

    i = 0;
    while ((name = SDL_GetKeyboardName(i)))
    {
      keyboardName[i] = wxString(name, wxConvISO8859_1);

      for (j = i - 1; j >= 0; --j)
      {
        if (keyboardName[i] == keyboardName[j])
        {
          keyboardVirtualIndex[i] = keyboardVirtualIndex[j] + 1;
          break;
        }
      }
      if (j < 0)
      {
        keyboardVirtualIndex[i] = 0;
      }
      i++;
    }
    if(i < MAX_DEVICES)
    {
      keyboardName[i].Empty();
    }
}

void event_catcher::close_devices()
{
    int i;
    for(i=0; i<MAX_DEVICES && jstick[i]; ++i)
    {
      SDL_JoystickClose(jstick[i]);
    }
}

bool event_catcher::check_device(wxString device_type, wxString device_name, wxString device_id)
{
    int i;
    int nb = 0;
    if(device_type == _("keyboard"))
    {
      for(i=0; i<MAX_DEVICES && !keyboardName[i].IsEmpty(); ++i)
      {
        if(keyboardName[i] == device_name)
        {
          if(nb == wxAtoi(device_id))
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
      }
    }
    else if(device_type == _("mouse"))
    {
      for(i=0; i<MAX_DEVICES && !mouseName[i].IsEmpty(); ++i)
      {
        if(mouseName[i] == device_name)
        {
          if(nb == wxAtoi(device_id))
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
      }
    }
    else if(device_type == _("joystick"))
    {
      for(i=0; i<MAX_DEVICES && jstick[i]; ++i)
      {
        if(joystickName[i] == device_name)
        {
          if(nb == wxAtoi(device_id))
          {
            return true;
          }
          else
          {
            nb++;
          }
        }
      }
    }
    return false;
}

void event_catcher::clean()
{
    close_devices();
    SDL_Quit();
}

void event_catcher::run(wxString device_type, wxString event_type)
{
    done = 0;
    SDL_Event events[EVENT_BUFFER_SIZE];
    SDL_Event* event;
    int num_evt;
    const char* event_id;

    m_DeviceType = wxEmptyString;
    m_DeviceId = wxEmptyString;
    m_DeviceName = wxEmptyString;
    m_EventType = wxEmptyString;
    m_EventId = wxEmptyString;

    init();

#ifndef WIN32
    usleep(250000);
#endif

    SDL_WM_GrabInput(SDL_GRAB_ON);

    SDL_ShowCursor(SDL_DISABLE);

    done = 0;

    /*
     * Purge events (launching the event catcher generates a mouse motion).
     */
    SDL_PumpEvents();
    num_evt = SDL_PeepEvents(events, sizeof(events) / sizeof(events[0]),
                             SDL_GETEVENT, SDL_ALLEVENTS);

    while (!done)
    {
        SDL_PumpEvents();
        num_evt = SDL_PeepEvents(events, sizeof(events) / sizeof(events[0]),
                                 SDL_GETEVENT, SDL_ALLEVENTS);
        if (num_evt > 0)
        {
            for (event = events; event < events + num_evt; ++event)
            {
                switch (event->type)
                {
                case SDL_KEYDOWN:
                    if(device_type != wxEmptyString && device_type != _("keyboard"))
                    {
                        break;
                    }
                    if(event_type == _("button"))
                    {
                        m_DeviceType = _("keyboard");
                        m_DeviceId = wxString::Format(wxT("%i"),keyboardVirtualIndex[event->key.which]);
                        m_DeviceName = keyboardName[event->key.which];
                        m_EventType = _("button");
                        event_id = get_chars_from_key(event->key.keysym.sym);
                        m_EventId = wxString(event_id, wxConvUTF8);
                        done = 1;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if(device_type != wxEmptyString && device_type != _("mouse"))
                    {
                        break;
                    }
                    if(event_type == _("button"))
                    {
                        m_DeviceType = _("mouse");
                        m_DeviceId = wxString::Format(wxT("%i"),mouseVirtualIndex[event->button.which]);
                        m_DeviceName = mouseName[event->button.which];
                        m_EventType = _("button");
                        event_id = get_chars_from_mouse_button(event->button.button);
                        m_EventId = wxString(event_id, wxConvUTF8);
                        done = 1;
                    }
                    break;
                case SDL_JOYBUTTONDOWN:
                    if(device_type != wxEmptyString && device_type != _("joystick"))
                    {
                        break;
                    }
                    if(event_type == _("button"))
                    {
                        m_DeviceType = _("joystick");
                        m_DeviceId = wxString::Format(wxT("%i"),joystickVirtualIndex[event->jbutton.which]);
                        m_DeviceName = joystickName[event->jbutton.which];
                        m_EventType = _("button");
                        m_EventId = wxString::Format(wxT("%i"),event->jbutton.button);
                        done = 1;
                    }
                    break;
                case SDL_JOYHATMOTION:
                    if(device_type != wxEmptyString && device_type != _("joystick"))
                    {
                        break;
                    }
                    if(event_type == _("button"))
                    {
                        m_DeviceType = _("joystick");
                        m_DeviceId = wxString::Format(wxT("%i"),joystickVirtualIndex[event->jhat.which]);
                        m_DeviceName = joystickName[event->jhat.which];
                        m_EventType = _("button");
                        if(event->jhat.value & SDL_HAT_UP)
                        {
                          m_EventId = wxString::Format(wxT("%i"),joystickNbButton[event->jhat.which]+4*event->jhat.hat);
                          done = 1;
                        }
                        else if(event->jhat.value & SDL_HAT_RIGHT)
                        {
                          m_EventId = wxString::Format(wxT("%i"),joystickNbButton[event->jhat.which]+4*event->jhat.hat+1);
                          done = 1;
                        }
                        else if(event->jhat.value & SDL_HAT_DOWN)
                        {
                          m_EventId = wxString::Format(wxT("%i"),joystickNbButton[event->jhat.which]+4*event->jhat.hat+2);
                          done = 1;
                        }
                        else if(event->jhat.value & SDL_HAT_LEFT)
                        {
                          m_EventId = wxString::Format(wxT("%i"),joystickNbButton[event->jhat.which]+4*event->jhat.hat+3);
                          done = 1;
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if(device_type != wxEmptyString && device_type != _("mouse"))
                    {
                        break;
                    }
                    if(event_type == _("axis"))
                    {
                        if(abs(event->motion.xrel) > 5 || abs(event->motion.yrel) > 5)
                        {
                            m_DeviceType = _("mouse");
                            m_DeviceId = wxString::Format(wxT("%i"),mouseVirtualIndex[event->motion.which]);
                            m_DeviceName = mouseName[event->button.which];
                            m_EventType = _("axis");
                            done = 1;
                            if(abs(event->motion.xrel) > abs(event->motion.yrel))
                            {
                                m_EventId = _("x");
                            }
                            else
                            {
                                m_EventId = _("y");
                            }
                        }
                    }
                    else if(event_type == _("axis up"))
                    {
                        if(event->motion.xrel > 5 || event->motion.yrel > 5)
                        {
                            m_DeviceType = _("mouse");
                            m_DeviceId = wxString::Format(wxT("%i"),mouseVirtualIndex[event->motion.which]);
                            m_DeviceName = mouseName[event->button.which];
                            m_EventType = _("axis");
                            done = 1;
                            if(event->motion.xrel > event->motion.yrel)
                            {
                                m_EventId = _("x");
                            }
                            else
                            {
                                m_EventId = _("y");
                            }
                        }
                    }
                    else if(event_type == _("axis down"))
                    {
                        if(event->motion.xrel < -5 || event->motion.yrel < -5)
                        {
                            m_DeviceType = _("mouse");
                            m_DeviceId = wxString::Format(wxT("%i"),mouseVirtualIndex[event->motion.which]);
                            m_DeviceName = mouseName[event->button.which];
                            m_EventType = _("axis");
                            done = 1;
                            if(event->motion.xrel < event->motion.yrel)
                            {
                                m_EventId = _("x");
                            }
                            else
                            {
                                m_EventId = _("y");
                            }
                        }
                    }
                    break;
                case SDL_JOYAXISMOTION:
                    if(device_type != wxEmptyString && device_type != _("joystick"))
                    {
                        break;
                    }
                    /*
                     * Ugly patch for the sixaxis.
                     */
                    if(joystickSixaxis[event->jaxis.which] && event->jaxis.axis > 3)
                    {
                        event->jaxis.value = (event->jaxis.value + 32767) / 2;
                    }
                    if(event_type == _("axis"))
                    {
                        if(abs(event->jaxis.value) > 10000)
                        {
                            m_DeviceType = _("joystick");
                            m_DeviceId = wxString::Format(wxT("%i"),joystickVirtualIndex[event->jaxis.which]);
                            m_DeviceName = joystickName[event->jaxis.which];
                            m_EventType = _("axis");
                            m_EventId = wxString::Format(wxT("%i"),event->jaxis.axis);
                            done = 1;
                        }
                    }
                    else if(event_type == _("axis up"))
                    {
                        if(event->jaxis.value > 10000)
                        {
                            m_DeviceType = _("joystick");
                            m_DeviceId = wxString::Format(wxT("%i"),joystickVirtualIndex[event->jaxis.which]);
                            m_DeviceName = joystickName[event->jaxis.which];
                            m_EventType = _("axis");
                            m_EventId = wxString::Format(wxT("%i"),event->jaxis.axis);
                            done = 1;
                        }
                    }
                    else if(event_type == _("axis down"))
                    {
                        if(event->jaxis.value < -10000)
                        {
                            m_DeviceType = _("joystick");
                            m_DeviceId = wxString::Format(wxT("%i"),joystickVirtualIndex[event->jaxis.which]);
                            m_DeviceName = joystickName[event->jaxis.which];
                            m_EventType = _("axis");
                            m_EventId = wxString::Format(wxT("%i"),event->jaxis.axis);
                            done = 1;
                        }
                    }
                    break;
                }
            }
        }
        usleep(10000);
    }

    clean();
}
