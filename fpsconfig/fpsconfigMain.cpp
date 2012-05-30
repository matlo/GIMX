/***************************************************************
 * Name:      fpsconfigMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-08-06
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "fpsconfigMain.h"
#include <wx/msgdlg.h>
#include <ConfigurationFile.h>

#ifndef WIN32
#include <pwd.h>
#endif

//(*InternalHeaders(fpsconfigFrame)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "background.png.cpp"
#include <wx/mstream.h>

#include <wx/aboutdlg.h>
#include "fpsconfig.h"

#include <locale.h>
#include <wx/filename.h>

#include "../shared/updater/updater.h"
#include "../directories.h"

#include <wx/tooltip.h>
#include "wx/numdlg.h"

#include <sstream>

using namespace std;

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

//(*IdInit(fpsconfigFrame)
const long fpsconfigFrame::ID_SPINCTRL8 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL7 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL6 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL5 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL4 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL10 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL11 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL3 = wxNewId();
const long fpsconfigFrame::ID_BUTTON10 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL13 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL14 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL1 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL2 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL24 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL4 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL26 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL2 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL3 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL22 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT1 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT7 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT2 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT3 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT4 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT5 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT6 = wxNewId();
const long fpsconfigFrame::ID_BUTTON1 = wxNewId();
const long fpsconfigFrame::ID_BUTTON2 = wxNewId();
const long fpsconfigFrame::ID_BUTTON3 = wxNewId();
const long fpsconfigFrame::ID_BUTTON4 = wxNewId();
const long fpsconfigFrame::ID_BUTTON5 = wxNewId();
const long fpsconfigFrame::ID_BUTTON6 = wxNewId();
const long fpsconfigFrame::ID_BUTTON7 = wxNewId();
const long fpsconfigFrame::ID_BUTTON8 = wxNewId();
const long fpsconfigFrame::ID_BUTTON9 = wxNewId();
const long fpsconfigFrame::ID_BUTTON11 = wxNewId();
const long fpsconfigFrame::ID_BUTTON12 = wxNewId();
const long fpsconfigFrame::ID_BUTTON13 = wxNewId();
const long fpsconfigFrame::ID_BUTTON14 = wxNewId();
const long fpsconfigFrame::ID_BUTTON15 = wxNewId();
const long fpsconfigFrame::ID_BUTTON16 = wxNewId();
const long fpsconfigFrame::ID_BUTTON17 = wxNewId();
const long fpsconfigFrame::ID_BUTTON18 = wxNewId();
const long fpsconfigFrame::ID_BUTTON19 = wxNewId();
const long fpsconfigFrame::ID_BUTTON20 = wxNewId();
const long fpsconfigFrame::ID_BUTTON21 = wxNewId();
const long fpsconfigFrame::ID_CHOICE2 = wxNewId();
const long fpsconfigFrame::ID_CHOICE1 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL1 = wxNewId();
const long fpsconfigFrame::ID_TEXTCTRL25 = wxNewId();
const long fpsconfigFrame::ID_SPINCTRL9 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT8 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT9 = wxNewId();
const long fpsconfigFrame::ID_BUTTON22 = wxNewId();
const long fpsconfigFrame::ID_PANEL1 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM1 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM4 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM2 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM3 = wxNewId();
const long fpsconfigFrame::idMenuQuit = wxNewId();
const long fpsconfigFrame::ID_MENUITEM6 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM7 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM5 = wxNewId();
const long fpsconfigFrame::idMenuAbout = wxNewId();
const long fpsconfigFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(fpsconfigFrame,wxFrame)
    //(*EventTable(fpsconfigFrame)
    //*)
END_EVENT_TABLE()

class wxBackgroundBitmap : public wxEvtHandler {
    typedef wxEvtHandler Inherited;
public:
    wxBackgroundBitmap(const wxBitmap &B) : wxEvtHandler(), Bitmap(B) { }
    virtual bool        ProcessEvent(wxEvent &Event);
protected:
    wxBitmap            Bitmap;
};

bool wxBackgroundBitmap::ProcessEvent(wxEvent &Event)
{
    if (Event.GetEventType() == wxEVT_ERASE_BACKGROUND) {
        wxEraseEvent &EraseEvent = dynamic_cast<wxEraseEvent &>(Event);
        wxDC *DC = EraseEvent.GetDC();
        DC->DrawBitmap(Bitmap, 0, 0, false);
        return true;
    } else return Inherited::ProcessEvent(Event);
}

const char* button_labels[BI_MAX] =
{
    "undef",
    "select", "start", "PS",
    "up", "right", "down", "left",
    "triangle", "circle", "cross", "square",
    "l1", "r1",
    "l2", "r2",
    "l3", "r3"
};

const char* axis_labels[AI_MAX] =
{
    "undef",
    "lstick up",
    "lstick right",
    "lstick down",
    "lstick left"
};

fpsconfigFrame::fpsconfigFrame(wxString file,wxWindow* parent,wxWindowID id)
{
    unsigned int i;

    //(*Initialize(fpsconfigFrame)
    wxMenu* MenuHelp;
    wxMenuItem* MenuItemAbout;
    wxMenu* MenuAdvanced;
    wxMenuItem* MenuItemQuit;
    wxMenu* MenuFile;
    wxMenuBar* MenuBar1;
    
    Create(parent, wxID_ANY, _("Gimx-fpsconfig"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(614,423));
    SetBackgroundColour(wxColour(255,255,255));
    Panel1 = new wxPanel(this, ID_PANEL1, wxPoint(0,0), wxSize(613,366), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Panel1->SetBackgroundColour(wxColour(255,255,255));
    SpinCtrlXyRatioADS = new wxSpinCtrl(Panel1, ID_SPINCTRL8, _T("100"), wxPoint(486,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL8"));
    SpinCtrlXyRatioADS->SetValue(_T("100"));
    SpinCtrlXyRatioHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL7, _T("100"), wxPoint(486,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL7"));
    SpinCtrlXyRatioHipFire->SetValue(_T("100"));
    SpinCtrlAccelerationADS = new wxSpinCtrl(Panel1, ID_SPINCTRL6, _T("100"), wxPoint(418,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL6"));
    SpinCtrlAccelerationADS->SetValue(_T("100"));
    SpinCtrlAccelerationHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL5, _T("100"), wxPoint(418,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL5"));
    SpinCtrlAccelerationHipFire->SetValue(_T("100"));
    SpinCtrlSensitivityADS = new wxSpinCtrl(Panel1, ID_SPINCTRL4, _T("100"), wxPoint(360,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL4"));
    SpinCtrlSensitivityADS->SetValue(_T("100"));
    SpinCtrlFilterHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL10, _T("0"), wxPoint(588,320), wxSize(19,-1), 0, -10000, 10000, 0, _T("ID_SPINCTRL10"));
    SpinCtrlFilterHipFire->SetValue(_T("0"));
    SpinCtrlFilterADS = new wxSpinCtrl(Panel1, ID_SPINCTRL11, _T("0"), wxPoint(588,352), wxSize(19,-1), 0, -10000, 10000, 0, _T("ID_SPINCTRL11"));
    SpinCtrlFilterADS->SetValue(_T("0"));
    SpinCtrlSensitivityHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL3, _T("100"), wxPoint(360,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL3"));
    SpinCtrlSensitivityHipFire->SetValue(_T("100"));
    stickright = new wxButton(Panel1, ID_BUTTON10, wxEmptyString, wxPoint(256,222), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON10"));
    wxFont stickrightFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    stickright->SetFont(stickrightFont);
    SpinCtrlBufferSizeHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL13, _T("1"), wxPoint(506,320), wxSize(44,-1), 0, 1, 30, 1, _T("ID_SPINCTRL13"));
    SpinCtrlBufferSizeHipFire->SetValue(_T("1"));
    SpinCtrlBufferSizeHipFire->SetToolTip(_("Buffer size (Hip Fire)"));
    SpinCtrlBufferSizeADS = new wxSpinCtrl(Panel1, ID_SPINCTRL14, _T("1"), wxPoint(506,352), wxSize(44,-1), 0, 1, 30, 1, _T("ID_SPINCTRL14"));
    SpinCtrlBufferSizeADS->SetValue(_T("1"));
    SpinCtrlBufferSizeADS->SetToolTip(_("Buffer size (ADS)"));
    SpinCtrlDeadZoneHipFire = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("20"), wxPoint(184,320), wxSize(44,-1), 0, 0, 64, 20, _T("ID_SPINCTRL1"));
    SpinCtrlDeadZoneHipFire->SetValue(_T("20"));
    SpinCtrlDeadZoneHipFire->SetToolTip(_("Dead zone (Hip Fire)"));
    SpinCtrlDeadZoneADS = new wxSpinCtrl(Panel1, ID_SPINCTRL2, _T("20"), wxPoint(184,352), wxSize(44,-1), 0, 0, 64, 20, _T("ID_SPINCTRL2"));
    SpinCtrlDeadZoneADS->SetValue(_T("20"));
    SpinCtrlDeadZoneADS->SetToolTip(_("Dead zone (ADS)"));
    TextCtrlSensitivityADS = new wxTextCtrl(Panel1, ID_TEXTCTRL24, _("1.00"), wxPoint(312,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL24"));
    TextCtrlSensitivityADS->SetToolTip(_("Sensitivity (ADS)"));
    TextCtrlSensitivityHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL4, _("1.00"), wxPoint(312,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    TextCtrlSensitivityHipFire->SetToolTip(_("Sensitivity (Hip Fire)"));
    TextCtrlAccelerationADS = new wxTextCtrl(Panel1, ID_TEXTCTRL26, _("1.00"), wxPoint(380,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL26"));
    TextCtrlAccelerationADS->SetToolTip(_("Acceleration (ADS)"));
    TextCtrlFilterHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL2, _("0.00"), wxPoint(550,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrlFilterHipFire->SetToolTip(_("Filter (Hip Fire)"));
    TextCtrlFilterADS = new wxTextCtrl(Panel1, ID_TEXTCTRL3, _("0.00"), wxPoint(550,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    TextCtrlFilterADS->SetToolTip(_("Filter (ADS)"));
    TextCtrlAccelerationHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL22, _("1.00"), wxPoint(380,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL22"));
    TextCtrlAccelerationHipFire->SetToolTip(_("Acceleration (Hip Fire)"));
    StaticTextHipFire = new wxStaticText(Panel1, ID_STATICTEXT1, _("Hip Fire"), wxPoint(120,328), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    StaticTextShape = new wxStaticText(Panel1, ID_STATICTEXT7, _("Shape"), wxPoint(246,296), wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    StaticTextShape->SetToolTip(_("Dead zone shape"));
    StaticTextADS = new wxStaticText(Panel1, ID_STATICTEXT2, _("ADS"), wxPoint(144,360), wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    StaticTextADS->SetToolTip(_("Aiming Down the Sights"));
    StaticTextDZ = new wxStaticText(Panel1, ID_STATICTEXT3, _("DZ"), wxPoint(192,296), wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    StaticTextDZ->SetToolTip(_("Dead zone"));
    StaticTextSens = new wxStaticText(Panel1, ID_STATICTEXT4, _("Sens."), wxPoint(320,296), wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    StaticTextSens->SetToolTip(_("Sensitivity"));
    StaticTextAccel = new wxStaticText(Panel1, ID_STATICTEXT5, _("Accel."), wxPoint(382,296), wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    StaticTextAccel->SetToolTip(_("Acceleration"));
    StaticTextXy = new wxStaticText(Panel1, ID_STATICTEXT6, _("x/y"), wxPoint(452,296), wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    StaticTextXy->SetToolTip(_("x/y ratio"));
    l2 = new wxButton(Panel1, ID_BUTTON1, wxEmptyString, wxPoint(136,0), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
    wxFont l2Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    l2->SetFont(l2Font);
    l1 = new wxButton(Panel1, ID_BUTTON2, wxEmptyString, wxPoint(136,32), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
    wxFont l1Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    l1->SetFont(l1Font);
    up = new wxButton(Panel1, ID_BUTTON3, wxEmptyString, wxPoint(104,96), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    wxFont upFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    up->SetFont(upFont);
    left = new wxButton(Panel1, ID_BUTTON4, wxEmptyString, wxPoint(80,128), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON4"));
    wxFont leftFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    left->SetFont(leftFont);
    right = new wxButton(Panel1, ID_BUTTON5, wxEmptyString, wxPoint(128,128), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON5"));
    wxFont rightFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    right->SetFont(rightFont);
    down = new wxButton(Panel1, ID_BUTTON6, wxEmptyString, wxPoint(104,160), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON6"));
    wxFont downFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    down->SetFont(downFont);
    l3 = new wxButton(Panel1, ID_BUTTON7, wxEmptyString, wxPoint(194,222), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON7"));
    wxFont l3Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    l3->SetFont(l3Font);
    stickup = new wxButton(Panel1, ID_BUTTON8, wxEmptyString, wxPoint(194,176), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON8"));
    wxFont stickupFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    stickup->SetFont(stickupFont);
    stickleft = new wxButton(Panel1, ID_BUTTON9, wxEmptyString, wxPoint(130,222), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON9"));
    wxFont stickleftFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    stickleft->SetFont(stickleftFont);
    stickdown = new wxButton(Panel1, ID_BUTTON11, wxEmptyString, wxPoint(194,264), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON11"));
    wxFont stickdownFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    stickdown->SetFont(stickdownFont);
    select = new wxButton(Panel1, ID_BUTTON12, wxEmptyString, wxPoint(256,144), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON12"));
    wxFont selectFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    select->SetFont(selectFont);
    start = new wxButton(Panel1, ID_BUTTON13, wxEmptyString, wxPoint(312,144), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON13"));
    wxFont startFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    start->SetFont(startFont);
    ps = new wxButton(Panel1, ID_BUTTON14, wxEmptyString, wxPoint(284,176), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON14"));
    wxFont psFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    ps->SetFont(psFont);
    r2 = new wxButton(Panel1, ID_BUTTON15, wxEmptyString, wxPoint(424,0), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON15"));
    wxFont r2Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    r2->SetFont(r2Font);
    r1 = new wxButton(Panel1, ID_BUTTON16, wxEmptyString, wxPoint(424,32), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON16"));
    wxFont r1Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    r1->SetFont(r1Font);
    triangle = new wxButton(Panel1, ID_BUTTON17, wxEmptyString, wxPoint(488,56), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON17"));
    wxFont triangleFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    triangle->SetFont(triangleFont);
    square = new wxButton(Panel1, ID_BUTTON18, wxEmptyString, wxPoint(384,104), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON18"));
    wxFont squareFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    square->SetFont(squareFont);
    circle = new wxButton(Panel1, ID_BUTTON19, wxEmptyString, wxPoint(544,112), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON19"));
    wxFont circleFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    circle->SetFont(circleFont);
    cross = new wxButton(Panel1, ID_BUTTON20, wxEmptyString, wxPoint(496,192), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON20"));
    wxFont crossFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    cross->SetFont(crossFont);
    r3 = new wxButton(Panel1, ID_BUTTON21, wxEmptyString, wxPoint(368,222), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON21"));
    wxFont r3Font(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    r3->SetFont(r3Font);
    ChoiceDeadZoneShapeHipFire = new wxChoice(Panel1, ID_CHOICE2, wxPoint(230,320), wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    ChoiceDeadZoneShapeHipFire->SetSelection( ChoiceDeadZoneShapeHipFire->Append(_("Circle")) );
    ChoiceDeadZoneShapeHipFire->Append(_("Rectangle"));
    ChoiceDeadZoneShapeHipFire->SetToolTip(_("Dead zone shape (Hip Fire)"));
    ChoiceDeadZoneShapeADS = new wxChoice(Panel1, ID_CHOICE1, wxPoint(230,352), wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    ChoiceDeadZoneShapeADS->SetSelection( ChoiceDeadZoneShapeADS->Append(_("Circle")) );
    ChoiceDeadZoneShapeADS->Append(_("Rectangle"));
    ChoiceDeadZoneShapeADS->SetToolTip(_("Dead zone shape (ADS)"));
    TextCtrlXyRatioHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL1, _("1.00"), wxPoint(438,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrlXyRatioHipFire->SetToolTip(_("x/y ratio (Hip Fire)"));
    TextCtrlXyRatioADS = new wxTextCtrl(Panel1, ID_TEXTCTRL25, _("1.00"), wxPoint(438,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL25"));
    TextCtrlXyRatioADS->SetToolTip(_("x/y ratio (ADS)"));
    SpinCtrlDPI = new wxSpinCtrl(Panel1, ID_SPINCTRL9, _T("0"), wxPoint(24,256), wxSize(64,-1), 0, 0, 20000, 0, _T("ID_SPINCTRL9"));
    SpinCtrlDPI->SetValue(_T("0"));
    SpinCtrlDPI->SetToolTip(_("Enter your mouse DPI value."));
    StaticText8 = new wxStaticText(Panel1, ID_STATICTEXT8, _("Mouse DPI"), wxPoint(24,240), wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    StaticTextSmoothing = new wxStaticText(Panel1, ID_STATICTEXT9, _("Smoothing"), wxPoint(514,296), wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    StaticTextSmoothing->SetToolTip(_("Mouse smoothing"));
    ButtonConvertSensitivity = new wxButton(Panel1, ID_BUTTON22, _("Convert\nsensitivity"), wxPoint(24,288), wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON22"));
    MenuBar1 = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItemNew = new wxMenuItem(MenuFile, ID_MENUITEM1, _("New\tCtrl+N"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemNew);
    MenuItemOpen = new wxMenuItem(MenuFile, ID_MENUITEM4, _("Open\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemOpen);
    MenuItemSave = new wxMenuItem(MenuFile, ID_MENUITEM2, _("Save\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemSave);
    MenuItemSave->Enable(false);
    MenuItemSaveAs = new wxMenuItem(MenuFile, ID_MENUITEM3, _("Save As"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemSaveAs);
    MenuItemQuit = new wxMenuItem(MenuFile, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemQuit);
    MenuBar1->Append(MenuFile, _("&File"));
    MenuAdvanced = new wxMenu();
    MenuEditLabels = new wxMenuItem(MenuAdvanced, ID_MENUITEM6, _("Edit Labels"), wxEmptyString, wxITEM_CHECK);
    MenuAdvanced->Append(MenuEditLabels);
    MenuAutoBindControls = new wxMenuItem(MenuAdvanced, ID_MENUITEM7, _("Auto-bind controls"), wxEmptyString, wxITEM_NORMAL);
    MenuAdvanced->Append(MenuAutoBindControls);
    MenuBar1->Append(MenuAdvanced, _("Advanced"));
    MenuHelp = new wxMenu();
    MenuUpdate = new wxMenuItem(MenuHelp, ID_MENUITEM5, _("Update"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuUpdate);
    MenuItemAbout = new wxMenuItem(MenuHelp, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItemAbout);
    MenuBar1->Append(MenuHelp, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    
    Connect(ID_SPINCTRL8,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL7,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL6,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL5,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL10,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL11,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnSpinCtrlChange);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_TEXTCTRL24,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL24,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL26,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL26,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL22,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL22,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON19,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON20,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_BUTTON21,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonClick);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL25,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL25,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&fpsconfigFrame::OnTextCtrlText);
    Connect(ID_SPINCTRL9,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&fpsconfigFrame::OnMouseDPIChange);
    Connect(ID_BUTTON22,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&fpsconfigFrame::OnButtonConvertSensitivityClick);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuNew);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuOpen);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuSave);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuSaveAs);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnQuit);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuAutoBindControls);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuUpdate);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnAbout);
    //*)

    textDialog = new wxTextEntryDialog(this, _("Edit Label"));

    wxMemoryInputStream istream(background_png, sizeof background_png);
    wxImage background_img(istream, wxBITMAP_TYPE_PNG);
    wxBackgroundBitmap* ToolBarBackground = new wxBackgroundBitmap(wxBitmap(background_img));
    Panel1->PushEventHandler(ToolBarBackground);

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

    default_directory = wxEmptyString;

#ifndef WIN32
    if(!getuid())
    {
      int answer = wxMessageBox(_("It's not recommended to run as root user. Continue?"), _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        exit(0);
      }
    }

    default_directory.Append(wxFileName::GetHomeDir());
    default_directory.Append(_("/.emuclient/"));

	  /* Init user's config directory. */
    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( wxT("Can't init ~/.emuclient/config!"), wxT("Error"), wxICON_ERROR);
    }
#endif

    default_directory.Append(_(CONFIG_DIR));

    FileDialog1->SetDirectory(default_directory);

    current_dpi = 0;

    for(i=0; i<sizeof(values)/sizeof(values[0]); ++i)
    {
      values[i] = 1;
    }

  	/* Open the file given as argument. */
    if(!file.IsEmpty())
    {
      wxString wxfile = default_directory + file;

      if(::wxFileExists(wxfile))
      {
        configFile.ReadConfigFile(string(wxfile.mb_str()));
        LoadConfig();
        FileDialog1->SetFilename(file);
      }
      else
      {
        wxMessageBox( wxT("Cannot open config file: ") + wxString(file, wxConvUTF8), wxT("Error"), wxICON_ERROR);
      }
    }

	  wxToolTip::SetDelay(0);
}

fpsconfigFrame::~fpsconfigFrame()
{
    //(*Destroy(fpsconfigFrame)
    //*)
}

void fpsconfigFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void fpsconfigFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(_(INFO_DESCR)) + wxString(_("\n")) + wxString(_(INFO_YEAR)) + wxString(_(" ")) + wxString(_(INFO_DEV)) + wxString(_(" ")) + wxString(_(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void updateTextCtrlValue(wxTextCtrl* text, wxSpinEvent& event, double* value)
{
    wxString str;
    *value = (double)(event.GetInt()) / 100;
    str = wxString::Format(wxT("%.02f"), *value);
    text->SetValue(str);
}

void fpsconfigFrame::OnSpinCtrlChange(wxSpinEvent& event)
{
    wxSpinCtrl* spin = (wxSpinCtrl*)event.GetEventObject();

    if(spin == SpinCtrlSensitivityHipFire)
    {
        updateTextCtrlValue(TextCtrlSensitivityHipFire, event, values);
    }
    else if(spin == SpinCtrlSensitivityADS)
    {
        updateTextCtrlValue(TextCtrlSensitivityADS, event, values+1);
    }
    else if(spin == SpinCtrlAccelerationHipFire)
    {
        updateTextCtrlValue(TextCtrlAccelerationHipFire, event, values+2);
    }
    else if(spin == SpinCtrlAccelerationADS)
    {
        updateTextCtrlValue(TextCtrlAccelerationADS, event, values+3);
    }
    else if(spin == SpinCtrlXyRatioHipFire)
    {
        updateTextCtrlValue(TextCtrlXyRatioHipFire, event, values+4);
    }
    else if(spin == SpinCtrlXyRatioADS)
    {
        updateTextCtrlValue(TextCtrlXyRatioADS, event, values+5);
    }
    else if(spin == SpinCtrlFilterHipFire)
    {
        updateTextCtrlValue(TextCtrlFilterHipFire, event, values+6);
    }
    else if(spin == SpinCtrlFilterADS)
    {
        updateTextCtrlValue(TextCtrlFilterADS, event, values+7);
    }
}

e_button_index fpsconfigFrame::getButtonIndex(wxButton* button)
{
    if(button == select)
    {
        return bi_select;
    }
    else if(button == start)
    {
        return bi_start;
    }
    else if(button == ps)
    {
        return bi_ps;
    }
    else if(button == up)
    {
        return bi_up;
    }
    else if(button == down)
    {
        return bi_down;
    }
    else if(button == right)
    {
        return bi_right;
    }
    else if(button == left)
    {
        return bi_left;
    }
    else if(button == square)
    {
        return bi_square;
    }
    else if(button == cross)
    {
        return bi_cross;
    }
    else if(button == circle)
    {
        return bi_circle;
    }
    else if(button == triangle)
    {
        return bi_triangle;
    }
    else if(button == l1)
    {
        return bi_l1;
    }
    else if(button == l2)
    {
        return bi_l2;
    }
    else if(button == l3)
    {
        return bi_l3;
    }
    else if(button == r1)
    {
        return bi_r1;
    }
    else if(button == r2)
    {
        return bi_r2;
    }
    else if(button == r3)
    {
        return bi_r3;
    }
    return bi_undef;//not supposed to occur...
}

wxButton* fpsconfigFrame::getButtonButton(string blabel)
{
    if(blabel == button_labels[bi_select])
    {
        return select;
    }
    else if(blabel == button_labels[bi_start])
    {
        return start;
    }
    else if(blabel == button_labels[bi_ps])
    {
        return ps;
    }
    else if(blabel == button_labels[bi_up])
    {
        return up;
    }
    else if(blabel == button_labels[bi_down])
    {
        return down;
    }
    else if(blabel == button_labels[bi_right])
    {
        return right;
    }
    else if(blabel == button_labels[bi_left])
    {
        return left;
    }
    else if(blabel == button_labels[bi_square])
    {
        return square;
    }
    else if(blabel == button_labels[bi_cross])
    {
        return cross;
    }
    else if(blabel == button_labels[bi_circle])
    {
        return circle;
    }
    else if(blabel == button_labels[bi_triangle])
    {
        return triangle;
    }
    else if(blabel == button_labels[bi_l1])
    {
        return l1;
    }
    else if(blabel == button_labels[bi_l2])
    {
        return l2;
    }
    else if(blabel == button_labels[bi_l3])
    {
        return l3;
    }
    else if(blabel == button_labels[bi_r1])
    {
        return r1;
    }
    else if(blabel == button_labels[bi_r2])
    {
        return r2;
    }
    else if(blabel == button_labels[bi_r3])
    {
        return r3;
    }
    return NULL;//not supposed to occur...
}

wxButton* fpsconfigFrame::getAxisButton(string alabel)
{
    if(alabel == axis_labels[ai_ls_down])
    {
        return stickdown;
    }
    else if(alabel == axis_labels[ai_ls_left])
    {
        return stickleft;
    }
    else if(alabel == axis_labels[ai_ls_right])
    {
        return stickright;
    }
    else if(alabel == axis_labels[ai_ls_up])
    {
        return stickup;
    }
    return NULL;//not supposed to occur...
}

e_axis_index fpsconfigFrame::getAxisIndex(wxButton* button)
{
    if(button == stickdown)
    {
        return ai_ls_down;
    }
    else if(button == stickleft)
    {
        return ai_ls_left;
    }
    else if(button == stickright)
    {
        return ai_ls_right;
    }
    else if(button == stickup)
    {
        return ai_ls_up;
    }
    return ai_undef;//not supposed to occur...
}

void fpsconfigFrame::OnButtonClick(wxCommandEvent& event)
{
  e_button_index bindex = bi_undef;
  e_axis_index aindex = ai_undef;

  ((wxButton*) event.GetEventObject())->Enable(false);

  if (MenuEditLabels->IsChecked())
  {
    bindex = getButtonIndex((wxButton*) event.GetEventObject());

    if (bindex != bi_undef)
    {
      textDialog->SetValue(wxString(buttons[bindex].GetLabel().c_str(), wxConvUTF8));
    }
    else
    {
      aindex = getAxisIndex((wxButton*) event.GetEventObject());

      if (aindex != ai_undef)
      {
        textDialog->SetValue(wxString(axes[aindex].GetLabel().c_str(), wxConvUTF8));
      }
    }

    if (textDialog->ShowModal() == wxID_OK)
    {
      if (bindex != bi_undef)
      {
        buttons[bindex].SetLabel(string(textDialog->GetValue().mb_str()));
        string tt(buttons[bindex].GetEvent()->GetId());
        if (!buttons[bindex].GetLabel().empty())
        {
          tt.append(" [");
          tt.append(buttons[bindex].GetLabel());
          tt.append("]");
        }
        ((wxButton*) event.GetEventObject())->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
      }
      else if (aindex != ai_undef)
      {
        axes[aindex].SetLabel(string(textDialog->GetValue().mb_str()));
        string tt(axes[aindex].GetEvent()->GetId());
        if (!axes[aindex].GetLabel().empty())
        {
          tt.append(" [");
          tt.append(axes[aindex].GetLabel());
          tt.append("]");
        }
        ((wxButton*) event.GetEventObject())->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
      }
    }
  }
  else
  {
    StatusBar1->SetStatusText(_("Press a button."));
    evcatch.run("", "button");
    StatusBar1->SetStatusText(wxEmptyString);
    ((wxButton*) event.GetEventObject())->SetLabel(wxString(evcatch.GetEventId().c_str(), wxConvUTF8));

    bindex = getButtonIndex((wxButton*) event.GetEventObject());

    if (bindex != bi_undef)
    {
      buttons[bindex].SetDevice(Device(evcatch.GetDeviceType(), "0", ""));
      buttons[bindex].SetEvent(Event("button", evcatch.GetEventId()));
      buttons[bindex].SetButton(button_labels[bindex]);
      string tt(buttons[bindex].GetEvent()->GetId());
      if (!buttons[bindex].GetLabel().empty())
      {
        tt.append(" [");
        tt.append(buttons[bindex].GetLabel());
        tt.append("]");
      }
      ((wxButton*) event.GetEventObject())->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
    }
    else
    {
      aindex = getAxisIndex((wxButton*) event.GetEventObject());

      if (aindex != ai_undef)
      {
        axes[aindex].SetDevice(Device(evcatch.GetDeviceType(), "0", ""));
        axes[aindex].SetEvent(Event("button", evcatch.GetEventId()));
        axes[aindex].SetAxis(axis_labels[aindex]);
        string tt(axes[aindex].GetEvent()->GetId());
        if (!axes[aindex].GetLabel().empty())
        {
          tt.append(" [");
          tt.append(axes[aindex].GetLabel());
          tt.append("]");
        }
        ((wxButton*) event.GetEventObject())->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
      }
    }
  }

  ((wxButton*) event.GetEventObject())->Enable(true);
}

void fpsconfigFrame::OnMenuNew(wxCommandEvent& event)
{
    FileDialog1->SetFilename(wxEmptyString);

    wxButton* button;

    for(int i=bi_select; i<BI_MAX; i++)
    {
        buttons[i] = ButtonMapper();
        button = getButtonButton(button_labels[i]);
        if(button != NULL)
        {
            button->SetLabel(wxEmptyString);
            button->UnsetToolTip();
        }
    }

    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        axes[i] = AxisMapper();
        button = getAxisButton(axis_labels[i]);
        if(button != NULL)
        {
            button->SetLabel(wxEmptyString);
            button->UnsetToolTip();
        }
    }

    SpinCtrlDeadZoneHipFire->SetValue(20);
    SpinCtrlDeadZoneADS->SetValue(20);
    ChoiceDeadZoneShapeADS->SetSelection(0);
    ChoiceDeadZoneShapeHipFire->SetSelection(0);
    TextCtrlSensitivityHipFire->SetValue(_("1.00"));
    TextCtrlSensitivityADS->SetValue(_("1.00"));
    TextCtrlAccelerationHipFire->SetValue(_("1.00"));
    TextCtrlAccelerationADS->SetValue(_("1.00"));
    TextCtrlXyRatioHipFire->SetValue(_("1.00"));
    TextCtrlXyRatioADS->SetValue(_("1.00"));

    SpinCtrlDPI->Enable(true);
    SpinCtrlDPI->SetToolTip(_("Enter your mouse DPI value."));

    configFile = ConfigurationFile();

    MenuItemSave->Enable(false);
}

void fpsconfigFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _T("Save Config file"), _T(""), _T(""), _T("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);
    saveFileDialog.SetFilename(FileDialog1->GetFilename());

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(string(FileName.mb_str()));

    OnMenuSave(event);
}

void fpsconfigFrame::OnMenuSave(wxCommandEvent& event)
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    double mx, my;
    double xyratio;
    wxString wsmx, wsmy, wsxyratio;
    bool found;

    string sDzHf, sBsHf, sDzADS, sBsADS;
    stringstream ss1, ss2, ss3, ss4;
    ss1 << SpinCtrlDeadZoneHipFire->GetValue();
    ss1 >> sDzHf;
    ss2 << SpinCtrlBufferSizeHipFire->GetValue();
    ss2 >> sBsHf;
    ss3 << SpinCtrlDeadZoneADS->GetValue();
    ss3 >> sDzADS;
    ss4 << SpinCtrlBufferSizeADS->GetValue();
    ss4 >> sBsADS;

    /*
     * Save DPI value.
     */
    configFile.GetController(0)->SetMouseDPI(current_dpi);
    /*
     * Save Hip Fire config.
     */
    //Save ButtonMappers
    buttonMappers = configFile.GetController(0)->GetConfiguration(0)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end() && !found; it++)
            {
                if(it->GetButton() == buttons[i].GetButton())
                {
                    it->SetEvent(*buttons[i].GetEvent());
                    it->SetLabel(buttons[i].GetLabel());
                    if(it->GetDevice()->GetType() != buttons[i].GetDevice()->GetType())
                    {
                        it->SetDevice(*buttons[i].GetDevice());
                        if(it->GetDevice()->GetType() == "mouse")
                        {
                          it->GetDevice()->SetName(defaultMouseName);
                        }
                        else if(it->GetDevice()->GetType() == "keyboard")
                        {
                          it->GetDevice()->SetName(defaultKeyboardName);
                        }
                    }
                    found = true;
                }
            }
            if(found == false)
            {
                buttonMappers->push_front(buttons[i]);
            }
        }
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(0)->GetConfiguration(0)->GetAxisMapperList();
    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        if(!axes[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
            {
                if(it->GetAxis() == axes[i].GetAxis())
                {
                  it->SetEvent(*axes[i].GetEvent());
                  it->SetLabel(axes[i].GetLabel());
                  if(it->GetDevice()->GetType() != axes[i].GetDevice()->GetType())
                  {
                    it->SetDevice(*axes[i].GetDevice());
                  }
                  found = true;
                }
            }
            if(found == false)
            {
                axisMappers->push_front(axes[i]);
            }
        }
    }
    found = false;
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "x" && it->GetAxis() == "rstick x")
        {
            it->GetEvent()->SetDeadZone(sDzHf);
            it->GetEvent()->SetMultiplier(string(TextCtrlSensitivityHipFire->GetValue().mb_str()));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationHipFire->GetValue().mb_str()));
            it->GetEvent()->SetShape(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str()));
            it->GetEvent()->SetBufferSize(sBsHf);
            it->GetEvent()->SetFilter(string(TextCtrlFilterHipFire->GetValue().mb_str()));
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper("mouse", "0", "", "axis", "x", "rstick x",
            sDzHf, string(TextCtrlSensitivityHipFire->GetValue().mb_str()),
            string(TextCtrlAccelerationHipFire->GetValue().mb_str()),
            string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str()),
            sBsHf, string(TextCtrlFilterHipFire->GetValue().mb_str()), "Aiming - x axis"));
    }
    wsmx = TextCtrlSensitivityHipFire->GetValue();
    wsxyratio = TextCtrlXyRatioHipFire->GetValue();
    if(wsmx.ToDouble(&mx) && wsxyratio.ToDouble(&xyratio))
    {
        my = mx * values[4];
    }
    else
    {
        my = mx;
    }
    wsmy = wxString::Format(wxT("%.02f"), my);
    found = false;
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "y" && it->GetAxis() == "rstick y")
        {
            it->GetEvent()->SetDeadZone(sDzHf);
            it->GetEvent()->SetMultiplier(string(wsmy.mb_str()));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationHipFire->GetValue().mb_str()));
            it->GetEvent()->SetShape(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str()));
            it->GetEvent()->SetBufferSize(sBsHf);
            it->GetEvent()->SetFilter(string(TextCtrlFilterHipFire->GetValue().mb_str()));
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper("mouse", "0", "", "axis", "y", "rstick y",
            sDzHf, string(wsmy.mb_str()), string(TextCtrlAccelerationHipFire->GetValue().mb_str()),
            string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str()),
            sBsHf, string(TextCtrlFilterHipFire->GetValue().mb_str()), "Aiming - y axis"));
    }
    /*
     * Save ADS config.
     */
    //Save Trigger
    if(configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->GetType() != "mouse"
        || configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetEvent()->GetId() != "BUTTON_RIGHT"
        || configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetSwitchBack() != "yes")
    {
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetType("mouse");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetName("");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetId("0");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetEvent()->SetId("BUTTON_RIGHT");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetSwitchBack("yes");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetDelay(0);
    }
    //Save ButtonMappers
    buttonMappers = configFile.GetController(0)->GetConfiguration(1)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end() && !found; it++)
            {
                if(it->GetButton() == buttons[i].GetButton())
                {
                    it->SetEvent(*buttons[i].GetEvent());
                    it->SetLabel(buttons[i].GetLabel());
                    if(it->GetDevice()->GetType() != buttons[i].GetDevice()->GetType())
                    {
                        it->SetDevice(*buttons[i].GetDevice());
                        if(it->GetDevice()->GetType() == "mouse")
                        {
                          it->GetDevice()->SetName(defaultMouseName);
                        }
                        else if(it->GetDevice()->GetType() == "keyboard")
                        {
                          it->GetDevice()->SetName(defaultKeyboardName);
                        }
                    }
                    found = true;
                }
            }
            if(found == false)
            {
                buttonMappers->push_front(buttons[i]);
            }
        }
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(0)->GetConfiguration(1)->GetAxisMapperList();
    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        if(!axes[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
            {
                if(it->GetAxis() == axes[i].GetAxis())
                {
                  it->SetEvent(*axes[i].GetEvent());
                  it->SetLabel(axes[i].GetLabel());
                  if(it->GetDevice()->GetType() != axes[i].GetDevice()->GetType())
                  {
                    it->SetDevice(*axes[i].GetDevice());
                  }
                  found = true;
                }
            }
            if(found == false)
            {
                axisMappers->push_front(axes[i]);
            }
        }
    }
    found = false;
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "x" && it->GetAxis() == "rstick x")
        {
            it->GetEvent()->SetDeadZone(sDzADS);
            it->GetEvent()->SetMultiplier(string(TextCtrlSensitivityADS->GetValue().mb_str()));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationADS->GetValue().mb_str()));
            it->GetEvent()->SetShape(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str()));
            it->GetEvent()->SetBufferSize(sBsADS);
            it->GetEvent()->SetFilter(string(TextCtrlFilterADS->GetValue().mb_str()));
            found = true;
        }
    }
    if(found == false)
    {
      axisMappers->push_front(AxisMapper("mouse", "0", "", "axis", "x", "rstick x",
          sDzADS, string(TextCtrlSensitivityADS->GetValue().mb_str()), string(TextCtrlAccelerationADS->GetValue().mb_str()),
          string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str()),
          sBsADS, string(TextCtrlFilterADS->GetValue().mb_str()), "Aiming - x axis"));
    }
    wsmx = TextCtrlSensitivityADS->GetValue();
    wsxyratio = TextCtrlXyRatioADS->GetValue();
    if(wsmx.ToDouble(&mx) && wsxyratio.ToDouble(&xyratio))
    {
        my = mx * values[5];
    }
    else
    {
        my = mx;
    }
    wsmy = wxString::Format(wxT("%.02f"), my);
    found = false;
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "y" && it->GetAxis() == "rstick y")
        {
            it->GetEvent()->SetDeadZone(sDzADS);
            it->GetEvent()->SetMultiplier(string(wsmy.mb_str()));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationADS->GetValue().mb_str()));
            it->GetEvent()->SetShape(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str()));
            it->GetEvent()->SetBufferSize(sBsADS);
            it->GetEvent()->SetFilter(string(TextCtrlFilterADS->GetValue().mb_str()));
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper("mouse", "0", "", "axis", "y", "rstick y",
            sDzADS, string(wsmy.mb_str()), string(TextCtrlAccelerationADS->GetValue().mb_str()),
            string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str()),
            sBsADS, string(TextCtrlFilterADS->GetValue().mb_str()), "Aiming - y axis"));
    }

    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(wxT("Can't save ") + wxString(configFile.GetFilePath().c_str(), wxConvUTF8), wxT("Error"), wxICON_ERROR);
    }
}

void fpsconfigFrame::LoadConfig()
{
  std::list<ButtonMapper>* buttonMappers;
  std::list<AxisMapper>* axisMappers;
  e_button_index bindex;
  e_axis_index aindex;
  wxButton* button;

  double mx, my, exp, f;
  double xyratio;
  wxString wsmx, wsmy, wsexp, wsxyratio, wsf;

  current_dpi = configFile.GetController(0)->GetMouseDPI();
  SpinCtrlDPI->SetValue(current_dpi);
  if(current_dpi)
  {
    SpinCtrlDPI->Enable(false);
    SpinCtrlDPI->UnsetToolTip();
  }
  else
  {
    SpinCtrlDPI->Enable(true);
    SpinCtrlDPI->SetToolTip(_("Enter your mouse DPI value."));
  }

  defaultMouseName = "";
  defaultKeyboardName = "";

  /*
   * Load Hip Fire config.
   */
  //Load ButtonMappers
  for(int i=bi_select; i<BI_MAX; i++)
  {
      buttons[i] = ButtonMapper();
      button = getButtonButton(button_labels[i]);
      if(button != NULL)
      {
          button->SetLabel(wxEmptyString);
          button->UnsetToolTip();
      }
  }
  buttonMappers = configFile.GetController(0)->GetConfiguration(0)->GetButtonMapperList();
  for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); it++)
  {
      button = getButtonButton(it->GetButton());

      if(button == NULL)
      {
          continue;
      }

      bindex = getButtonIndex(button);

      if(bindex == bi_undef)
      {
          continue;
      }

      if(!buttons[bindex].GetEvent()->GetId().empty())
      {
          continue;
      }

      buttons[bindex] = *it;
      button->SetLabel(wxString(it->GetEvent()->GetId().c_str(), wxConvUTF8));
      string tt(it->GetEvent()->GetId());
      if(!it->GetLabel().empty())
      {
          tt.append(" [");
          tt.append(it->GetLabel());
          tt.append("]");
      }
      button->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
      if(!it->GetDevice()->GetName().empty())
      {
          if(it->GetDevice()->GetType() == "mouse")
          {
              defaultMouseName = it->GetDevice()->GetName();
          }
          else if(it->GetDevice()->GetType() == "keyboard")
          {
              defaultKeyboardName = it->GetDevice()->GetName();
          }
      }
  }
  //Load AxisMappers
  for(int i=ai_ls_up; i<AI_MAX; i++)
  {
      axes[i] = AxisMapper();
      button = getAxisButton(axis_labels[i]);
      if(button != NULL)
      {
          button->SetLabel(wxEmptyString);
          button->UnsetToolTip();
      }
  }
  wsmx.erase();
  wsmy.erase();
  mx = 0;
  my = 0;
  axisMappers = configFile.GetController(0)->GetConfiguration(0)->GetAxisMapperList();
  for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
  {
      button = getAxisButton(it->GetAxis());

      if(button == NULL)
      {
          if(it->GetAxis() == "rstick x")
          {
              SpinCtrlDeadZoneHipFire->SetValue(wxAtoi(wxString(it->GetEvent()->GetDeadZone().c_str(), wxConvUTF8)));
              ChoiceDeadZoneShapeHipFire->SetStringSelection(wxString(it->GetEvent()->GetShape().c_str(), wxConvUTF8));

              SpinCtrlBufferSizeHipFire->SetValue(wxAtoi(wxString(it->GetEvent()->GetBufferSize().c_str(), wxConvUTF8)));

              wsf = wxString(it->GetEvent()->GetFilter().c_str(), wxConvUTF8);

              if(wsf.ToDouble(&f))
              {
                TextCtrlFilterHipFire->SetValue(wsf);
                SpinCtrlFilterHipFire->SetValue(f*100);
              }

              wsmx = wxString(it->GetEvent()->GetMultiplier().c_str(), wxConvUTF8);

              if(wsmx.ToDouble(&mx))
              {
                  TextCtrlSensitivityHipFire->SetValue(wsmx);
                  SpinCtrlSensitivityHipFire->SetValue(mx*100);
                  if(my && mx)
                  {
                      xyratio = my / mx;
                      wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                      TextCtrlXyRatioHipFire->SetValue(wsxyratio);
                      SpinCtrlXyRatioHipFire->SetValue(xyratio*100);
                  }
              }

              wsexp = wxString(it->GetEvent()->GetExponent().c_str(), wxConvUTF8);

              if(wsexp.ToDouble(&exp))
              {
                TextCtrlAccelerationHipFire->SetValue(wxString(it->GetEvent()->GetExponent().c_str(), wxConvUTF8));
                SpinCtrlAccelerationHipFire->SetValue(exp*100);
              }
          }
          else if(it->GetAxis() == "rstick y")
          {
              wsmy = wxString(it->GetEvent()->GetMultiplier().c_str(), wxConvUTF8);

              if(wsmy.ToDouble(&my) && mx && my)
              {
                  xyratio = my / mx;
                  wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                  TextCtrlXyRatioHipFire->SetValue(wsxyratio);
                  SpinCtrlXyRatioHipFire->SetValue(xyratio*100);
                  values[4] = xyratio;
              }
          }
      }
      else
      {
          aindex = getAxisIndex(button);

          if(aindex == ai_undef)
          {
              continue;
          }

          if(!axes[aindex].GetEvent()->GetId().empty())
          {
              continue;
          }

          if(button->GetLabel().empty())
          {
              axes[aindex] = *it;
              button->SetLabel(wxString(it->GetEvent()->GetId().c_str(), wxConvUTF8));
              string tt(it->GetEvent()->GetId());
              if(!it->GetLabel().empty())
              {
                  tt.append(" [");
                  tt.append(it->GetLabel());
                  tt.append("]");
              }
              button->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
          }
      }
  }

  /*
   * Load ADS config.
   */
  wsmx.erase();
  wsmy.erase();
  mx = 0;
  my = 0;
  axisMappers = configFile.GetController(0)->GetConfiguration(1)->GetAxisMapperList();
  for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
  {
      button = getAxisButton(it->GetAxis());

      if(button == NULL)
      {
          if(it->GetAxis() == "rstick x")
          {
              SpinCtrlDeadZoneADS->SetValue(wxAtoi(wxString(it->GetEvent()->GetDeadZone().c_str(), wxConvUTF8)));
              ChoiceDeadZoneShapeADS->SetStringSelection(wxString(it->GetEvent()->GetShape().c_str(), wxConvUTF8));

              SpinCtrlBufferSizeADS->SetValue(wxAtoi(wxString(it->GetEvent()->GetBufferSize().c_str(), wxConvUTF8)));

              wsf = wxString(it->GetEvent()->GetFilter().c_str(), wxConvUTF8);

              if(wsf.ToDouble(&f))
              {
                TextCtrlFilterADS->SetValue(wsf);
                SpinCtrlFilterADS->SetValue(f*100);
              }

              wsmx = wxString(it->GetEvent()->GetMultiplier().c_str(), wxConvUTF8);

              if(wsmx.ToDouble(&mx))
              {
                  TextCtrlSensitivityADS->SetValue(wsmx);
                  SpinCtrlSensitivityADS->SetValue(mx*100);
                  if(my && mx)
                  {
                      xyratio = my / mx;
                      wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                      TextCtrlXyRatioADS->SetValue(wsxyratio);
                      SpinCtrlXyRatioADS->SetValue(xyratio*100);
                  }
              }

              wsexp = wxString(it->GetEvent()->GetExponent().c_str(), wxConvUTF8);

              if(wsexp.ToDouble(&exp))
              {
                TextCtrlAccelerationADS->SetValue(wxString(it->GetEvent()->GetExponent().c_str(), wxConvUTF8));
                SpinCtrlAccelerationADS->SetValue(exp*100);
              }
          }
          else if(it->GetAxis() == "rstick y")
          {
              wsmy = wxString(it->GetEvent()->GetMultiplier().c_str(), wxConvUTF8);

              if(wsmy.ToDouble(&my) && mx && my)
              {
                  xyratio = my / mx;
                  wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                  TextCtrlXyRatioADS->SetValue(wsxyratio);
                  SpinCtrlXyRatioADS->SetValue(xyratio*100);
                  values[5] = xyratio;
              }
          }
      }
  }

  MenuItemSave->Enable(true);
}

void fpsconfigFrame::OnMenuOpen(wxCommandEvent& event)
{
    FileDialog1->SetDirectory(default_directory);

    if ( FileDialog1->ShowModal() != wxID_OK ) return;

    wxString FileName = FileDialog1->GetPath();
    if ( FileName.IsEmpty() ) return;

    configFile.ReadConfigFile(string(FileName.mb_str()));

    LoadConfig();
}

void fpsconfigFrame::OnTextCtrlText(wxCommandEvent& event)
{
    wxString str;
    wxTextCtrl* text;
    double value;
    int ivalue;

    text = (wxTextCtrl*)event.GetEventObject();
    str = text->GetValue();

    if(str.IsEmpty() && event.GetEventType() != wxEVT_COMMAND_TEXT_ENTER)
    {
        return;
    }

    if(str.Replace(_(","), _(".")))
    {
        text->SetValue(str);
    }

    if(!str.ToDouble(&value))
    {
        if (text == TextCtrlFilterHipFire || text == TextCtrlFilterADS)
        {
            text->SetValue(_("0.00"));
        }
        else
        {
            text->SetValue(_("1.00"));
        }
    }
    else
    {
        ivalue = round(value * 100);
        if(text == TextCtrlSensitivityHipFire)
        {
            if(value > 100)
            {
                value = 100;
                ivalue = value * 100;
                text->SetValue(_("100.00"));
            }
            else if(value < -100)
            {
                value = -100;
                ivalue = value * 100;
                text->SetValue(_("-100.00"));
            }
            SpinCtrlSensitivityHipFire->SetValue(ivalue);
            values[0] = value;
        }
        else if(text == TextCtrlSensitivityADS)
        {
            if(value > 100)
            {
                value = 100;
                ivalue = value * 100;
                text->SetValue(_("100.00"));
            }
            else if(value < -100)
            {
                value = -100;
                ivalue = value * 100;
                text->SetValue(_("-100.00"));
            }
            SpinCtrlSensitivityADS->SetValue(ivalue);
            values[1] = value;
        }
        else if(text == TextCtrlAccelerationHipFire)
        {
            if(value > 2)
            {
                value = 2;
                ivalue = value * 100;
                text->SetValue(_("2.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(_("0.00"));
            }
            SpinCtrlAccelerationHipFire->SetValue(ivalue);
            values[2] = value;
        }
        else if(text == TextCtrlAccelerationADS)
        {
            if(value > 2)
            {
                value = 2;
                ivalue = value * 100;
                text->SetValue(_("2.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(_("0.00"));
            }
            SpinCtrlAccelerationADS->SetValue(ivalue);
            values[3] = value;
        }
        else if(text == TextCtrlXyRatioHipFire)
        {
            SpinCtrlXyRatioHipFire->SetValue(ivalue);
            values[4] = value;
        }
        else if(text == TextCtrlXyRatioADS)
        {
            SpinCtrlXyRatioADS->SetValue(ivalue);
            values[5] = value;
        }
        else if(text == TextCtrlFilterHipFire)
        {
            if(value > 1)
            {
                value = 1;
                ivalue = value * 100;
                text->SetValue(_("1.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(_("0.00"));
            }
            SpinCtrlFilterHipFire->SetValue(ivalue);
            values[6] = value;
        }
        else if(text == TextCtrlFilterADS)
        {
            if(value > 1)
            {
                value = 1;
                ivalue = value * 100;
                text->SetValue(_("1.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(_("0.00"));
            }
            SpinCtrlFilterADS->SetValue(ivalue);
            values[7] = value;
        }
    }
}

#define STEP 100

void fpsconfigFrame::OnMouseDPIChange(wxSpinEvent& event)
{
    int v = SpinCtrlDPI->GetValue();
    int vceil = ceil((double)v/STEP)*STEP;
    int vfloor = floor((double)v/STEP)*STEP;
    int new_dpi;
    if(vceil-v > STEP/2)
    {
        new_dpi = vceil;
    }
    else
    {
        new_dpi = vfloor;
    }

    current_dpi = new_dpi;
    SpinCtrlDPI->SetValue(new_dpi);
}

void fpsconfigFrame::OnMenuUpdate(wxCommandEvent& event)
{
  int ret;

  updater u(VERSION_URL, VERSION_FILE, INFO_VERSION, DOWNLOAD_URL, DOWNLOAD_FILE);

  ret = u.checkversion();

  if (ret > 0)
  {
    int answer = wxMessageBox(_("Update available.\nStart installation?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    if (u.update() < 0)
    {
      wxMessageBox(wxT("Can't retrieve update file!"), wxT("Error"), wxICON_ERROR);
    }
  }
  else if (ret < 0)
  {
    wxMessageBox(wxT("Can't check version!"), wxT("Error"), wxICON_ERROR);
  }
  else
  {
    wxMessageBox(wxT("GIMX is up-to-date!"), wxT("Info"), wxICON_INFORMATION);
  }
}

void fpsconfigFrame::OnButtonConvertSensitivityClick(wxCommandEvent& event)
{
  unsigned int dest_value = 0;
  wxString wsm;

  if(!current_dpi)
  {
    return;
  }

  wxNumberEntryDialog dialog(this, wxT(""), wxT("Enter a number:"), wxT("Destination mouse DPI"), current_dpi, 100, 10000);

  if (dialog.ShowModal() == wxID_OK)
  {
    dest_value = dialog.GetValue();

    if(dest_value == current_dpi)
    {
      return;
    }

    /*
     * Store the new x multipliers so as not to loose precision due to rounding.
     */
    values[0] = values[0]*pow((double)current_dpi/dest_value, values[2]);
    SpinCtrlSensitivityHipFire->SetValue(values[0]*100);
    wsm = wxString::Format(wxT("%.02f"), (double)values[0]);

    TextCtrlSensitivityHipFire->SetValue(wsm);

    values[1] = values[1]*pow((double)current_dpi/dest_value, values[3]);
    SpinCtrlSensitivityADS->SetValue(values[1]*100);
    wsm = wxString::Format(wxT("%.02f"), (double)values[1]);

    TextCtrlSensitivityADS->SetValue(wsm);

    current_dpi = dest_value;
    SpinCtrlDPI->SetValue(dest_value);

  }
}

void fpsconfigFrame::OnMenuAutoBindControls(wxCommandEvent& event)
{
  if(configFile.GetFilePath().empty())
  {
    wxMessageBox( wxT("No config opened!"), wxT("Error"), wxICON_ERROR);
    return;
  }

  wxFileDialog FileDialog(this, _("Select the reference config."), default_directory, wxEmptyString, _T("XML files (*.xml)|*.xml"), wxFD_DEFAULT_STYLE);

  if ( FileDialog.ShowModal() != wxID_OK ) return;

  wxString FileName = FileDialog.GetPath();
  if ( FileName.IsEmpty() ) return;

  if(configFile.AutoBind(string(FileName.mb_str())) < 0)
  {
    wxMessageBox(wxT("Can't auto-bind controls!"), wxT("Error"), wxICON_ERROR);
  }
  else
  {
    LoadConfig();
    wxMessageBox(wxT("Auto-bind done!"), wxT("Info"), wxICON_INFORMATION);
  }
}
