/***************************************************************
 * Name:      serialMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef SERIALMAIN_H
#define SERIALMAIN_H

//(*Headers(serialFrame)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/snglinst.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/utils.h>
#include <wx/frame.h>
#include <wx/combobox.h>
#include <wx/statusbr.h>
//*)

class serialFrame: public wxFrame
{
    public:

        serialFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~serialFrame();

    private:

        //(*Handlers(serialFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonCheckClick(wxCommandEvent& event);
        void OnButtonStartClick(wxCommandEvent& event);
        void OnCheckBoxGrabClick(wxCommandEvent& event);
        void OnCheckBoxGuiClick(wxCommandEvent& event);
        void OnCheckBoxTerminalClick(wxCommandEvent& event);
        void OnChoice8Select(wxCommandEvent& event);
        void OnCheckBoxCalibrate(wxCommandEvent& event);
        void OnButtonCheckClick1(wxCommandEvent& event);
        void OnMenuEditConfig(wxCommandEvent& event);
        void OnMenuEditFpsConfig(wxCommandEvent& event);
        void OnMenuRefresh(wxCommandEvent& event);
        void OnControllerTypeSelect(wxCommandEvent& event);
        void OnButtonSpoofClick(wxCommandEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuStartupUpdates(wxCommandEvent& event);
        void OnMenuGetConfigs(wxCommandEvent& event);
        //*)

        void refresh();

        //(*Identifiers(serialFrame)
        static const long ID_STATICTEXT3;
        static const long ID_COMBOBOX1;
        static const long ID_STATICTEXT4;
        static const long ID_CHOICE1;
        static const long ID_BUTTON2;
        static const long ID_STATICTEXT1;
        static const long ID_COMBOBOX2;
        static const long ID_STATICTEXT2;
        static const long ID_CHECKBOX5;
        static const long ID_CHECKBOX6;
        static const long ID_CHECKBOX1;
        static const long ID_CHECKBOX4;
        static const long ID_CHECKBOX2;
        static const long ID_CHECKBOX3;
        static const long ID_CHOICE4;
        static const long ID_BUTTON1;
        static const long ID_BUTTON3;
        static const long ID_PANEL1;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM3;
        static const long idMenuQuit;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM5;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(serialFrame)
        wxCheckBox* CheckBoxCalibrate;
        wxMenuItem* MenuRefresh;
        wxMenuItem* MenuEditFpsConfig;
        wxStaticText* StaticText2;
        wxCheckBox* CheckBoxGui;
        wxCheckBox* CheckBoxGrab;
        wxComboBox* ComboBoxDevice;
        wxCheckBox* CheckBoxTerminal;
        wxChoice* ChoiceConfig;
        wxPanel* Panel1;
        wxStaticText* StaticText1;
        wxCheckBox* CheckBoxSubpositions;
        wxStaticText* StaticText3;
        wxComboBox* ComboBoxFrequency;
        wxSingleInstanceChecker SingleInstanceChecker1;
        wxChoice* ControllerType;
        wxStatusBar* StatusBar1;
        wxButton* ButtonCheck;
        wxButton* ButtonSpoof;
        wxMenuItem* MenuGetConfigs;
        wxButton* ButtonStart;
        wxMenuItem* MenuStartupUpdates;
        wxStaticText* StaticText4;
        wxMenuItem* MenuEditConfig;
        wxCheckBox* CheckBoxForceUpdates;
        wxMenuItem* MenuUpdate;
        //*)

        bool spoofed;
        bool started;

        DECLARE_EVENT_TABLE()
};

#endif // SERIALMAIN_H
