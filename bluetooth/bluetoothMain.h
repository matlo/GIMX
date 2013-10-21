/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef BLUETOOTHMAIN_H
#define BLUETOOTHMAIN_H

//(*Headers(bluetoothFrame)
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/snglinst.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/utils.h>
//*)

#include <wx/process.h>

class bluetoothFrame: public wxFrame
{
    public:

        bluetoothFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~bluetoothFrame();

        void OnProcessTerminated(wxProcess *process, int status);

    private:

        //(*Handlers(bluetoothFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnSelectSixaxisBdaddr(wxCommandEvent& event);
        void OnChoice2Select(wxCommandEvent& event);
        void OnSelectPS3Bdaddr(wxCommandEvent& event);
        void OnSelectBtDongle(wxCommandEvent& event);
        void OnSelectRefresh(wxCommandEvent& event);
        void OnButton2Click(wxCommandEvent& event);
        void OnChoice5Select(wxCommandEvent& event);
        void OnChoice6Select(wxCommandEvent& event);
        void OnChoice7Select(wxCommandEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void OnButton3Click(wxCommandEvent& event);
        void OnCheckBox1Click(wxCommandEvent& event);
        void OnSave(wxCommandEvent& event);
        void OnCheckBox2Click(wxCommandEvent& event);
        void OnCheckBox3Click(wxCommandEvent& event);
        void OnChoice8Select(wxCommandEvent& event);
        void OnButton4Click(wxCommandEvent& event);
        void OnMenuEditConfig(wxCommandEvent& event);
        void OnMenuEditFpsConfig(wxCommandEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuStartUpdates(wxCommandEvent& event);
        void OnMenuGetConfigs(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        void OnMenuOpenConfigDirectory(wxCommandEvent& event);
        //*)

        void readSixaxis();
        void readDongles();
        int setDongleAddress();
        void refresh();
        void autoBindControls(wxArrayString configs);

        //(*Identifiers(bluetoothFrame)
        static const long ID_STATICTEXT1;
        static const long ID_CHOICE1;
        static const long ID_STATICTEXT2;
        static const long ID_CHOICE2;
        static const long ID_STATICTEXT3;
        static const long ID_CHOICE3;
        static const long ID_STATICTEXT4;
        static const long ID_CHOICE5;
        static const long ID_STATICTEXT6;
        static const long ID_CHOICE6;
        static const long ID_STATICTEXT7;
        static const long ID_CHOICE7;
        static const long ID_STATICTEXT8;
        static const long ID_BUTTON2;
        static const long ID_CHECKBOX5;
        static const long ID_CHOICE8;
        static const long ID_BUTTON1;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT9;
        static const long ID_STATICTEXT10;
        static const long ID_STATICTEXT11;
        static const long ID_STATICTEXT12;
        static const long ID_STATICTEXT13;
        static const long ID_STATICTEXT14;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX2;
        static const long ID_CHECKBOX3;
        static const long ID_CHECKBOX6;
        static const long ID_CHECKBOX7;
        static const long ID_CHOICE4;
        static const long ID_BUTTON4;
        static const long ID_BUTTON3;
        static const long ID_PANEL1;
        static const long ID_MENUITEM3;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM8;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM9;
        static const long idMenuQuit;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM5;
        static const long ID_MENUITEM6;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(bluetoothFrame)
        wxPanel* Panel1;
        wxStaticText* StaticText13;
        wxStaticText* StaticText14;
        wxStatusBar* StatusBar1;
        wxChoice* Choice3;
        wxMenuItem* MenuAutoBindControls;
        wxButton* Button4;
        wxChoice* ChoiceConfig;
        wxButton* Button1;
        wxMenuItem* MenuGetConfigs;
        wxCheckBox* CheckBox1;
        wxCheckBox* CheckBox6;
        wxButton* Button2;
        wxButton* Button3;
        wxChoice* Choice2;
        wxMenuItem* MenuUpdate;
        wxStaticText* StaticText1;
        wxStaticText* StaticText10;
        wxMenuItem* MenuStartUpdates;
        wxStaticText* StaticText3;
        wxMenuItem* MenuItem3;
        wxChoice* Choice5;
        wxChoice* Choice1;
        wxCheckBox* CheckBox2;
        wxStaticText* StaticText8;
        wxStaticText* StaticText12;
        wxCheckBox* CheckBox7;
        wxChoice* Choice7;
        wxStaticText* StaticText7;
        wxMenuItem* MenuItem5;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText2;
        wxMenuItem* MenuItem4;
        wxMenuItem* MenuItem6;
        wxCheckBox* CheckBox3;
        wxStaticText* StaticText6;
        wxSingleInstanceChecker SingleInstanceChecker1;
        wxChoice* Choice6;
        wxChoice* Choice8;
        wxStaticText* StaticText9;
        wxStaticText* StaticText11;
        wxCheckBox* CheckBox5;
        //*)

        wxLocale* locale;

        wxString m_SelectedSixaxis;
        wxString m_SelectedPS3;
        wxString m_SelectedBtDongle;
        int m_SelectedController;

        int pid[7];
        bool dongleInUse[256];
        int dongle[7];

        bool started;

        DECLARE_EVENT_TABLE()
};

#endif // BLUETOOTHMAIN_H
