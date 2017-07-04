/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FPSCONFIGMAIN_H
#define FPSCONFIGMAIN_H

//(*Headers(fpsconfigFrame)
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

#include <gimxconfigeditor/include/EventCatcher.h>
#include <gimxconfigeditor/include/ConfigurationFile.h>

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
    class wxBackgroundBitmap : public wxEvtHandler {
        typedef wxEvtHandler Inherited;
    public:
        wxBackgroundBitmap(const wxBitmap &B) : wxEvtHandler(), Bitmap(B) { }
        bool ProcessEvent(wxEvent &Event)
        {
            if (Event.GetEventType() == wxEVT_ERASE_BACKGROUND) {
                wxEraseEvent &EraseEvent = dynamic_cast<wxEraseEvent &>(Event);
                wxDC *DC = EraseEvent.GetDC();
                DC->DrawBitmap(Bitmap, 0, 0, false);
                return true;
            } else return Inherited::ProcessEvent(Event);
        }
    protected:
        wxBitmap            Bitmap;
    };

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
        void OnMenuEditLabels(wxCommandEvent& event);
        void OnButtonConvertSensitivityClick(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        void OnMenuOpenConfigDirectory(wxCommandEvent& event);
        void OnMenuItemWindowEventsSelected(wxCommandEvent& event);
        //*)

        e_button_index getButtonIndex(wxButton* button);
        e_axis_index getAxisIndex(wxButton* button);
        wxButton* getButtonButton(string blabel);
        wxButton* getAxisButton(string blabel);

        void LoadConfig();
        void readLabels();

        string reverseTranslate(string str);

        pair<Device, Event> selectEvent();

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
        static const long ID_MENUITEM8;
        static const long idMenuQuit;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM9;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(fpsconfigFrame)
        wxButton* down;
        wxSpinCtrl* SpinCtrlFilterHipFire;
        wxButton* left;
        wxMenuItem* MenuItemSaveAs;
        wxStaticText* StaticTextAccel;
        wxSpinCtrl* SpinCtrlSensitivityHipFire;
        wxButton* ps;
        wxPanel* Panel1;
        wxButton* circle;
        wxTextCtrl* TextCtrlSensitivityADS;
        wxMenuItem* MenuEditLabels;
        wxStatusBar* StatusBar1;
        wxButton* start;
        wxMenuItem* MenuAutoBindControls;
        wxTextCtrl* TextCtrlXyRatioADS;
        wxButton* right;
        wxChoice* ChoiceDeadZoneShapeHipFire;
        wxChoice* ChoiceDeadZoneShapeADS;
        wxMenuItem* MenuItemWindowEvents;
        wxSpinCtrl* SpinCtrlBufferSizeADS;
        wxStaticText* StaticTextShape;
        wxMenuItem* MenuItemSave;
        wxButton* r1;
        wxSpinCtrl* SpinCtrlBufferSizeHipFire;
        wxSpinCtrl* SpinCtrlXyRatioADS;
        wxStaticText* StaticTextSmoothing;
        wxButton* square;
        wxSpinCtrl* SpinCtrlDeadZoneHipFire;
        wxButton* r2;
        wxButton* l2;
        wxButton* r3;
        wxStaticText* StaticTextHipFire;
        wxButton* l1;
        wxSpinCtrl* SpinCtrlAccelerationHipFire;
        wxButton* ButtonConvertSensitivity;
        wxTextCtrl* TextCtrlFilterHipFire;
        wxMenuItem* MenuItemNew;
        wxStaticText* StaticText8;
        wxSpinCtrl* SpinCtrlSensitivityADS;
        wxFileDialog* FileDialog1;
        wxStaticText* StaticTextDZ;
        wxButton* l3;
        wxSpinCtrl* SpinCtrlXyRatioHipFire;
        wxTextCtrl* TextCtrlXyRatioHipFire;
        wxSpinCtrl* SpinCtrlDeadZoneADS;
        wxButton* up;
        wxButton* stickright;
        wxSpinCtrl* SpinCtrlAccelerationADS;
        wxStaticText* StaticTextSens;
        wxButton* stickup;
        wxTextCtrl* TextCtrlSensitivityHipFire;
        wxButton* select;
        wxButton* cross;
        wxStaticText* StaticTextADS;
        wxTextCtrl* TextCtrlAccelerationADS;
        wxSpinCtrl* SpinCtrlDPI;
        wxButton* stickdown;
        wxButton* stickleft;
        wxMenuItem* MenuItemOpen;
        wxSpinCtrl* SpinCtrlFilterADS;
        wxButton* triangle;
        wxTextCtrl* TextCtrlFilterADS;
        wxTextCtrl* TextCtrlAccelerationHipFire;
        wxMenuItem* MenuItemOpenConfigDirectory;
        wxStaticText* StaticTextXy;
        //*)

        wxBackgroundBitmap* ToolBarBackground;

        wxLocale* locale;

        EventCatcher* evcatch;
        ConfigurationFile configFile;

        ControlMapper buttons[BI_MAX];
        ControlMapper old_buttons[BI_MAX];
        ControlMapper axes[AI_MAX];
        ControlMapper old_axes[AI_MAX];

        unsigned int current_dpi;

        double values[8];
		
        string defaultMouseName;
        string defaultKeyboardName;
        string defaultMouseId;
        string defaultKeyboardId;

        wxString default_directory;

        wxArrayString b_labels;
        wxArrayString a_labels;

        DECLARE_EVENT_TABLE()
};

#endif // FPSCONFIGMAIN_H
