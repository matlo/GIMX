/***************************************************************
 * Name:      sixstatusMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-02-17
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixstatusMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(sixstatusFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <sstream>
#include <wx/timer.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

using namespace std;

int max_axis_value = 256;
int mean_axis_value = max_axis_value/2;

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(sixstatusFrame)
const long sixstatusFrame::ID_STATICTEXT1 = wxNewId();
const long sixstatusFrame::ID_GAUGE1 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT4 = wxNewId();
const long sixstatusFrame::ID_GAUGE2 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT3 = wxNewId();
const long sixstatusFrame::ID_GAUGE3 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT2 = wxNewId();
const long sixstatusFrame::ID_GAUGE4 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT5 = wxNewId();
const long sixstatusFrame::ID_GAUGE5 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT6 = wxNewId();
const long sixstatusFrame::ID_GAUGE6 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT7 = wxNewId();
const long sixstatusFrame::ID_GAUGE7 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT8 = wxNewId();
const long sixstatusFrame::ID_GAUGE8 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT9 = wxNewId();
const long sixstatusFrame::ID_GAUGE9 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT10 = wxNewId();
const long sixstatusFrame::ID_GAUGE10 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT11 = wxNewId();
const long sixstatusFrame::ID_GAUGE11 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT12 = wxNewId();
const long sixstatusFrame::ID_GAUGE12 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT13 = wxNewId();
const long sixstatusFrame::ID_GAUGE13 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT14 = wxNewId();
const long sixstatusFrame::ID_GAUGE14 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT15 = wxNewId();
const long sixstatusFrame::ID_GAUGE15 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT16 = wxNewId();
const long sixstatusFrame::ID_GAUGE16 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT17 = wxNewId();
const long sixstatusFrame::ID_GAUGE17 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT18 = wxNewId();
const long sixstatusFrame::ID_GAUGE18 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT31 = wxNewId();
const long sixstatusFrame::ID_GAUGE31 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT32 = wxNewId();
const long sixstatusFrame::ID_GAUGE32 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT33 = wxNewId();
const long sixstatusFrame::ID_GAUGE33 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT42 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT43 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT39 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT38 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT41 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT40 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT19 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT21 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT20 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT22 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT23 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT24 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT25 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT26 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT37 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT34 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT27 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT28 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT29 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT30 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT46 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT47 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT36 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT35 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT44 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT45 = wxNewId();
const long sixstatusFrame::ID_STATICTEXT48 = wxNewId();
const long sixstatusFrame::idMenuQuit = wxNewId();
const long sixstatusFrame::idMenuAbout = wxNewId();
const long sixstatusFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(sixstatusFrame,wxFrame)
    //(*EventTable(sixstatusFrame)
    //*)
END_EVENT_TABLE()

typedef enum
{
  NONE,
  MC,
  CC,
  MX,
  MY,
  DZX,
  DZY,
  DZS,
  RD,
  VE,
  EX,
  EY,
  TEST
}e_current_cal;

static e_current_cal current_cal;

int lstick_x = 0;
int lstick_y = 0;
int rstick_x = 0;
int rstick_y = 0;
int bup = 0;
int bdown = 0;
int bright = 0;
int bleft = 0;
int br1 = 0;
int br2 = 0;
int bl1 = 0;
int bl2 = 0;
int bcircle = 0;
int bsquare = 0;
int bcross = 0;
int btriangle = 0;
int br3 = 0;
int bl3 = 0;
int bstart = 0;
int bselect = 0;
int bps = 0;

string status;
string cm;
string cc;
string dz_x;
string dz_y;
string mul_x;
string xyratio;
string exponent_x;
string exponent_y;
string shape;
string radius;
string velocity;

void read_status(void)
{
    string s;
    int skip;

    while(cin && getline(cin, s))
    {
        //cout << s << endl;
        stringstream parser(s);

        if(!s.compare(0, 16, "calibration done"))
        {
            status = string("config saved");
            current_cal = NONE;
        }
        else if(!s.compare(0, 17, "calibrating mouse"))
        {
            cm = parser.str().substr(18);
            status = string("config not saved");
        }
        else if(!s.compare(0, 16, "calibrating conf"))
        {
            parser >> s;
            parser >> s;
            parser >> cc;
        }
        else if(!s.compare(0, 15, "mouse selection"))
        {
            current_cal = MC;
        }
        else if(!s.compare(0, 16, "config selection"))
        {
            current_cal = CC;
        }
        else if(!s.compare(0, 24, "calibrating multiplier x"))
        {
            current_cal = MX;
        }
        else if(!s.compare(0, 21, "calibrating x/y ratio"))
        {
            current_cal = MY;
        }
        else if(!s.compare(0, 23, "calibrating dead zone x"))
        {
            current_cal = DZX;
        }
        else if(!s.compare(0, 23, "calibrating dead zone y"))
        {
            current_cal = DZY;
        }
        else if(!s.compare(0, 27, "calibrating dead zone shape"))
        {
            current_cal = DZS;
        }
        else if(!s.compare(0, 22, "calibrating exponent x"))
        {
            current_cal = EX;
        }
        else if(!s.compare(0, 22, "calibrating exponent y"))
        {
            current_cal = EY;
        }
        else if(!s.compare(0, 28, "adjusting circle test radius"))
        {
            current_cal = RD;
        }
        else if(!s.compare(0, 30, "adjusting circle test velocity"))
        {
            current_cal = VE;
        }
        else if(!s.compare(0, 24, "translation test started"))
        {
            current_cal = TEST;
        }
        else if(!s.compare(0, 13, "multiplier_x:"))
        {
            parser >> s;
            parser >> mul_x;
        }
        else if(!s.compare(0, 10, "x/y_ratio:"))
        {
            parser >> s;
            parser >> xyratio;
        }
        else if(!s.compare(0, 12, "dead_zone_x:"))
        {
            parser >> s;
            parser >> dz_x;
        }
        else if(!s.compare(0, 12, "dead_zone_y:"))
        {
            parser >> s;
            parser >> dz_y;
        }
        else if(!s.compare(0, 11, "exponent_x:"))
        {
            parser >> s;
            parser >> exponent_x;
        }
        else if(!s.compare(0, 11, "exponent_y:"))
        {
            parser >> s;
            parser >> exponent_y;
        }
        else if(!s.compare(0, 6, "shape:"))
        {
            parser >> s;
            parser >> shape;
        }
        else if(!s.compare(0, 7, "radius:"))
        {
            parser >> s;
            parser >> radius;
        }
        else if(!s.compare(0, 9, "velocity:"))
        {
            parser >> s;
            parser >> velocity;
        }
        else if(!s.compare(0, 15, "max_axis_value:"))
        {
            parser >> s;
            parser >> max_axis_value;
            mean_axis_value = max_axis_value/2;
        }
#ifdef WIN32
        /*
         * Hackish way to keep emuclient window in foreground.
         */
        else if(!s.compare(0, 15, "sdl initialized"))
        {
          HWND Handle = FindWindow(NULL, TEXT("Sixaxis Control"));
          if(Handle)
          {
            SetForegroundWindow(Handle);
          }
        }
#endif

        while(parser >> s)
        {
            if(s == "axis")
            {
                break;
            }
        }

        parser.ignore(2);
        parser >> lstick_x;
        parser.ignore(1);
        parser >> lstick_y;
        parser.ignore(3);
        parser >> rstick_x;
        parser.ignore(1);
        parser >> rstick_y;
        parser.ignore(7);
        parser >> skip;
        parser.ignore(1);
        parser >> skip;
        parser.ignore(1);
        parser >> skip;
        parser.ignore(1);
        parser >> skip;
        parser.ignore(2);

        bup = 0;
        bdown = 0;
        bright = 0;
        bleft = 0;
        br1 = 0;
        br2 = 0;
        bl1 = 0;
        bl2 = 0;
        bcircle = 0;
        bsquare = 0;
        bcross = 0;
        btriangle = 0;
        br3 = 0;
        bl3 = 0;
        bstart = 0;
        bselect = 0;
        bps = 0;

        while(parser >> s)
        {
            //cout << s << endl;
            if(!s.compare(0, 3, "up-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> bup;
            }
            else if(!s.compare(0, 5, "down-"))
            {
                string test (s, 5, 2);
                stringstream converter(test);
                converter >> hex >> bdown;
            }
            else if(!s.compare(0, 6, "right-"))
            {
                string test (s, 6, 2);
                stringstream converter(test);
                converter >> hex >> bright;
            }
            else if(!s.compare(0, 5, "left-"))
            {
                string test (s, 5, 2);
                stringstream converter(test);
                converter >> hex >> bleft;
            }
            else if(!s.compare(0, 3, "r1-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> br1;
            }
            else if(!s.compare(0, 3, "r2-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> br2;
            }
            else if(!s.compare(0, 3, "l1-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> bl1;
            }
            else if(!s.compare(0, 3, "l2-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> bl2;
            }
            else if(!s.compare(0, 3, "r1-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> br1;
            }
            else if(!s.compare(0, 3, "r2-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> br2;
            }
            else if(!s.compare(0, 7, "circle-"))
            {
                string test (s, 7, 2);
                stringstream converter(test);
                converter >> hex >> bcircle;
            }
            else if(!s.compare(0, 7, "square-"))
            {
                string test (s, 7, 2);
                stringstream converter(test);
                converter >> hex >> bsquare;
            }
            else if(!s.compare(0, 6, "cross-"))
            {
                string test (s, 6, 2);
                stringstream converter(test);
                converter >> hex >> bcross;
            }
            else if(!s.compare(0, 9, "triangle-"))
            {
                string test (s, 9, 2);
                stringstream converter(test);
                converter >> hex >> btriangle;
            }
            else if(!s.compare(0, 3, "r3-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> br3;
            }
            else if(!s.compare(0, 3, "l3-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> bl3;
            }
            else if(!s.compare(0, 6, "start-"))
            {
                string test (s, 6, 2);
                stringstream converter(test);
                converter >> hex >> bstart;
            }
            else if(!s.compare(0, 7, "select-"))
            {
                string test (s, 7, 2);
                stringstream converter(test);
                converter >> hex >> bselect;
            }
            else if(!s.compare(0, 3, "ps-"))
            {
                string test (s, 3, 2);
                stringstream converter(test);
                converter >> hex >> bps;
            }
        }
        //usleep(1000);
        //cout << lstick_x << " " << lstick_y << " " << rstick_x << " " << rstick_y << endl;
    }
}

sixstatusFrame::sixstatusFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(sixstatusFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItem2;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer7;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxGridSizer* GridSizer1;
    wxFlexGridSizer* FlexGridSizer8;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxMenu* Menu2;
    wxStaticBoxSizer* StaticBoxSizer5;
    
    Create(parent, wxID_ANY, _("Gimx-status"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Sixaxis status"));
    GridSizer1 = new wxGridSizer(6, 8, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Left Stick x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    GridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge1 = new wxGauge(this, ID_GAUGE1, 255, wxDefaultPosition, wxSize(-1,-1), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE1"));
    Gauge1->SetValue(127);
    GridSizer1->Add(Gauge1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Left Stick y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    GridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge2 = new wxGauge(this, ID_GAUGE2, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE2"));
    Gauge2->SetValue(127);
    GridSizer1->Add(Gauge2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Right Stick x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    GridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge3 = new wxGauge(this, ID_GAUGE3, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE3"));
    Gauge3->SetValue(127);
    GridSizer1->Add(Gauge3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Right Stick y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    GridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge4 = new wxGauge(this, ID_GAUGE4, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE4"));
    Gauge4->SetValue(127);
    GridSizer1->Add(Gauge4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Up"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    GridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge5 = new wxGauge(this, ID_GAUGE5, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE5"));
    GridSizer1->Add(Gauge5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    GridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge6 = new wxGauge(this, ID_GAUGE6, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE6"));
    GridSizer1->Add(Gauge6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Right"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    GridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge7 = new wxGauge(this, ID_GAUGE7, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE7"));
    GridSizer1->Add(Gauge7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Left"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    GridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge8 = new wxGauge(this, ID_GAUGE8, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE8"));
    GridSizer1->Add(Gauge8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("R1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    GridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge9 = new wxGauge(this, ID_GAUGE9, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE9"));
    GridSizer1->Add(Gauge9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("R2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    GridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge10 = new wxGauge(this, ID_GAUGE10, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE10"));
    GridSizer1->Add(Gauge10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("L1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    GridSizer1->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge11 = new wxGauge(this, ID_GAUGE11, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE11"));
    GridSizer1->Add(Gauge11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("L2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    GridSizer1->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge12 = new wxGauge(this, ID_GAUGE12, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE12"));
    GridSizer1->Add(Gauge12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    GridSizer1->Add(StaticText13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge13 = new wxGauge(this, ID_GAUGE13, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE13"));
    GridSizer1->Add(Gauge13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Square"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    GridSizer1->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge14 = new wxGauge(this, ID_GAUGE14, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE14"));
    GridSizer1->Add(Gauge14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText15 = new wxStaticText(this, ID_STATICTEXT15, _("Cross"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    GridSizer1->Add(StaticText15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge15 = new wxGauge(this, ID_GAUGE15, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE15"));
    GridSizer1->Add(Gauge15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText16 = new wxStaticText(this, ID_STATICTEXT16, _("Triangle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    GridSizer1->Add(StaticText16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge16 = new wxGauge(this, ID_GAUGE16, 255, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE16"));
    GridSizer1->Add(Gauge16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(this, ID_STATICTEXT17, _("R3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    GridSizer1->Add(StaticText17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge17 = new wxGauge(this, ID_GAUGE17, 255, wxDefaultPosition, wxSize(30,30), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE17"));
    GridSizer1->Add(Gauge17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText18 = new wxStaticText(this, ID_STATICTEXT18, _("L3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    GridSizer1->Add(StaticText18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge18 = new wxGauge(this, ID_GAUGE18, 255, wxDefaultPosition, wxSize(30,30), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE18"));
    GridSizer1->Add(Gauge18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(this, ID_STATICTEXT31, _("Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    GridSizer1->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge31 = new wxGauge(this, ID_GAUGE31, 255, wxDefaultPosition, wxSize(30,30), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE31"));
    GridSizer1->Add(Gauge31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText32 = new wxStaticText(this, ID_STATICTEXT32, _("Select"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    GridSizer1->Add(StaticText32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge32 = new wxGauge(this, ID_GAUGE32, 255, wxDefaultPosition, wxSize(30,30), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE32"));
    GridSizer1->Add(Gauge32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText33 = new wxStaticText(this, ID_STATICTEXT33, _("PS"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    GridSizer1->Add(StaticText33, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Gauge33 = new wxGauge(this, ID_GAUGE33, 255, wxDefaultPosition, wxSize(30,30), wxGA_SMOOTH, wxDefaultValidator, _T("ID_GAUGE33"));
    GridSizer1->Add(Gauge33, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(GridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Mouse calibration (use the mouse wheel to change values)"));
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer8 = new wxFlexGridSizer(3, 2, 0, 0);
    StaticText42 = new wxStaticText(this, ID_STATICTEXT42, _("Status:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer8->Add(StaticText42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText43 = new wxStaticText(this, ID_STATICTEXT43, _("off"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer8->Add(StaticText43, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText39 = new wxStaticText(this, ID_STATICTEXT39, _("Mouse (rctrl+F1):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer8->Add(StaticText39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText38 = new wxStaticText(this, ID_STATICTEXT38, _("Name id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer8->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText41 = new wxStaticText(this, ID_STATICTEXT41, _("Config (F2):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer8->Add(StaticText41, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText40 = new wxStaticText(this, ID_STATICTEXT40, _("0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer8->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(1, 4, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Multiplier"));
    FlexGridSizer3 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticText19 = new wxStaticText(this, ID_STATICTEXT19, _("x (F3):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer3->Add(StaticText19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(this, ID_STATICTEXT21, _("00.00"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer3->Add(StaticText21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText20 = new wxStaticText(this, ID_STATICTEXT20, _("x/y ratio (F4):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    FlexGridSizer3->Add(StaticText20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(this, ID_STATICTEXT22, _("00.00"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer3->Add(StaticText22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Dead zone"));
    FlexGridSizer4 = new wxFlexGridSizer(3, 2, 0, 0);
    StaticText23 = new wxStaticText(this, ID_STATICTEXT23, _("x (F5):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer4->Add(StaticText23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText24 = new wxStaticText(this, ID_STATICTEXT24, _("000"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer4->Add(StaticText24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText25 = new wxStaticText(this, ID_STATICTEXT25, _("y (F6):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer4->Add(StaticText25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText26 = new wxStaticText(this, ID_STATICTEXT26, _("000"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer4->Add(StaticText26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText37 = new wxStaticText(this, ID_STATICTEXT37, _("Shape (F7):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer4->Add(StaticText37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText34 = new wxStaticText(this, ID_STATICTEXT34, _("Rectangle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer4->Add(StaticText34, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Exponent"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticText27 = new wxStaticText(this, ID_STATICTEXT27, _("x (F8):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer2->Add(StaticText27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText28 = new wxStaticText(this, ID_STATICTEXT28, _("0.00"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer2->Add(StaticText28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText29 = new wxStaticText(this, ID_STATICTEXT29, _("y (F9):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    FlexGridSizer2->Add(StaticText29, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(this, ID_STATICTEXT30, _("0.00"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer2->Add(StaticText30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7 = new wxStaticBoxSizer(wxVERTICAL, this, _("Advanced tests"));
    FlexGridSizer6 = new wxFlexGridSizer(4, 2, 0, 0);
    StaticText46 = new wxStaticText(this, ID_STATICTEXT46, _("Circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    FlexGridSizer6->Add(StaticText46, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText47 = new wxStaticText(this, ID_STATICTEXT47, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT47"));
    FlexGridSizer6->Add(StaticText47, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText36 = new wxStaticText(this, ID_STATICTEXT36, _("radius (F10):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer6->Add(StaticText36, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText35 = new wxStaticText(this, ID_STATICTEXT35, _("00000"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer6->Add(StaticText35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText44 = new wxStaticText(this, ID_STATICTEXT44, _("velocity (F11):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer6->Add(StaticText44, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText45 = new wxStaticText(this, ID_STATICTEXT45, _("000"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
    FlexGridSizer6->Add(StaticText45, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText48 = new wxStaticText(this, ID_STATICTEXT48, _("Translation (F12)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer6->Add(StaticText48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixstatusFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixstatusFrame::OnAbout);
    //*)

#ifdef WIN32
    monTimer.SetOwner(this, 1);
    monTimer.Start(10);
    Connect(1, wxEVT_TIMER,(wxObjectEventFunction)&sixstatusFrame::OnTimer);
#else
    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(sixstatusFrame::OnIdle) );
#endif
    pthread_t thread;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    pthread_create( &thread, &thread_attr, (void* (*)(void*))read_status, NULL);
}

sixstatusFrame::~sixstatusFrame()
{
    //(*Destroy(sixstatusFrame)
    //*)
}

void sixstatusFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void sixstatusFrame::OnAbout(wxCommandEvent& event)
{
  wxString msg = _("Gimx-status\n(c) Matlo GNU GPL\nHomepage: http://www.gimx.fr/\nSource code: http://code.google.com/p/diyps3controller/\nForum: http://www.forum.gimx.fr/\n");
  wxMessageBox(msg, _("Welcome to..."));
}

int changed = 0;

static void set_text(wxStaticText* text, string s)
{
  wxString ws = wxString(s.c_str(), wxConvISO8859_1);
  if(text->GetLabel() != ws)
  {
    text->SetLabel(ws);
  }
}

static void set_text_color(wxStaticText* text, wxColour colour)
{
  if(text->GetForegroundColour() != colour)
  {
    text->SetForegroundColour(colour);
    text->SetLabel(text->GetLabel());
  }
}

static void clamp(wxGauge* Gauge, int val, wxStaticText* Text)
{
    int high = Gauge->GetRange();
    if(val < 0)
    {
        if(Gauge->GetValue() != 0)
        {
          changed = 1;
          Gauge->SetValue(0);
          set_text_color(Text, wxColour(255, 0, 0));
        }
    }
    else if(val > high)
    {
        if(Gauge->GetValue() != high)
        {
          changed = 1;
          Gauge->SetValue(high);
          set_text_color(Text, wxColour(255, 0, 0));
        }
    }
    else
    {
        if(Gauge->GetValue() != val)
        {
          changed = 1;
          Gauge->SetValue(val);
          set_text_color(Text, wxColour(0, 0, 0));
        }
    }
}

void sixstatusFrame::TextColor()
{
  switch(current_cal)
  {
    case NONE:
      set_text_color(StaticText43, wxColour(255, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case MC:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(255, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case CC:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(255, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case MX:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(255, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case MY:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(255, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case DZX:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(255, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case DZY:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(255, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case DZS:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(255, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case RD:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(255, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case VE:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(255, 0, 0));
      break;
    case EX:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(255, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case EY:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(255, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
    case TEST:
      set_text_color(StaticText43, wxColour(0, 0, 0));
      set_text_color(StaticText38, wxColour(0, 0, 0));
      set_text_color(StaticText40, wxColour(0, 0, 0));
      set_text_color(StaticText21, wxColour(0, 0, 0));
      set_text_color(StaticText22, wxColour(0, 0, 0));
      set_text_color(StaticText24, wxColour(0, 0, 0));
      set_text_color(StaticText26, wxColour(0, 0, 0));
      set_text_color(StaticText28, wxColour(0, 0, 0));
      set_text_color(StaticText30, wxColour(0, 0, 0));
      set_text_color(StaticText34, wxColour(0, 0, 0));
      set_text_color(StaticText35, wxColour(0, 0, 0));
      set_text_color(StaticText45, wxColour(0, 0, 0));
      break;
  }
}

void sixstatusFrame::Update()
{
    changed = 0;
#ifndef WIN32
    if(current_cal == NONE)
    {
#endif
    if(Gauge1->GetRange() != max_axis_value)
    {
      Gauge1->SetRange(max_axis_value);
      Gauge2->SetRange(max_axis_value);
      Gauge3->SetRange(max_axis_value);
      Gauge4->SetRange(max_axis_value);
    }
    clamp(Gauge1, mean_axis_value+lstick_x, StaticText1);
    clamp(Gauge2, mean_axis_value-lstick_y, StaticText4);
    clamp(Gauge3, mean_axis_value+rstick_x, StaticText3);
    clamp(Gauge4, mean_axis_value-rstick_y, StaticText2);
    clamp(Gauge5, bup, StaticText5);
    clamp(Gauge6, bdown, StaticText6);
    clamp(Gauge7, bright, StaticText7);
    clamp(Gauge8, bleft, StaticText8);
    clamp(Gauge9, br1, StaticText9);
    clamp(Gauge10, br2, StaticText10);
    clamp(Gauge11, bl1, StaticText11);
    clamp(Gauge12, bl2, StaticText12);
    clamp(Gauge13, bcircle, StaticText13);
    clamp(Gauge14, bsquare, StaticText14);
    clamp(Gauge15, bcross, StaticText15);
    clamp(Gauge16, btriangle, StaticText16);
    clamp(Gauge17, br3, StaticText17);
    clamp(Gauge18, bl3, StaticText18);
    clamp(Gauge31, bstart, StaticText31);
    clamp(Gauge32, bselect, StaticText32);
    clamp(Gauge33, bps, StaticText33);
#ifndef WIN32
    }
#endif

    set_text(StaticText43, status);
    set_text(StaticText38, cm);
    set_text(StaticText40, cc);
    set_text(StaticText21, mul_x);
    set_text(StaticText22, xyratio);
    set_text(StaticText24, dz_x);
    set_text(StaticText26, dz_y);
    set_text(StaticText28, exponent_x);
    set_text(StaticText30, exponent_y);
    set_text(StaticText34, shape);
    set_text(StaticText35, radius);
    set_text(StaticText45, velocity);

    TextColor();

    //cout << mean_axis_value+lstick_x << " " << mean_axis_value-lstick_y << " " << mean_axis_value+rstick_x << " " << mean_axis_value-rstick_y << endl;

    if(!cin)
    {
        cout << "no more cin: exiting" << endl;
        exit(0);
    }
}

#ifndef WIN32
void sixstatusFrame::OnIdle(wxIdleEvent& evt)
{
    Update();

    if(changed)
    {
      Refresh();
    }

    usleep(10000);
    evt.RequestMore();
}
#else
void sixstatusFrame::OnTimer(wxTimerEvent& evt)
{
    Update();
}
#endif

