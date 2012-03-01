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
const long fpsconfigFrame::ID_CHECKBOX1 = wxNewId();
const long fpsconfigFrame::ID_STATICTEXT9 = wxNewId();
const long fpsconfigFrame::ID_PANEL1 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM1 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM4 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM2 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM3 = wxNewId();
const long fpsconfigFrame::idMenuQuit = wxNewId();
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
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, wxID_ANY, _("Gimx-fpsconfig"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(614,423));
    SetBackgroundColour(wxColour(255,255,255));
    Panel1 = new wxPanel(this, ID_PANEL1, wxPoint(0,0), wxSize(613,366), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Panel1->SetBackgroundColour(wxColour(255,255,255));
    SpinCtrl8 = new wxSpinCtrl(Panel1, ID_SPINCTRL8, _T("100"), wxPoint(486,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL8"));
    SpinCtrl8->SetValue(_T("100"));
    SpinCtrl7 = new wxSpinCtrl(Panel1, ID_SPINCTRL7, _T("100"), wxPoint(486,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL7"));
    SpinCtrl7->SetValue(_T("100"));
    SpinCtrl6 = new wxSpinCtrl(Panel1, ID_SPINCTRL6, _T("100"), wxPoint(418,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL6"));
    SpinCtrl6->SetValue(_T("100"));
    SpinCtrl5 = new wxSpinCtrl(Panel1, ID_SPINCTRL5, _T("100"), wxPoint(418,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL5"));
    SpinCtrl5->SetValue(_T("100"));
    SpinCtrl4 = new wxSpinCtrl(Panel1, ID_SPINCTRL4, _T("100"), wxPoint(360,352), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL4"));
    SpinCtrl4->SetValue(_T("100"));
    SpinCtrl10 = new wxSpinCtrl(Panel1, ID_SPINCTRL10, _T("0"), wxPoint(588,320), wxSize(19,-1), 0, -10000, 10000, 0, _T("ID_SPINCTRL10"));
    SpinCtrl10->SetValue(_T("0"));
    SpinCtrl11 = new wxSpinCtrl(Panel1, ID_SPINCTRL11, _T("0"), wxPoint(588,352), wxSize(19,-1), 0, -10000, 10000, 0, _T("ID_SPINCTRL11"));
    SpinCtrl11->SetValue(_T("0"));
    SpinCtrl3 = new wxSpinCtrl(Panel1, ID_SPINCTRL3, _T("100"), wxPoint(360,320), wxSize(19,-1), 0, -10000, 10000, 100, _T("ID_SPINCTRL3"));
    SpinCtrl3->SetValue(_T("100"));
    stickright = new wxButton(Panel1, ID_BUTTON10, wxEmptyString, wxPoint(256,222), wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON10"));
    wxFont stickrightFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    stickright->SetFont(stickrightFont);
    SpinCtrl13 = new wxSpinCtrl(Panel1, ID_SPINCTRL13, _T("1"), wxPoint(506,320), wxSize(44,-1), 0, 1, 30, 1, _T("ID_SPINCTRL13"));
    SpinCtrl13->SetValue(_T("1"));
    SpinCtrl13->SetToolTip(_("Buffer size"));
    SpinCtrl14 = new wxSpinCtrl(Panel1, ID_SPINCTRL14, _T("1"), wxPoint(506,352), wxSize(44,-1), 0, 1, 30, 1, _T("ID_SPINCTRL14"));
    SpinCtrl14->SetValue(_T("1"));
    SpinCtrl14->SetToolTip(_("Buffer size"));
    SpinCtrl1 = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("20"), wxPoint(184,320), wxSize(44,-1), 0, 0, 64, 20, _T("ID_SPINCTRL1"));
    SpinCtrl1->SetValue(_T("20"));
    SpinCtrl1->SetToolTip(_("Dead zone"));
    SpinCtrl2 = new wxSpinCtrl(Panel1, ID_SPINCTRL2, _T("20"), wxPoint(184,352), wxSize(44,-1), 0, 0, 64, 20, _T("ID_SPINCTRL2"));
    SpinCtrl2->SetValue(_T("20"));
    SpinCtrl2->SetToolTip(_("Dead zone"));
    TextCtrl24 = new wxTextCtrl(Panel1, ID_TEXTCTRL24, _("1.00"), wxPoint(312,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL24"));
    TextCtrl24->SetToolTip(_("Sensitivity"));
    TextCtrl4 = new wxTextCtrl(Panel1, ID_TEXTCTRL4, _("1.00"), wxPoint(312,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    TextCtrl4->SetToolTip(_("Sensitivity"));
    TextCtrl26 = new wxTextCtrl(Panel1, ID_TEXTCTRL26, _("1.00"), wxPoint(380,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL26"));
    TextCtrl26->SetToolTip(_("Acceleration"));
    TextCtrl2 = new wxTextCtrl(Panel1, ID_TEXTCTRL2, _("0.00"), wxPoint(550,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrl2->SetToolTip(_("Filter"));
    TextCtrl3 = new wxTextCtrl(Panel1, ID_TEXTCTRL3, _("0.00"), wxPoint(550,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    TextCtrl3->SetToolTip(_("Filter"));
    TextCtrl22 = new wxTextCtrl(Panel1, ID_TEXTCTRL22, _("1.00"), wxPoint(380,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL22"));
    TextCtrl22->SetToolTip(_("Acceleration"));
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("primary"), wxPoint(120,328), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    StaticText7 = new wxStaticText(Panel1, ID_STATICTEXT7, _("Shape"), wxPoint(246,296), wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    StaticText7->SetToolTip(_("Dead zone shape"));
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("secondary"), wxPoint(104,360), wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("DZ"), wxPoint(192,296), wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    StaticText3->SetToolTip(_("Dead zone"));
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Sens."), wxPoint(320,296), wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    StaticText4->SetToolTip(_("Sensitivity"));
    StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT5, _("Accel."), wxPoint(382,296), wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    StaticText5->SetToolTip(_("Acceleration"));
    StaticText6 = new wxStaticText(Panel1, ID_STATICTEXT6, _("x/y"), wxPoint(452,296), wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    StaticText6->SetToolTip(_("x/y ratio"));
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
    Choice2 = new wxChoice(Panel1, ID_CHOICE2, wxPoint(230,320), wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice2->SetSelection( Choice2->Append(_("Circle")) );
    Choice2->Append(_("Rectangle"));
    Choice2->SetToolTip(_("Dead zone shape"));
    Choice1 = new wxChoice(Panel1, ID_CHOICE1, wxPoint(230,352), wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice1->SetSelection( Choice1->Append(_("Circle")) );
    Choice1->Append(_("Rectangle"));
    Choice1->SetToolTip(_("Dead zone shape"));
    TextCtrl1 = new wxTextCtrl(Panel1, ID_TEXTCTRL1, _("1.00"), wxPoint(438,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrl1->SetToolTip(_("x/y ratio"));
    TextCtrl25 = new wxTextCtrl(Panel1, ID_TEXTCTRL25, _("1.00"), wxPoint(438,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL25"));
    TextCtrl25->SetToolTip(_("x/y ratio"));
    SpinCtrl9 = new wxSpinCtrl(Panel1, ID_SPINCTRL9, _T("0"), wxPoint(24,256), wxSize(64,-1), 0, 0, 9900, 0, _T("ID_SPINCTRL9"));
    SpinCtrl9->SetValue(_T("0"));
    SpinCtrl9->SetToolTip(_("Set your mouse DPI if you are building a new config with unknown calibration parameters.\nTo use someone else\'s calibration parameters: set the parameters and the corresponding DPI, tick the box below, and set the new DPI."));
    StaticText8 = new wxStaticText(Panel1, ID_STATICTEXT8, _("Mouse DPI"), wxPoint(24,240), wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    CheckBox1 = new wxCheckBox(Panel1, ID_CHECKBOX1, _("adjust\nsensitivity"), wxPoint(24,280), wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox1->SetValue(false);
    CheckBox1->SetToolTip(_("Let this box unticked to build a new config with unknown calibration parameters."));
    StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT9, _("Smoothing"), wxPoint(514,296), wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    StaticText9->SetToolTip(_("Mouse smoothing"));
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu1, ID_MENUITEM1, _("New\tCtrl+N"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem6 = new wxMenuItem(Menu1, ID_MENUITEM4, _("Open\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem6);
    MenuItem4 = new wxMenuItem(Menu1, ID_MENUITEM2, _("Save\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem4);
    MenuItem4->Enable(false);
    MenuItem5 = new wxMenuItem(Menu1, ID_MENUITEM3, _("Save As"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem5);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuUpdate = new wxMenuItem(Menu2, ID_MENUITEM5, _("Update"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuUpdate);
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
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuNew);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuOpen);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuSave);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuSaveAs);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnQuit);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuUpdate);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnAbout);
    //*)

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

    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( wxT("Can't init ~/.emuclient/config!"), wxT("Error"), wxICON_ERROR);
    }
    if(system("test -d ~/.emuclient/config/example || (mkdir -p ~/.emuclient/config/example && cp /etc/emuclient/config/* ~/.emuclient/config/example)"))
    {
    }
#endif

    default_directory.Append(_(CONFIG_DIR));

    FileDialog1->SetDirectory(default_directory);

    current_dpi = 0;

    for(i=0; i<sizeof(values)/sizeof(values[0]); ++i)
    {
      values[i] = 1;
    }

    if(!file.IsEmpty())
    {
      wxString wxfile = wxEmptyString;

      if(file.EndsWith(_("*")))
      {
#ifndef WIN32
        wxfile.Append(wxFileName::GetHomeDir());
        wxfile.Append(_("/.emuclient/"));
#endif
        wxfile.Append(_(CONFIG_EXAMPLE_DIR));
        wxfile.Append(file.SubString(0, file.Length() - 2));
      }
      else
      {
        wxfile.Append(default_directory);
        wxfile.Append(file);
      }

      if(::wxFileExists(wxfile))
      {
        configFile.ReadConfigFile(wxfile);
        LoadConfig();
      }
      else
      {
        wxMessageBox( wxT("Cannot open config file: ") + wxString(file, wxConvUTF8), wxT("Error"), wxICON_ERROR);
      }

      if(file.EndsWith(_("*")))
      {
        wxMessageBox( wxT("This file is read-only."), wxT("Info"), wxICON_INFORMATION);
        MenuItem4->Enable(false);
      }
    }
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

    if(spin == SpinCtrl3)
    {
        updateTextCtrlValue(TextCtrl4, event, values);
    }
    else if(spin == SpinCtrl4)
    {
        updateTextCtrlValue(TextCtrl24, event, values+1);
    }
    else if(spin == SpinCtrl5)
    {
        updateTextCtrlValue(TextCtrl22, event, values+2);
    }
    else if(spin == SpinCtrl6)
    {
        updateTextCtrlValue(TextCtrl26, event, values+3);
    }
    else if(spin == SpinCtrl7)
    {
        updateTextCtrlValue(TextCtrl1, event, values+4);
    }
    else if(spin == SpinCtrl8)
    {
        updateTextCtrlValue(TextCtrl25, event, values+5);
    }
    else if(spin == SpinCtrl10)
    {
        updateTextCtrlValue(TextCtrl2, event, values+6);
    }
    else if(spin == SpinCtrl11)
    {
        updateTextCtrlValue(TextCtrl3, event, values+7);
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

wxButton* fpsconfigFrame::getButtonButton(wxString blabel)
{
    if(blabel == wxString(button_labels[bi_select], wxConvUTF8))
    {
        return select;
    }
    else if(blabel == wxString(button_labels[bi_start], wxConvUTF8))
    {
        return start;
    }
    else if(blabel == wxString(button_labels[bi_ps], wxConvUTF8))
    {
        return ps;
    }
    else if(blabel == wxString(button_labels[bi_up], wxConvUTF8))
    {
        return up;
    }
    else if(blabel == wxString(button_labels[bi_down], wxConvUTF8))
    {
        return down;
    }
    else if(blabel == wxString(button_labels[bi_right], wxConvUTF8))
    {
        return right;
    }
    else if(blabel == wxString(button_labels[bi_left], wxConvUTF8))
    {
        return left;
    }
    else if(blabel == wxString(button_labels[bi_square], wxConvUTF8))
    {
        return square;
    }
    else if(blabel == wxString(button_labels[bi_cross], wxConvUTF8))
    {
        return cross;
    }
    else if(blabel == wxString(button_labels[bi_circle], wxConvUTF8))
    {
        return circle;
    }
    else if(blabel == wxString(button_labels[bi_triangle], wxConvUTF8))
    {
        return triangle;
    }
    else if(blabel == wxString(button_labels[bi_l1], wxConvUTF8))
    {
        return l1;
    }
    else if(blabel == wxString(button_labels[bi_l2], wxConvUTF8))
    {
        return l2;
    }
    else if(blabel == wxString(button_labels[bi_l3], wxConvUTF8))
    {
        return l3;
    }
    else if(blabel == wxString(button_labels[bi_r1], wxConvUTF8))
    {
        return r1;
    }
    else if(blabel == wxString(button_labels[bi_r2], wxConvUTF8))
    {
        return r2;
    }
    else if(blabel == wxString(button_labels[bi_r3], wxConvUTF8))
    {
        return r3;
    }
    return NULL;//not supposed to occur...
}

wxButton* fpsconfigFrame::getAxisButton(wxString alabel)
{
    if(alabel == wxString(axis_labels[ai_ls_down], wxConvUTF8))
    {
        return stickdown;
    }
    else if(alabel == wxString(axis_labels[ai_ls_left], wxConvUTF8))
    {
        return stickleft;
    }
    else if(alabel == wxString(axis_labels[ai_ls_right], wxConvUTF8))
    {
        return stickright;
    }
    else if(alabel == wxString(axis_labels[ai_ls_up], wxConvUTF8))
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
    ((wxButton*)event.GetEventObject())->Enable(false);

    e_button_index bindex;
    e_axis_index aindex;

    evcatch.run();
    ((wxButton*)event.GetEventObject())->SetLabel(evcatch.GetEventId());
    ((wxButton*)event.GetEventObject())->SetToolTip(evcatch.GetEventId());

    bindex = getButtonIndex((wxButton*)event.GetEventObject());

    if(bindex != bi_undef)
    {
        buttons[bindex].SetDevice(Device(evcatch.GetDeviceType(), _("0"), _("")));
        buttons[bindex].SetEvent(Event(_("button"), evcatch.GetEventId()));
        buttons[bindex].SetButton(wxString(button_labels[bindex], wxConvUTF8));
    }
    else
    {
        aindex = getAxisIndex((wxButton*)event.GetEventObject());

        if(aindex != ai_undef)
        {
            axes[aindex].SetDevice(Device(evcatch.GetDeviceType(), _("0"), _("")));
            axes[aindex].SetEvent(Event(_("button"), evcatch.GetEventId()));
            axes[aindex].SetAxis(wxString(axis_labels[aindex], wxConvUTF8));
        }
    }

    ((wxButton*)event.GetEventObject())->Enable(true);
}

void fpsconfigFrame::OnMenuNew(wxCommandEvent& event)
{
    wxButton* button;

    for(int i=bi_select; i<BI_MAX; i++)
    {
        buttons[i] = ButtonMapper();
        button = getButtonButton(wxString(button_labels[i], wxConvUTF8));
        if(button != NULL)
        {
            button->SetLabel(_(""));
            button->SetToolTip(_(""));
        }
    }

    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        axes[i] = AxisMapper();
        button = getAxisButton(wxString(axis_labels[i], wxConvUTF8));
        if(button != NULL)
        {
            button->SetLabel(_(""));
            button->SetToolTip(_(""));
        }
    }

    SpinCtrl1->SetValue(20);
    SpinCtrl2->SetValue(20);
    Choice1->SetSelection(0);
    Choice2->SetSelection(0);
    TextCtrl4->SetValue(_("1.00"));
    TextCtrl24->SetValue(_("1.00"));
    TextCtrl22->SetValue(_("1.00"));
    TextCtrl26->SetValue(_("1.00"));
    TextCtrl1->SetValue(_("1.00"));
    TextCtrl25->SetValue(_("1.00"));

    configFile = ConfigurationFile();

    MenuItem4->Enable(false);
}

void fpsconfigFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _T("Save Config file"), _T(""), _T(""), _T("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(FileName);

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
    /*
     * Save DPI value.
     */
    configFile.GetController(0)->SetMouseDPI(current_dpi);
    /*
     * Save primary config.
     */
    //Save ButtonMappers
    buttonMappers = configFile.GetController(0)->GetConfiguration(0)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().IsEmpty())
        {
            found = false;
            for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end() && !found && !found; it++)
            {
                if(it->GetButton() == buttons[i].GetButton())
                {
                    it->SetEvent(*buttons[i].GetEvent());
                    if(it->GetDevice()->GetType() != buttons[i].GetDevice()->GetType())
                    {
                        it->SetDevice(*buttons[i].GetDevice());
                        if(it->GetDevice()->GetType() == _("mouse"))
                        {
                          it->GetDevice()->SetName(defaultMouseName);
                        }
                        else if(it->GetDevice()->GetType() == _("keyboard"))
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
        if(!axes[i].GetDevice()->GetType().IsEmpty())
        {
            found = false;
            for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
            {
                if(it->GetAxis() == axes[i].GetAxis())
                {
                  it->SetEvent(*axes[i].GetEvent());
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
        if(it->GetDevice()->GetType() == _("mouse") && it->GetEvent()->GetType() == _("axis") && it->GetEvent()->GetId() == _("x") && it->GetAxis() == _("rstick x"))
        {
            it->GetEvent()->SetDeadZone(wxString::Format(wxT("%i"),SpinCtrl1->GetValue()));
            it->GetEvent()->SetMultiplier(TextCtrl4->GetValue());
            it->GetEvent()->SetExponent(TextCtrl22->GetValue());
            it->GetEvent()->SetShape(Choice2->GetStringSelection());
            it->GetEvent()->SetBufferSize(wxString::Format(wxT("%i"),SpinCtrl13->GetValue()));
            it->GetEvent()->SetFilter(TextCtrl2->GetValue());
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper(_("mouse"), _("0"), _(""), _("axis"), _("x"), _("rstick x"), wxString::Format(wxT("%i"),SpinCtrl1->GetValue()), TextCtrl4->GetValue(), TextCtrl22->GetValue(), Choice2->GetStringSelection(), wxString::Format(wxT("%i"),SpinCtrl13->GetValue()), TextCtrl2->GetValue()));
    }
    wsmx = TextCtrl4->GetValue();
    wsxyratio = TextCtrl1->GetValue();
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
        if(it->GetDevice()->GetType() == _("mouse") && it->GetEvent()->GetType() == _("axis") && it->GetEvent()->GetId() == _("y") && it->GetAxis() == _("rstick y"))
        {
            it->GetEvent()->SetDeadZone(wxString::Format(wxT("%i"),SpinCtrl1->GetValue()));
            it->GetEvent()->SetMultiplier(wsmy);
            it->GetEvent()->SetExponent(TextCtrl22->GetValue());
            it->GetEvent()->SetShape(Choice2->GetStringSelection());
            it->GetEvent()->SetBufferSize(wxString::Format(wxT("%i"),SpinCtrl13->GetValue()));
            it->GetEvent()->SetFilter(TextCtrl2->GetValue());
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper(_("mouse"), _("0"), _(""), _("axis"), _("y"), _("rstick y"), wxString::Format(wxT("%i"),SpinCtrl1->GetValue()), wsmy, TextCtrl22->GetValue(), Choice2->GetStringSelection(), wxString::Format(wxT("%i"),SpinCtrl13->GetValue()), TextCtrl2->GetValue()));
    }
    /*
     * Save secondary config.
     */
    //Save Trigger
    if(configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->GetType() != _("mouse")
        || configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetEvent()->GetId() != (_("BUTTON_RIGHT"))
        || configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetSwitchBack() != _("yes"))
    {
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetType(_("mouse"));
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetName(_(""));
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetId(_("0"));
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetEvent()->SetId(_("BUTTON_RIGHT"));
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetSwitchBack(_("yes"));
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetDelay(0);
    }
    //Save ButtonMappers
    buttonMappers = configFile.GetController(0)->GetConfiguration(1)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().IsEmpty())
        {
            found = false;
            for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end() && !found; it++)
            {
                if(it->GetButton() == buttons[i].GetButton())
                {
                    it->SetEvent(*buttons[i].GetEvent());
                    if(it->GetDevice()->GetType() != buttons[i].GetDevice()->GetType())
                    {
                        it->SetDevice(*buttons[i].GetDevice());
                        if(it->GetDevice()->GetType() == _("mouse"))
                        {
                          it->GetDevice()->SetName(defaultMouseName);
                        }
                        else if(it->GetDevice()->GetType() == _("keyboard"))
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
        if(!axes[i].GetDevice()->GetType().IsEmpty())
        {
            found = false;
            for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end() && !found; it++)
            {
                if(it->GetAxis() == axes[i].GetAxis())
                {
                  it->SetEvent(*axes[i].GetEvent());
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
        if(it->GetDevice()->GetType() == _("mouse") && it->GetEvent()->GetType() == _("axis") && it->GetEvent()->GetId() == _("x") && it->GetAxis() == _("rstick x"))
        {
            it->GetEvent()->SetDeadZone(wxString::Format(wxT("%i"),SpinCtrl2->GetValue()));
            it->GetEvent()->SetMultiplier(TextCtrl24->GetValue());
            it->GetEvent()->SetExponent(TextCtrl26->GetValue());
            it->GetEvent()->SetShape(Choice1->GetStringSelection());
            it->GetEvent()->SetBufferSize(wxString::Format(wxT("%i"),SpinCtrl14->GetValue()));
            it->GetEvent()->SetFilter(TextCtrl3->GetValue());
            found = true;
        }
    }
    if(found == false)
    {
      axisMappers->push_front(AxisMapper(_("mouse"), _("0"), _(""), _("axis"), _("x"), _("rstick x"), wxString::Format(wxT("%i"),SpinCtrl2->GetValue()), TextCtrl24->GetValue(), TextCtrl26->GetValue(), Choice1->GetStringSelection(), wxString::Format(wxT("%i"), SpinCtrl14->GetValue()), TextCtrl3->GetValue()));
    }
    wsmx = TextCtrl24->GetValue();
    wsxyratio = TextCtrl25->GetValue();
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
        if(it->GetDevice()->GetType() == _("mouse") && it->GetEvent()->GetType() == _("axis") && it->GetEvent()->GetId() == _("y") && it->GetAxis() == _("rstick y"))
        {
            it->GetEvent()->SetDeadZone(wxString::Format(wxT("%i"),SpinCtrl2->GetValue()));
            it->GetEvent()->SetMultiplier(wsmy);
            it->GetEvent()->SetExponent(TextCtrl26->GetValue());
            it->GetEvent()->SetShape(Choice1->GetStringSelection());
            it->GetEvent()->SetBufferSize(wxString::Format(wxT("%i"),SpinCtrl14->GetValue()));
            it->GetEvent()->SetFilter(TextCtrl3->GetValue());
            found = true;
        }
    }
    if(found == false)
    {
        axisMappers->push_front(AxisMapper(_("mouse"), _("0"), _(""), _("axis"), _("y"), _("rstick y"), wxString::Format(wxT("%i"),SpinCtrl2->GetValue()), wsmy, TextCtrl26->GetValue(), Choice1->GetStringSelection(), wxString::Format(wxT("%i"), SpinCtrl14->GetValue()), TextCtrl3->GetValue()));
    }

    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(wxT("Can't save ") + configFile.GetFilePath(), wxT("Error"), wxICON_ERROR);
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
  SpinCtrl9->SetValue(current_dpi);

  defaultMouseName = wxEmptyString;
  defaultKeyboardName = wxEmptyString;

  /*
   * Load primary config.
   */
  //Load ButtonMappers
  for(int i=bi_select; i<BI_MAX; i++)
  {
      buttons[i] = ButtonMapper();
      button = getButtonButton(wxString(button_labels[i], wxConvUTF8));
      if(button != NULL)
      {
          button->SetLabel(_(""));
          button->SetToolTip(_(""));
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

      if(!buttons[bindex].GetEvent()->GetId().IsEmpty())
      {
          continue;
      }

      buttons[bindex] = *it;
      button->SetLabel(it->GetEvent()->GetId());
      button->SetToolTip(it->GetEvent()->GetId());
      if(!it->GetDevice()->GetName().IsEmpty())
      {
          if(it->GetDevice()->GetType() == _("mouse"))
          {
              defaultMouseName = it->GetDevice()->GetName();
          }
          else if(it->GetDevice()->GetType() == _("keyboard"))
          {
              defaultKeyboardName = it->GetDevice()->GetName();
          }
      }
  }
  //Load AxisMappers
  for(int i=ai_ls_up; i<AI_MAX; i++)
  {
      axes[i] = AxisMapper();
      button = getAxisButton(wxString(axis_labels[i], wxConvUTF8));
      if(button != NULL)
      {
          button->SetLabel(_(""));
          button->SetToolTip(_(""));
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
          if(it->GetAxis() == _("rstick x"))
          {
              SpinCtrl1->SetValue(wxAtoi(it->GetEvent()->GetDeadZone()));
              Choice2->SetStringSelection(it->GetEvent()->GetShape());

              SpinCtrl13->SetValue(wxAtoi(it->GetEvent()->GetBufferSize()));

              wsf = it->GetEvent()->GetFilter();

              if(wsf.ToDouble(&f))
              {
                TextCtrl2->SetValue(wsf);
                SpinCtrl10->SetValue(f*100);
              }

              wsmx = it->GetEvent()->GetMultiplier();

              if(wsmx.ToDouble(&mx))
              {
                  TextCtrl4->SetValue(wsmx);
                  SpinCtrl3->SetValue(mx*100);
                  if(my && mx)
                  {
                      xyratio = my / mx;
                      wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                      TextCtrl1->SetValue(wsxyratio);
                      SpinCtrl7->SetValue(xyratio*100);
                  }
              }

              wsexp = it->GetEvent()->GetExponent();

              if(wsexp.ToDouble(&exp))
              {
                TextCtrl22->SetValue(it->GetEvent()->GetExponent());
                SpinCtrl5->SetValue(exp*100);
              }
          }
          else if(it->GetAxis() == _("rstick y"))
          {
              wsmy = it->GetEvent()->GetMultiplier();

              if(wsmy.ToDouble(&my) && mx && my)
              {
                  xyratio = my / mx;
                  wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                  TextCtrl1->SetValue(wsxyratio);
                  SpinCtrl7->SetValue(xyratio*100);
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

          if(!axes[aindex].GetEvent()->GetId().IsEmpty())
          {
              continue;
          }

          if(button->GetLabel().IsEmpty())
          {
              axes[aindex] = *it;
              button->SetLabel(it->GetEvent()->GetId());
              button->SetToolTip(it->GetEvent()->GetId());
          }
      }
  }

  /*
   * Load secondary config.
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
          if(it->GetAxis() == _("rstick x"))
          {
              SpinCtrl2->SetValue(wxAtoi(it->GetEvent()->GetDeadZone()));
              Choice1->SetStringSelection(it->GetEvent()->GetShape());

              SpinCtrl14->SetValue(wxAtoi(it->GetEvent()->GetBufferSize()));

              wsf = it->GetEvent()->GetFilter();

              if(wsf.ToDouble(&f))
              {
                TextCtrl3->SetValue(wsf);
                SpinCtrl11->SetValue(f*100);
              }

              wsmx = it->GetEvent()->GetMultiplier();

              if(wsmx.ToDouble(&mx))
              {
                  TextCtrl24->SetValue(wsmx);
                  SpinCtrl4->SetValue(mx*100);
                  if(my && mx)
                  {
                      xyratio = my / mx;
                      wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                      TextCtrl25->SetValue(wsxyratio);
                      SpinCtrl8->SetValue(xyratio*100);
                  }
              }

              wsexp = it->GetEvent()->GetExponent();

              if(wsexp.ToDouble(&exp))
              {
                TextCtrl26->SetValue(it->GetEvent()->GetExponent());
                SpinCtrl6->SetValue(exp*100);
              }
          }
          else if(it->GetAxis() == _("rstick y"))
          {
              wsmy = it->GetEvent()->GetMultiplier();

              if(wsmy.ToDouble(&my) && mx && my)
              {
                  xyratio = my / mx;
                  wsxyratio = wxString::Format(wxT("%.02f"), xyratio);

                  TextCtrl25->SetValue(wsxyratio);
                  SpinCtrl8->SetValue(xyratio*100);
                  values[5] = xyratio;
              }
          }
      }
  }

  MenuItem4->Enable(true);
}

void fpsconfigFrame::OnMenuOpen(wxCommandEvent& event)
{
    if ( FileDialog1->ShowModal() != wxID_OK ) return;

    wxString FileName = FileDialog1->GetPath();
    if ( FileName.IsEmpty() ) return;

    configFile.ReadConfigFile(FileName);

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
        if (text == TextCtrl2 || text == TextCtrl3)
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
        if(text == TextCtrl4)
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
            SpinCtrl3->SetValue(ivalue);
            values[0] = value;
        }
        else if(text == TextCtrl24)
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
            SpinCtrl4->SetValue(ivalue);
            values[1] = value;
        }
        else if(text == TextCtrl22)
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
            SpinCtrl5->SetValue(ivalue);
            values[2] = value;
        }
        else if(text == TextCtrl26)
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
            SpinCtrl6->SetValue(ivalue);
            values[3] = value;
        }
        else if(text == TextCtrl1)
        {
            SpinCtrl7->SetValue(ivalue);
            values[4] = value;
        }
        else if(text == TextCtrl25)
        {
            SpinCtrl8->SetValue(ivalue);
            values[5] = value;
        }
        else if(text == TextCtrl2)
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
            SpinCtrl10->SetValue(ivalue);
            values[6] = value;
        }
        else if(text == TextCtrl3)
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
            SpinCtrl11->SetValue(ivalue);
            values[7] = value;
        }
    }
}

#define STEP 100

void fpsconfigFrame::OnMouseDPIChange(wxSpinEvent& event)
{
    int v = SpinCtrl9->GetValue();
    int vceil = ceil((double)v/STEP)*STEP;
    int vfloor = floor((double)v/STEP)*STEP;
    int new_dpi;
    wxString wsm;
    if(vceil-v > STEP/2)
    {
        new_dpi = vceil;
    }
    else
    {
        new_dpi = vfloor;
    }

    if(CheckBox1->IsChecked() && current_dpi && new_dpi)
    {
        /*
         * Store the new x multipliers so as not to loose precision due to rounding.
         */
        values[0] = values[0]*pow((double)current_dpi/new_dpi, values[2]);
        SpinCtrl3->SetValue(values[0]*100);
        wsm = wxString::Format(wxT("%.02f"), (double)values[0]);

        TextCtrl4->SetValue(wsm);

        values[1] = values[1]*pow((double)current_dpi/new_dpi, values[3]);
        SpinCtrl4->SetValue(values[1]*100);
        wsm = wxString::Format(wxT("%.02f"), (double)values[1]);

        TextCtrl24->SetValue(wsm);
    }

    current_dpi = new_dpi;
    SpinCtrl9->SetValue(new_dpi);
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
