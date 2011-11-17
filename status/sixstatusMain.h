/***************************************************************
 * Name:      sixstatusMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-02-17
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef SIXSTATUSMAIN_H
#define SIXSTATUSMAIN_H

//(*Headers(sixstatusFrame)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/statusbr.h>
//*)

class sixstatusFrame: public wxFrame
{
    public:

        sixstatusFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~sixstatusFrame();

    private:

        //(*Handlers(sixstatusFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        //*)

        void OnTimer(wxTimerEvent& evt);
        void OnIdle(wxIdleEvent& evt);
        void TextColor();
        void Update();
        wxTimer monTimer;

        //(*Identifiers(sixstatusFrame)
        static const long ID_STATICTEXT1;
        static const long ID_GAUGE1;
        static const long ID_STATICTEXT4;
        static const long ID_GAUGE2;
        static const long ID_STATICTEXT3;
        static const long ID_GAUGE3;
        static const long ID_STATICTEXT2;
        static const long ID_GAUGE4;
        static const long ID_STATICTEXT5;
        static const long ID_GAUGE5;
        static const long ID_STATICTEXT6;
        static const long ID_GAUGE6;
        static const long ID_STATICTEXT7;
        static const long ID_GAUGE7;
        static const long ID_STATICTEXT8;
        static const long ID_GAUGE8;
        static const long ID_STATICTEXT9;
        static const long ID_GAUGE9;
        static const long ID_STATICTEXT10;
        static const long ID_GAUGE10;
        static const long ID_STATICTEXT11;
        static const long ID_GAUGE11;
        static const long ID_STATICTEXT12;
        static const long ID_GAUGE12;
        static const long ID_STATICTEXT13;
        static const long ID_GAUGE13;
        static const long ID_STATICTEXT14;
        static const long ID_GAUGE14;
        static const long ID_STATICTEXT15;
        static const long ID_GAUGE15;
        static const long ID_STATICTEXT16;
        static const long ID_GAUGE16;
        static const long ID_STATICTEXT17;
        static const long ID_GAUGE17;
        static const long ID_STATICTEXT18;
        static const long ID_GAUGE18;
        static const long ID_STATICTEXT31;
        static const long ID_GAUGE31;
        static const long ID_STATICTEXT32;
        static const long ID_GAUGE32;
        static const long ID_STATICTEXT33;
        static const long ID_GAUGE33;
        static const long ID_STATICTEXT42;
        static const long ID_STATICTEXT43;
        static const long ID_STATICTEXT39;
        static const long ID_STATICTEXT38;
        static const long ID_STATICTEXT41;
        static const long ID_STATICTEXT40;
        static const long ID_STATICTEXT19;
        static const long ID_STATICTEXT21;
        static const long ID_STATICTEXT20;
        static const long ID_STATICTEXT22;
        static const long ID_STATICTEXT23;
        static const long ID_STATICTEXT24;
        static const long ID_STATICTEXT25;
        static const long ID_STATICTEXT26;
        static const long ID_STATICTEXT37;
        static const long ID_STATICTEXT34;
        static const long ID_STATICTEXT27;
        static const long ID_STATICTEXT28;
        static const long ID_STATICTEXT29;
        static const long ID_STATICTEXT30;
        static const long ID_STATICTEXT36;
        static const long ID_STATICTEXT35;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(sixstatusFrame)
        wxStaticText* StaticText10;
        wxStaticText* StaticText22;
        wxStaticText* StaticText9;
        wxStaticText* StaticText20;
        wxGauge* Gauge3;
        wxStaticText* StaticText29;
        wxGauge* Gauge31;
        wxStaticText* StaticText37;
        wxStaticText* StaticText33;
        wxStaticText* StaticText13;
        wxStaticText* StaticText2;
        wxStaticText* StaticText30;
        wxStaticText* StaticText14;
        wxStaticText* StaticText26;
        wxStaticText* StaticText6;
        wxGauge* Gauge4;
        wxGauge* Gauge1;
        wxStaticText* StaticText40;
        wxGauge* Gauge12;
        wxGauge* Gauge6;
        wxStaticText* StaticText19;
        wxStaticText* StaticText42;
        wxStaticText* StaticText32;
        wxStaticText* StaticText38;
        wxGauge* Gauge32;
        wxStaticText* StaticText8;
        wxStaticText* StaticText11;
        wxStaticText* StaticText18;
        wxGauge* Gauge5;
        wxStaticText* StaticText31;
        wxStaticText* StaticText1;
        wxStaticText* StaticText27;
        wxGauge* Gauge18;
        wxStaticText* StaticText3;
        wxStaticText* StaticText21;
        wxStaticText* StaticText39;
        wxGauge* Gauge10;
        wxStaticText* StaticText23;
        wxStaticText* StaticText24;
        wxGauge* Gauge11;
        wxGauge* Gauge33;
        wxStaticText* StaticText34;
        wxStaticText* StaticText5;
        wxStaticText* StaticText7;
        wxStatusBar* StatusBar1;
        wxGauge* Gauge17;
        wxStaticText* StaticText28;
        wxStaticText* StaticText41;
        wxStaticText* StaticText43;
        wxStaticText* StaticText15;
        wxStaticText* StaticText12;
        wxGauge* Gauge2;
        wxGauge* Gauge8;
        wxGauge* Gauge15;
        wxStaticText* StaticText35;
        wxGauge* Gauge16;
        wxStaticText* StaticText25;
        wxStaticText* StaticText36;
        wxStaticText* StaticText17;
        wxStaticText* StaticText4;
        wxGauge* Gauge14;
        wxGauge* Gauge7;
        wxGauge* Gauge9;
        wxStaticText* StaticText16;
        wxGauge* Gauge13;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // SIXSTATUSMAIN_H
