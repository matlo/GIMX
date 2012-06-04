/***************************************************************
 * Name:      fpsconfigMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-08-06
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef FPSCONFIGMAIN_H
#define FPSCONFIGMAIN_H

//(*Headers(fpsconfigFrame)
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

#include <event_catcher.h>
#include <ConfigurationFile.h>

#include <wx/textdlg.h>

typedef enum button_index {
    bi_undef = 0,
    bi_select, bi_start, bi_ps,
    bi_up, bi_right, bi_down, bi_left,
    bi_triangle, bi_circle, bi_cross, bi_square,
    bi_l1, bi_r1,
    bi_l2, bi_r2,
    bi_l3, bi_r3,
    BI_MAX
} e_button_index;

typedef enum axis_index {
    ai_undef = 0,
    ai_ls_up, ai_ls_right, ai_ls_down, ai_ls_left,
    AI_MAX
} e_axis_index;

class fpsconfigFrame: public wxFrame
{
    public:

        fpsconfigFrame(wxString file,wxWindow* parent,wxWindowID id = -1);
        virtual ~fpsconfigFrame();

    private:

        //(*Handlers(fpsconfigFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonClick(wxCommandEvent& event);
        void OnMenuSave(wxCommandEvent& event);
        void OnMenuNew(wxCommandEvent& event);
        void OnMenuSaveAs(wxCommandEvent& event);
        void OnMenuOpen(wxCommandEvent& event);
        void OnSpinCtrlChange(wxSpinEvent& event);
        void OnTextCtrlText(wxCommandEvent& event);
        void OnMouseDPIChange(wxSpinEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuEditLabels(wxCommandEvent& event);
        void OnButtonConvertSensitivityClick(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        //*)

        e_button_index getButtonIndex(wxButton* button);
        e_axis_index getAxisIndex(wxButton* button);
        wxButton* getButtonButton(string blabel);
        wxButton* getAxisButton(string blabel);

        void LoadConfig();
        void readLabels();        

        //(*Identifiers(fpsconfigFrame)
        static const long ID_SPINCTRL8;
        static const long ID_SPINCTRL7;
        static const long ID_SPINCTRL6;
        static const long ID_SPINCTRL5;
        static const long ID_SPINCTRL4;
        static const long ID_SPINCTRL10;
        static const long ID_SPINCTRL11;
        static const long ID_SPINCTRL3;
        static const long ID_BUTTON10;
        static const long ID_SPINCTRL13;
        static const long ID_SPINCTRL14;
        static const long ID_SPINCTRL1;
        static const long ID_SPINCTRL2;
        static const long ID_TEXTCTRL24;
        static const long ID_TEXTCTRL4;
        static const long ID_TEXTCTRL26;
        static const long ID_TEXTCTRL2;
        static const long ID_TEXTCTRL3;
        static const long ID_TEXTCTRL22;
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT7;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT4;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT6;
        static const long ID_BUTTON1;
        static const long ID_BUTTON2;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_BUTTON5;
        static const long ID_BUTTON6;
        static const long ID_BUTTON7;
        static const long ID_BUTTON8;
        static const long ID_BUTTON9;
        static const long ID_BUTTON11;
        static const long ID_BUTTON12;
        static const long ID_BUTTON13;
        static const long ID_BUTTON14;
        static const long ID_BUTTON15;
        static const long ID_BUTTON16;
        static const long ID_BUTTON17;
        static const long ID_BUTTON18;
        static const long ID_BUTTON19;
        static const long ID_BUTTON20;
        static const long ID_BUTTON21;
        static const long ID_CHOICE2;
        static const long ID_CHOICE1;
        static const long ID_TEXTCTRL1;
        static const long ID_TEXTCTRL25;
        static const long ID_SPINCTRL9;
        static const long ID_STATICTEXT8;
        static const long ID_STATICTEXT9;
        static const long ID_BUTTON22;
        static const long ID_PANEL1;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM3;
        static const long idMenuQuit;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM5;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(fpsconfigFrame)
        wxButton* stickdown;
        wxTextCtrl* TextCtrlFilterADS;
        wxMenuItem* MenuItemSave;
        wxButton* l1;
        wxButton* r3;
        wxSpinCtrl* SpinCtrlAccelerationADS;
        wxMenuItem* MenuItemSaveAs;
        wxButton* stickright;
        wxStaticText* StaticTextADS;
        wxStaticText* StaticTextHipFire;
        wxButton* start;
        wxButton* stickleft;
        wxMenuItem* MenuEditLabels;
        wxButton* ButtonConvertSensitivity;
        wxSpinCtrl* SpinCtrlDeadZoneHipFire;
        wxButton* select;
        wxStaticText* StaticText8;
        wxButton* down;
        wxSpinCtrl* SpinCtrlBufferSizeADS;
        wxButton* stickup;
        wxPanel* Panel1;
        wxSpinCtrl* SpinCtrlFilterHipFire;
        wxTextCtrl* TextCtrlSensitivityHipFire;
        wxTextCtrl* TextCtrlFilterHipFire;
        wxFileDialog* FileDialog1;
        wxSpinCtrl* SpinCtrlSensitivityHipFire;
        wxStaticText* StaticTextSmoothing;
        wxMenuItem* MenuItemOpen;
        wxButton* r2;
        wxButton* l2;
        wxStaticText* StaticTextSens;
        wxStaticText* StaticTextDZ;
        wxSpinCtrl* SpinCtrlFilterADS;
        wxTextCtrl* TextCtrlAccelerationHipFire;
        wxSpinCtrl* SpinCtrlDeadZoneADS;
        wxTextCtrl* TextCtrlSensitivityADS;
        wxStatusBar* StatusBar1;
        wxTextCtrl* TextCtrlAccelerationADS;
        wxButton* left;
        wxButton* triangle;
        wxMenuItem* MenuItemNew;
        wxSpinCtrl* SpinCtrlDPI;
        wxButton* cross;
        wxButton* square;
        wxChoice* ChoiceDeadZoneShapeADS;
        wxSpinCtrl* SpinCtrlXyRatioHipFire;
        wxChoice* ChoiceDeadZoneShapeHipFire;
        wxSpinCtrl* SpinCtrlBufferSizeHipFire;
        wxTextCtrl* TextCtrlXyRatioADS;
        wxButton* ps;
        wxSpinCtrl* SpinCtrlSensitivityADS;
        wxSpinCtrl* SpinCtrlAccelerationHipFire;
        wxButton* l3;
        wxButton* up;
        wxButton* r1;
        wxStaticText* StaticTextXy;
        wxTextCtrl* TextCtrlXyRatioHipFire;
        wxStaticText* StaticTextAccel;
        wxSpinCtrl* SpinCtrlXyRatioADS;
        wxMenuItem* MenuUpdate;
        wxButton* right;
        wxStaticText* StaticTextShape;
        wxButton* circle;
        wxMenuItem* MenuAutoBindControls;
        //*)

        event_catcher evcatch;
        ConfigurationFile configFile;

        ButtonMapper buttons[BI_MAX];
        AxisMapper axes[AI_MAX];

        unsigned int current_dpi;

        double values[8];
		
        string defaultMouseName;
        string defaultKeyboardName;

        wxString default_directory;

        wxArrayString b_labels;
        wxArrayString a_labels;

        DECLARE_EVENT_TABLE()
};

#endif // FPSCONFIGMAIN_H
