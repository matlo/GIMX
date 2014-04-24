/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef LAUNCHERMAIN_H
#define LAUNCHERMAIN_H

//(*Headers(launcherFrame)
#include <wx/combobox.h>
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

class launcherFrame: public wxFrame
{
    public:

        launcherFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~launcherFrame();

        void OnProcessTerminated(wxProcess *process, int status);

    private:

        //(*Handlers(launcherFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonCheckClick(wxCommandEvent& event);
        void OnButtonStartClick(wxCommandEvent& event);
        void OnCheckBoxGrabClick(wxCommandEvent& event);
        void OnCheckBoxGuiClick(wxCommandEvent& event);
        void OnCheckBoxTerminalClick(wxCommandEvent& event);
        void OnChoice8Select(wxCommandEvent& event);
        void OnButtonCheckClick1(wxCommandEvent& event);
        void OnMenuEditConfig(wxCommandEvent& event);
        void OnMenuEditFpsConfig(wxCommandEvent& event);
        void OnMenuRefresh(wxCommandEvent& event);
        void OnControllerTypeSelect(wxCommandEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuStartupUpdates(wxCommandEvent& event);
        void OnMenuGetConfigs(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        void OnMenuOpenConfigDirectory(wxCommandEvent& event);
        void OnsourceChoiceSelect(wxCommandEvent& event);
        void OnComboBoxDeviceSelected(wxCommandEvent& event);
        void OnMenuPS3Tools(wxCommandEvent& event);
        //*)

        void refresh();
        void autoBindControls(wxArrayString configs);

        void readSixaxis(wxArrayString addresses[2]);
        void readDongles(wxArrayString dongleInfos[4]);
        int setDongleAddress(wxArrayString dongles, wxString device, wxString address);

        //(*Identifiers(launcherFrame)
        static const long ID_STATICTEXT4;
        static const long ID_CHOICE1;
        static const long ID_STATICTEXT3;
        static const long ID_COMBOBOX1;
        static const long ID_STATICTEXT1;
        static const long ID_CHOICE2;
        static const long ID_STATICTEXT2;
        static const long ID_COMBOBOX2;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX2;
        static const long ID_CHECKBOX3;
        static const long ID_CHOICE4;
        static const long ID_BUTTON1;
        static const long ID_BUTTON3;
        static const long ID_PANEL1;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM8;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM3;
        static const long idMenuQuit;
        static const long ID_MENUITEM9;
        static const long ID_MENUITEM10;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM5;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(launcherFrame)
        wxStaticBoxSizer* MouseSizer;
        wxStaticText* DeviceText;
        wxCheckBox* CheckBoxTerminal;
        wxFlexGridSizer* FlexGridSizer1;
        wxPanel* Panel1;
        wxCheckBox* CheckBoxGui;
        wxStatusBar* StatusBar1;
        wxMenuItem* MenuAutoBindControls;
        wxButton* ButtonCheck;
        wxMenuItem* MenuEditFpsConfig;
        wxMenu* Menu3;
        wxChoice* ChoiceConfig;
        wxMenuItem* MenuGetConfigs;
        wxChoice* sourceChoice;
        wxMenuItem* MenuUpdate;
        wxStaticText* StaticText1;
        wxChoice* ControllerType;
        wxMenuItem* MenuRefresh;
        wxMenuItem* MenuStartupUpdates;
        wxMenuItem* MenuItem3;
        wxComboBox* ComboBox1;
        wxMenuItem* MenuPS3Tools;
        wxComboBox* ComboBoxDevice;
        wxStaticText* StaticText4;
        wxCheckBox* CheckBoxGrab;
        wxFlexGridSizer* SourceIpSizer;
        wxStaticText* StaticText2;
        wxMenuItem* MenuPS4Tools;
        wxButton* ButtonStart;
        wxMenuItem* MenuEditConfig;
        wxSingleInstanceChecker SingleInstanceChecker1;
        //*)

        wxLocale* locale;

        bool started;

        DECLARE_EVENT_TABLE()
};

#endif // LAUNCHERMAIN_H
