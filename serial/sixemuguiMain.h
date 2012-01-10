/***************************************************************
 * Name:      sixemuguiMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef SIXEMUGUIMAIN_H
#define SIXEMUGUIMAIN_H

//(*Headers(sixemuguiFrame)
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

class sixemuguiFrame: public wxFrame
{
    public:

        sixemuguiFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~sixemuguiFrame();

    private:

        //(*Handlers(sixemuguiFrame)
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
        //*)

        //(*Identifiers(sixemuguiFrame)
        static const long ID_STATICTEXT3;
        static const long ID_COMBOBOX1;
        static const long ID_STATICTEXT4;
        static const long ID_COMBOBOX3;
        static const long ID_STATICTEXT1;
        static const long ID_COMBOBOX2;
        static const long ID_STATICTEXT2;
        static const long ID_CHECKBOX5;
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
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(sixemuguiFrame)
        wxCheckBox* CheckBoxCalibrate;
        wxMenuItem* MenuRefresh;
        wxMenuItem* MenuEditFpsConfig;
        wxStaticText* StaticText2;
        wxComboBox* ControllerType;
        wxCheckBox* CheckBoxGui;
        wxCheckBox* CheckBoxGrab;
        wxComboBox* ComboBoxDevice;
        wxCheckBox* CheckBoxTerminal;
        wxChoice* ChoiceConfig;
        wxPanel* Panel1;
        wxStaticText* StaticText1;
        wxStaticText* StaticText3;
        wxComboBox* ComboBoxFrequency;
        wxSingleInstanceChecker SingleInstanceChecker1;
        wxStatusBar* StatusBar1;
        wxButton* ButtonCheck;
        wxButton* ButtonStart;
        wxStaticText* StaticText4;
        wxMenuItem* MenuEditConfig;
        wxCheckBox* CheckBoxForceUpdates;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // SIXEMUGUIMAIN_H
