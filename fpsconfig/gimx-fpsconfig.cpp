/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "gimx-fpsconfig.h"
#include <wx/msgdlg.h>
#include <ConfigurationFile.h>

#ifndef WIN32
#include <pwd.h>
#endif

//(*InternalHeaders(fpsconfigFrame)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/font.h>
//*)

#include "background.png.cpp"
#include <wx/mstream.h>

#include <wx/aboutdlg.h>
#include "fpsconfig.h"

#include <locale.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include "../shared/updater/updater.h"
#include "../directories.h"

#include <wx/tooltip.h>
#include "wx/numdlg.h"

#include <sstream>

#include <libintl.h>
#include <wx/stdpaths.h>
#include <wx/busyinfo.h>

using namespace std;

/*
 * \brief Give the original english string for a locale string. \
 *        This function is useful for a few strings only.
 *
 * \param str the locale string
 *
 * \return the original string, or the string itself (if no original string found).
 */
string fpsconfigFrame::reverseTranslate(string str)
{
  wxString wxStr = wxString(str.c_str(), wxConvUTF8);

  if(wxStr == _("Circle"))
  {
    return "Circle";
  }
  if(wxStr == _("Rectangle"))
  {
    return "Rectangle";
  }

  return str;
}

class wxComboBoxDialog: public wxDialog
{
public:

	wxComboBoxDialog ( wxWindow * parent, const wxString & title, const wxString& value, const wxArrayString& choices ) : wxDialog( parent, -1, title, wxDefaultPosition, wxSize(150, 75), wxDEFAULT_DIALOG_STYLE)
  {
    vbox = new wxBoxSizer(wxVERTICAL);

    comboBox = new wxComboBox(this, -1, value, wxDefaultPosition, wxDefaultSize, choices, wxTE_PROCESS_ENTER);

    vbox->Add(comboBox, 1, wxALIGN_CENTER | wxEXPAND);

    hbox = new wxBoxSizer(wxHORIZONTAL);

    bOK = new wxButton( this, wxID_OK, _("OK"));
    bCancel = new wxButton( this, wxID_CANCEL, _("Cancel"));

    hbox->Add(bOK, 1, wxALIGN_CENTER | wxEXPAND);
    hbox->Add(bCancel, 1, wxALIGN_CENTER | wxEXPAND);

    vbox->Add(hbox, 1, wxALIGN_CENTER | wxEXPAND);

    SetSizer(vbox);

    Connect(comboBox->GetId(), wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( wxComboBoxDialog::OnComboBoxEnter ) );

    comboBox->SetFocus();
  }

	wxString GetValue() { return comboBox->GetValue(); }

  void OnComboBoxEnter( wxCommandEvent &event ) { EndModal(wxID_OK); }

protected:

  wxBoxSizer* hbox;
  wxBoxSizer* vbox;
  wxComboBox* comboBox;
  wxButton* bOK;
  wxButton* bCancel;
};

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
const long fpsconfigFrame::ID_MENUITEM8 = wxNewId();
const long fpsconfigFrame::idMenuQuit = wxNewId();
const long fpsconfigFrame::ID_MENUITEM6 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM7 = wxNewId();
const long fpsconfigFrame::ID_MENUITEM9 = wxNewId();
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
    "abs_axis_0", "abs_axis_1", "abs_axis_2",
    "abs_axis_3", "abs_axis_4", "abs_axis_5", "abs_axis_6",
    "abs_axis_7", "abs_axis_8", "abs_axis_9", "abs_axis_10",
    "abs_axis_11", "abs_axis_12",
    "abs_axis_13", "abs_axis_14",
    "abs_axis_15", "abs_axis_16"
};

const char* axis_labels[AI_MAX] =
{
    "undef",
    "rel_axis_1-",
    "rel_axis_0+",
    "rel_axis_1+",
    "rel_axis_0-"
};

fpsconfigFrame::fpsconfigFrame(wxString file,wxWindow* parent,wxWindowID id)
{
    unsigned int i;

    locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

    //(*Initialize(fpsconfigFrame)
    wxMenu* MenuFile;
    wxMenu* MenuAdvanced;
    wxMenuItem* MenuItemQuit;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItemAbout;
    wxMenu* MenuHelp;

    Create(parent, wxID_ANY, _("Gimx-fpsconfig"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetBackgroundColour(wxColour(255,255,255));
    Panel1 = new wxPanel(this, ID_PANEL1, wxPoint(0,0), wxSize(-1,-1), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
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
    TextCtrlSensitivityADS = new wxTextCtrl(Panel1, ID_TEXTCTRL24, wxEmptyString, wxPoint(312,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL24"));
    TextCtrlSensitivityADS->SetToolTip(_("Sensitivity (ADS)"));
    TextCtrlSensitivityHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL4, wxEmptyString, wxPoint(312,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    TextCtrlSensitivityHipFire->SetToolTip(_("Sensitivity (Hip Fire)"));
    TextCtrlAccelerationADS = new wxTextCtrl(Panel1, ID_TEXTCTRL26, wxEmptyString, wxPoint(380,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL26"));
    TextCtrlAccelerationADS->SetToolTip(_("Acceleration (ADS)"));
    TextCtrlFilterHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL2, wxEmptyString, wxPoint(550,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrlFilterHipFire->SetToolTip(_("Filter (Hip Fire)"));
    TextCtrlFilterADS = new wxTextCtrl(Panel1, ID_TEXTCTRL3, wxEmptyString, wxPoint(550,352), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    TextCtrlFilterADS->SetToolTip(_("Filter (ADS)"));
    TextCtrlAccelerationHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL22, wxEmptyString, wxPoint(380,320), wxSize(40,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL22"));
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
    TextCtrlXyRatioHipFire = new wxTextCtrl(Panel1, ID_TEXTCTRL1, wxEmptyString, wxPoint(438,320), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrlXyRatioHipFire->SetToolTip(_("x/y ratio (Hip Fire)"));
    TextCtrlXyRatioADS = new wxTextCtrl(Panel1, ID_TEXTCTRL25, wxEmptyString, wxPoint(438,352), wxSize(50,-1), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL25"));
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
    MenuItemOpenConfigDirectory = new wxMenuItem(MenuFile, ID_MENUITEM8, _("Open config directory"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemOpenConfigDirectory);
    MenuItemQuit = new wxMenuItem(MenuFile, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemQuit);
    MenuBar1->Append(MenuFile, _("&File"));
    MenuAdvanced = new wxMenu();
    MenuEditLabels = new wxMenuItem(MenuAdvanced, ID_MENUITEM6, _("Edit Labels"), wxEmptyString, wxITEM_CHECK);
    MenuAdvanced->Append(MenuEditLabels);
    MenuAutoBindControls = new wxMenuItem(MenuAdvanced, ID_MENUITEM7, _("Auto-bind controls"), wxEmptyString, wxITEM_NORMAL);
    MenuAdvanced->Append(MenuAutoBindControls);
    MenuItemWindowEvents = new wxMenuItem(MenuAdvanced, ID_MENUITEM9, _("Window events"), wxEmptyString, wxITEM_CHECK);
    MenuAdvanced->Append(MenuItemWindowEvents);
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
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuOpenConfigDirectory);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnQuit);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuAutoBindControls);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuItemWindowEventsSelected);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnMenuUpdate);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&fpsconfigFrame::OnAbout);
    //*)

    wxMemoryInputStream istream(background_png, sizeof background_png);
    wxImage background_img(istream, wxBITMAP_TYPE_PNG);
    wxBackgroundBitmap* ToolBarBackground = new wxBackgroundBitmap(wxBitmap(background_img));
    Panel1->PushEventHandler(ToolBarBackground);

    TextCtrlSensitivityHipFire->SetValue(wxT("1.00"));
    TextCtrlSensitivityADS->SetValue(wxT("1.00"));
    TextCtrlAccelerationHipFire->SetValue(wxT("1.00"));
    TextCtrlAccelerationADS->SetValue(wxT("1.00"));
    TextCtrlXyRatioHipFire->SetValue(wxT("1.00"));
    TextCtrlXyRatioADS->SetValue(wxT("1.00"));
    TextCtrlFilterHipFire->SetValue(wxT("0.00"));
    TextCtrlFilterADS->SetValue(wxT("0.00"));

    default_directory = wxEmptyString;

    defaultMouseName = "";
    defaultMouseId = "0";
    defaultKeyboardName = "";
    defaultKeyboardId = "0";

#ifndef WIN32
    if(!getuid())
    {
      int answer = wxMessageBox(_("It's not recommended to run as root user. Continue?"), _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        exit(0);
      }
    }
#endif

    //migrate configs
    wxString oldGimxDir = wxStandardPaths::Get().GetUserConfigDir().Append(wxT(OLD_GIMX_DIR));
    wxString gimxDir = wxStandardPaths::Get().GetUserConfigDir().Append(wxT(GIMX_DIR));
    if(wxDir::Exists(oldGimxDir) && !wxDir::Exists(gimxDir))
    {
      if(!wxRenameFile(oldGimxDir, gimxDir))
      {
        wxMessageBox( _("Can't migrate directory: ") + oldGimxDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    /* Retrieve config/ directory location */
    default_directory = wxStandardPaths::Get().GetUserConfigDir();
    default_directory.Append(wxT(GIMX_DIR));
    if(!wxDir::Exists(default_directory))
    {
      if(!wxMkdir(default_directory))
      {
        wxMessageBox( _("Can't init directory: ") + default_directory, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }
    default_directory.Append(wxT(CONFIG_DIR));
    if(!wxDir::Exists(default_directory))
    {
      if(!wxMkdir(default_directory))
      {
        wxMessageBox( _("Can't init directory: ") + default_directory, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    FileDialog1->SetDirectory(default_directory);

    current_dpi = 0;

    for(i=0; i<sizeof(values)/sizeof(values[0]); ++i)
    {
      values[i] = 1;
    }

    evcatch = event_catcher::getInstance();

  	/* Open the file given as argument. */
    if(!file.IsEmpty())
    {
      wxString wxfile = default_directory + file;

      if(::wxFileExists(wxfile))
      {
        configFile.ReadConfigFile(string(wxfile.mb_str(wxConvUTF8)));
        LoadConfig();
        FileDialog1->SetFilename(file);
      }
      else
      {
        wxMessageBox( _("Cannot open config file: ") + file, _("Error"), wxICON_ERROR);
      }
    }

	  wxToolTip::SetDelay(0);

    readLabels();

    Panel1->Fit();
    Fit();
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
  info.SetVersion(wxT(INFO_VERSION) + wxString(wxT(" ")) + wxString(wxT(INFO_ARCH)));
  wxString text = wxString(wxT(INFO_DESCR)) + wxString(wxT("\n")) + wxString(wxT(INFO_YEAR)) + wxString(wxT(" ")) + wxString(wxT(INFO_DEV)) + wxString(wxT(" ")) + wxString(wxT(INFO_LICENCE));
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
  wxString value;
  wxArrayString* as = NULL;

  ((wxButton*) event.GetEventObject())->Enable(false);

  if (MenuEditLabels->IsChecked())
  {
    bindex = getButtonIndex((wxButton*) event.GetEventObject());

    if (bindex != bi_undef)
    {
      value = wxString(buttons[bindex].GetLabel().c_str(), wxConvUTF8);
      as = &b_labels;
    }
    else
    {
      aindex = getAxisIndex((wxButton*) event.GetEventObject());

      if (aindex != ai_undef)
      {
        value = wxString(axes[aindex].GetLabel().c_str(), wxConvUTF8);
        as = &a_labels;
      }
    }

    if(!as)
    {
      return;
    }

    wxComboBoxDialog dialog ( this, _("Edit Label"), value, *as);

    if (dialog.ShowModal() == wxID_OK)
    {
      if (bindex != bi_undef)
      {
        buttons[bindex].SetLabel(string(dialog.GetValue().mb_str(wxConvUTF8)));
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
        axes[aindex].SetLabel(string(dialog.GetValue().mb_str(wxConvUTF8)));
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
    evcatch->run("", "button");
    StatusBar1->SetStatusText(wxEmptyString);

    pair<Device, Event> ev = selectEvent();

    if(ev.first.GetType() == "joystick")
    {
      wxMessageBox(_("Joystick controls are only supported through gimx-config."), _("Info"), wxICON_INFORMATION);
      ((wxButton*) event.GetEventObject())->Enable(true);
      return;
    }

    string device_name = "";
    string device_id = "0";
    if(ev.first.GetType() == "keyboard")
    {
      if(!defaultKeyboardName.empty())
      {
        device_name = defaultKeyboardName;
        device_id = defaultKeyboardId;
      }
    }
    else if(ev.first.GetType() == "mouse")
    {
      if(!defaultMouseName.empty())
      {
        device_name = defaultMouseName;
        device_id = defaultMouseId;
      }
    }

    ((wxButton*) event.GetEventObject())->SetLabel(wxString(ev.second.GetId().c_str(), wxConvUTF8));

    bindex = getButtonIndex((wxButton*) event.GetEventObject());

    if (bindex != bi_undef)
    {
      buttons[bindex].SetDevice(Device(ev.first.GetType(), device_id, device_name));
      buttons[bindex].SetEvent(Event("button", ev.second.GetId()));
      buttons[bindex].SetAxis(ControlMapper::GetAxisProps(button_labels[bindex]));
      string tt(buttons[bindex].GetEvent()->GetId());
      if (!buttons[bindex].GetLabel().empty())
      {
        string l = buttons[bindex].GetLabel();
        size_t pos = l.find(", not found");
        size_t size = sizeof(", not found");
        if(pos == string::npos)
        {
          pos = l.find(", duplicate");
          size = sizeof(", duplicate");
        }
        if(pos != string::npos)
        {
          l.replace(pos, size, "");
          buttons[bindex].SetLabel(l);
          wxButton tmp;
          ((wxButton*) event.GetEventObject())->SetBackgroundColour(tmp.GetBackgroundColour());
        }
        tt.append(" [");
        tt.append(l);
        tt.append("]");
      }
      ((wxButton*) event.GetEventObject())->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
    }
    else
    {
      aindex = getAxisIndex((wxButton*) event.GetEventObject());

      if (aindex != ai_undef)
      {
        axes[aindex].SetDevice(Device(ev.first.GetType(), device_id, device_name));
        axes[aindex].SetEvent(Event("button", ev.second.GetId()));
        axes[aindex].SetAxis(ControlMapper::GetAxisProps(axis_labels[aindex]));
        string tt(axes[aindex].GetEvent()->GetId());
        if (!axes[aindex].GetLabel().empty())
        {
          string l = axes[aindex].GetLabel();
          size_t pos = l.find(", not found");
          size_t size = sizeof(", not found");
          if(pos == string::npos)
          {
            pos = l.find(", duplicate");
            size = sizeof(", duplicate");
          }
          if(pos != string::npos)
          {
            l.replace(pos, size, "");
            axes[aindex].SetLabel(l);
            wxButton tmp;
            ((wxButton*) event.GetEventObject())->SetBackgroundColour(tmp.GetBackgroundColour());
          }
          tt.append(" [");
          tt.append(l);
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

    defaultMouseName = "";
    defaultMouseId = "0";
    defaultKeyboardName = "";
    defaultKeyboardId = "0";

    wxButton* button;

    for(int i=bi_select; i<BI_MAX; i++)
    {
        buttons[i] = ControlMapper();
        old_buttons[i] = ControlMapper();
        button = getButtonButton(button_labels[i]);
        if(button != NULL)
        {
            button->SetLabel(wxEmptyString);
            button->UnsetToolTip();
            wxButton tmp;
            button->SetBackgroundColour(tmp.GetBackgroundColour());
        }
    }

    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        axes[i] = ControlMapper();
        old_axes[i] = ControlMapper();
        button = getAxisButton(axis_labels[i]);
        if(button != NULL)
        {
            button->SetLabel(wxEmptyString);
            button->UnsetToolTip();
            wxButton tmp;
            button->SetBackgroundColour(tmp.GetBackgroundColour());
        }
    }

    SpinCtrlDeadZoneHipFire->SetValue(20);
    SpinCtrlDeadZoneADS->SetValue(20);
    ChoiceDeadZoneShapeADS->SetSelection(0);
    ChoiceDeadZoneShapeHipFire->SetSelection(0);
    TextCtrlSensitivityHipFire->SetValue(wxT("1.00"));
    TextCtrlSensitivityADS->SetValue(wxT("1.00"));
    TextCtrlAccelerationHipFire->SetValue(wxT("1.00"));
    TextCtrlAccelerationADS->SetValue(wxT("1.00"));
    TextCtrlXyRatioHipFire->SetValue(wxT("1.00"));
    TextCtrlXyRatioADS->SetValue(wxT("1.00"));
    SpinCtrlBufferSizeHipFire->SetValue(1);
    SpinCtrlBufferSizeADS->SetValue(1);
    TextCtrlFilterHipFire->SetValue(wxT("0.00"));
    TextCtrlFilterADS->SetValue(wxT("0.00"));

    SpinCtrlDPI->Enable(true);
    SpinCtrlDPI->SetToolTip(_("Enter your mouse DPI value."));

    configFile = ConfigurationFile();

    MenuItemSave->Enable(false);
}

void fpsconfigFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Save config file"), wxT(""), wxT(""), _("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);
    saveFileDialog.SetFilename(FileDialog1->GetFilename());

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(string(FileName.mb_str(wxConvUTF8)));

    OnMenuSave(event);

    FileDialog1->SetFilename(FileName);

    MenuItemSave->Enable(true);
}

void fpsconfigFrame::OnMenuSave(wxCommandEvent& event)
{
    std::list<ControlMapper>* ButtonMappers;
    std::list<ControlMapper>* AxisMappers;
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
    ButtonMappers = configFile.GetController(0)->GetConfiguration(0)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ControlMapper>::iterator it = ButtonMappers->begin(); it!=ButtonMappers->end() && !found; ++it)
            {
                if(old_buttons[i] == *it)
                {
                    *it = buttons[i];
                    found = true;
                }
            }
            if(found == false)
            {
                ButtonMappers->push_front(buttons[i]);
            }
        }
    }
    //Save AxisMappers
    AxisMappers = configFile.GetController(0)->GetConfiguration(0)->GetAxisMapperList();
    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        if(!axes[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
            {
                if(old_axes[i] == *it)
                {
                  *it = axes[i];
                  found = true;
                }
            }
            if(found == false)
            {
                AxisMappers->push_front(axes[i]);
            }
        }
    }
    found = false;
    for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "x" && it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_2")))
        {
            it->GetEvent()->SetDeadZone(sDzHf);
            it->GetEvent()->SetMultiplier(string(TextCtrlSensitivityHipFire->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationHipFire->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetShape(reverseTranslate(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str(wxConvUTF8))));

            std::list<MouseOptions>* mouseOptions = configFile.GetController(0)->GetConfiguration(0)->GetMouseOptionsList();
            std::list<MouseOptions>::iterator it2;
            for(it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
            {
              if(it2->GetMouse()->GetName() == it->GetDevice()->GetName()
                 && it2->GetMouse()->GetId() == it->GetDevice()->GetId())
              {
                it2->SetBufferSize(sBsHf);
                it2->SetFilter(string(TextCtrlFilterHipFire->GetValue().mb_str(wxConvUTF8)));
                break;
              }
            }
            if(it2==mouseOptions->end())
            {
              mouseOptions->push_back(MouseOptions(it->GetDevice()->GetName(), it->GetDevice()->GetId(),
                  "Aiming", sBsHf, string(TextCtrlFilterHipFire->GetValue().mb_str(wxConvUTF8))));
            }

            found = true;
        }
    }
    if(found == false)
    {
        AxisMappers->push_front(ControlMapper("mouse", defaultMouseId, defaultMouseName, "axis", "x", ControlMapper::GetAxisProps("rel_axis_2"),
            sDzHf, string(TextCtrlSensitivityHipFire->GetValue().mb_str(wxConvUTF8)),
            string(TextCtrlAccelerationHipFire->GetValue().mb_str(wxConvUTF8)),
            reverseTranslate(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str(wxConvUTF8))), "Aiming - x axis"));

        std::list<MouseOptions>* mouseOptions = configFile.GetController(0)->GetConfiguration(0)->GetMouseOptionsList();
        std::list<MouseOptions>::iterator it2;
        for(it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
        {
          if(it2->GetMouse()->GetName() == defaultMouseName
             && it2->GetMouse()->GetId() == defaultMouseId)
          {
            it2->SetBufferSize(sBsHf);
            it2->SetFilter(string(TextCtrlFilterHipFire->GetValue().mb_str(wxConvUTF8)));
            break;
          }
        }
        if(it2==mouseOptions->end())
        {
          mouseOptions->push_back(MouseOptions(defaultMouseName, defaultMouseId, "Aiming",
              sBsHf, string(TextCtrlFilterHipFire->GetValue().mb_str(wxConvUTF8))));
        }
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
    for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "y" && it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_3")))
        {
            it->GetEvent()->SetDeadZone(sDzHf);
            it->GetEvent()->SetMultiplier(string(wsmy.mb_str(wxConvUTF8)));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationHipFire->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetShape(reverseTranslate(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str(wxConvUTF8))));
            found = true;
        }
    }
    if(found == false)
    {
        AxisMappers->push_front(ControlMapper("mouse", defaultMouseId, defaultMouseName, "axis", "y", ControlMapper::GetAxisProps("rel_axis_3"),
            sDzHf, string(wsmy.mb_str(wxConvUTF8)), string(TextCtrlAccelerationHipFire->GetValue().mb_str(wxConvUTF8)),
            reverseTranslate(string(ChoiceDeadZoneShapeHipFire->GetStringSelection().mb_str(wxConvUTF8))), "Aiming - y axis"));
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
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetName(defaultMouseName);
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetDevice()->SetId(defaultMouseId);
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->GetEvent()->SetId("BUTTON_RIGHT");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetSwitchBack("yes");
        configFile.GetController(0)->GetConfiguration(1)->GetTrigger()->SetDelay(0);
    }
    //Save ButtonMappers
    ButtonMappers = configFile.GetController(0)->GetConfiguration(1)->GetButtonMapperList();
    for(int i=bi_select; i<BI_MAX; i++)
    {
        if(!buttons[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ControlMapper>::iterator it = ButtonMappers->begin(); it!=ButtonMappers->end() && !found; ++it)
            {
                if(old_buttons[i] == *it)
                {
                    *it = buttons[i];
                    found = true;
                }
            }
            if(found == false)
            {
                ButtonMappers->push_front(buttons[i]);
            }
        }
    }
    //Save AxisMappers
    AxisMappers = configFile.GetController(0)->GetConfiguration(1)->GetAxisMapperList();
    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        if(!axes[i].GetDevice()->GetType().empty())
        {
            found = false;
            for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
            {
                if(old_axes[i] == *it)
                {
                    *it = axes[i];
                    found = true;
                }
            }
            if(found == false)
            {
                AxisMappers->push_front(axes[i]);
            }
        }
    }
    found = false;
    for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "x" && it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_2")))
        {
            it->GetEvent()->SetDeadZone(sDzADS);
            it->GetEvent()->SetMultiplier(string(TextCtrlSensitivityADS->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationADS->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetShape(reverseTranslate(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str(wxConvUTF8))));

            std::list<MouseOptions>* mouseOptions = configFile.GetController(0)->GetConfiguration(1)->GetMouseOptionsList();
            std::list<MouseOptions>::iterator it2;
            for(it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
            {
              if(it2->GetMouse()->GetName() == it->GetDevice()->GetName()
                 && it2->GetMouse()->GetId() == it->GetDevice()->GetId())
              {
                it2->SetBufferSize(sBsADS);
                it2->SetFilter(string(TextCtrlFilterADS->GetValue().mb_str(wxConvUTF8)));
                break;
              }
            }
            if(it2==mouseOptions->end())
            {
              mouseOptions->push_back(MouseOptions(it->GetDevice()->GetName(), it->GetDevice()->GetId(),
                  "Aiming", sBsADS, string(TextCtrlFilterADS->GetValue().mb_str(wxConvUTF8))));
            }

            found = true;
        }
    }
    if(found == false)
    {
      AxisMappers->push_front(ControlMapper("mouse", defaultMouseId, defaultMouseName, "axis", "x", ControlMapper::GetAxisProps("rel_axis_2"),
          sDzADS, string(TextCtrlSensitivityADS->GetValue().mb_str(wxConvUTF8)), string(TextCtrlAccelerationADS->GetValue().mb_str(wxConvUTF8)),
          reverseTranslate(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str(wxConvUTF8))), "Aiming - x axis"));

      std::list<MouseOptions>* mouseOptions = configFile.GetController(0)->GetConfiguration(1)->GetMouseOptionsList();
      std::list<MouseOptions>::iterator it2;
      for(it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
      {
        if(it2->GetMouse()->GetName() == defaultMouseName
           && it2->GetMouse()->GetId() == defaultMouseId)
        {
          it2->SetBufferSize(sBsADS);
          it2->SetFilter(string(TextCtrlFilterADS->GetValue().mb_str(wxConvUTF8)));
          break;
        }
      }
      if(it2==mouseOptions->end())
      {
        mouseOptions->push_back(MouseOptions(defaultMouseName, defaultMouseId, "Aiming",
            sBsADS, string(TextCtrlFilterADS->GetValue().mb_str(wxConvUTF8))));
      }
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
    for(std::list<ControlMapper>::iterator it = AxisMappers->begin(); it!=AxisMappers->end() && !found; ++it)
    {
        if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis" && it->GetEvent()->GetId() == "y" && it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_3")))
        {
            it->GetEvent()->SetDeadZone(sDzADS);
            it->GetEvent()->SetMultiplier(string(wsmy.mb_str(wxConvUTF8)));
            it->GetEvent()->SetExponent(string(TextCtrlAccelerationADS->GetValue().mb_str(wxConvUTF8)));
            it->GetEvent()->SetShape(reverseTranslate(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str(wxConvUTF8))));
            found = true;
        }
    }
    if(found == false)
    {
        AxisMappers->push_front(ControlMapper("mouse", defaultMouseId, defaultMouseName, "axis", "y", ControlMapper::GetAxisProps("rel_axis_3"),
            sDzADS, string(wsmy.mb_str(wxConvUTF8)), string(TextCtrlAccelerationADS->GetValue().mb_str(wxConvUTF8)),
            reverseTranslate(string(ChoiceDeadZoneShapeADS->GetStringSelection().mb_str(wxConvUTF8))), "Aiming - y axis"));
    }

    for(int i=bi_select; i<BI_MAX; i++)
    {
        old_buttons[i] = buttons[i];
    }

    for(int i=ai_ls_up; i<AI_MAX; i++)
    {
        old_axes[i] = axes[i];
    }

    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(_("Can't save ") + wxString(configFile.GetFilePath().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
}

void fpsconfigFrame::LoadConfig()
{
  std::list<ControlMapper>* ButtonMappers[2];
  std::list<ControlMapper>* AxisMappers[2];
  std::list<MouseOptions>* mouseOptions;
  e_button_index bindex;
  e_axis_index aindex;
  wxButton* button;
  bool warn = false;

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
  defaultMouseId = "0";
  defaultKeyboardName = "";
  defaultKeyboardId = "0";

  /*
   * Load Hip Fire config.
   */
  //Load ButtonMappers
  for(int i=bi_select; i<BI_MAX; i++)
  {
      buttons[i] = ControlMapper();
      old_buttons[i] = ControlMapper();
      button = getButtonButton(button_labels[i]);
      if(button != NULL)
      {
          button->SetLabel(wxEmptyString);
          button->UnsetToolTip();
      }
  }
  ButtonMappers[0] = configFile.GetController(0)->GetConfiguration(0)->GetButtonMapperList();
  ButtonMappers[1] = configFile.GetController(0)->GetConfiguration(1)->GetButtonMapperList();
  if(ButtonMappers[0]->size() != ButtonMappers[1]->size())
  {
    warn = true;
  }
  else
  {
    for(std::list<ControlMapper>::iterator it1 = ButtonMappers[0]->begin(); it1!=ButtonMappers[0]->end(); ++it1)
    {
      std::list<ControlMapper>::iterator it2;
      for(it2 = ButtonMappers[1]->begin(); it2!=ButtonMappers[1]->end(); ++it2)
      {
        if(*it1 == *it2)
        {
          break;
        }
      }
      if(it2 == ButtonMappers[1]->end())
      {
        warn = true;
        break;
      }
    }
  }
  for(std::list<ControlMapper>::iterator it = ButtonMappers[0]->begin(); it!=ButtonMappers[0]->end(); ++it)
  {
      if(it->GetDevice()->GetType() == "joystick")
      {
        continue;
      }

      button = getButtonButton(it->GetGenericAxisName());

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

      if(button->GetLabel().empty())
      {
        buttons[bindex] = *it;
        old_buttons[bindex] = *it;
        button->SetLabel(wxString(it->GetEvent()->GetId().c_str(), wxConvUTF8));
        string tt(it->GetEvent()->GetId());
        wxButton bt;
        wxColour colour = bt.GetBackgroundColour();
        if(!it->GetLabel().empty())
        {
            tt.append(" [");
            tt.append(it->GetLabel());
            tt.append("]");
            if(tt.find(", not found") != string::npos  || tt.find(", duplicate") != string::npos)
            {
              colour = wxColour (255, 0, 0);
            }
        }
        button->SetBackgroundColour(colour);
        button->SetToolTip(wxString(tt.c_str(), wxConvUTF8));
      }

      if(!it->GetDevice()->GetName().empty())
      {
          if(it->GetDevice()->GetType() == "mouse")
          {
              if(defaultMouseName.empty())
              {
                defaultMouseName = it->GetDevice()->GetName();
                defaultMouseId = it->GetDevice()->GetId();
              }
          }
          else if(it->GetDevice()->GetType() == "keyboard")
          {
              if(defaultKeyboardName.empty())
              {
                defaultKeyboardName = it->GetDevice()->GetName();
                defaultKeyboardId = it->GetDevice()->GetId();
              }
          }
      }
  }
  //Load AxisMappers
  for(int i=ai_ls_up; i<AI_MAX; i++)
  {
      axes[i] = ControlMapper();
      old_axes[i] = ControlMapper();
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
  AxisMappers[0] = configFile.GetController(0)->GetConfiguration(0)->GetAxisMapperList();
  AxisMappers[1] = configFile.GetController(0)->GetConfiguration(1)->GetAxisMapperList();
  if(AxisMappers[0]->size() != AxisMappers[1]->size())
  {
    warn = true;
  }
  else
  {
    for(std::list<ControlMapper>::iterator it1 = AxisMappers[0]->begin(); it1!=AxisMappers[0]->end(); ++it1)
    {
      if(it1->GetEvent()->GetType() != "button")
      {
        continue;
      }
      std::list<ControlMapper>::iterator it2;
      for(it2 = AxisMappers[1]->begin(); it2!=AxisMappers[1]->end(); ++it2)
      {
        if(*it1 == *it2)
        {
          break;
        }
      }
      if(it2 == AxisMappers[1]->end())
      {
        warn = true;
        break;
      }
    }
  }
  for(std::list<ControlMapper>::iterator it = AxisMappers[0]->begin(); it!=AxisMappers[0]->end(); ++it)
  {
      if(it->GetDevice()->GetType() == "joystick")
      {
        continue;
      }

      button = getAxisButton(it->GetGenericAxisName());

      if(button == NULL)
      {
          if(it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_2")))
          {
              SpinCtrlDeadZoneHipFire->SetValue(wxAtoi(wxString(it->GetEvent()->GetDeadZone().c_str(), wxConvUTF8)));
              ChoiceDeadZoneShapeHipFire->SetStringSelection(wxString(gettext(it->GetEvent()->GetShape().c_str()), wxConvUTF8));

              mouseOptions = configFile.GetController(0)->GetConfiguration(0)->GetMouseOptionsList();
              for(std::list<MouseOptions>::iterator it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
              {
                if(it2->GetMouse()->GetName() == it->GetDevice()->GetName()
                   && it2->GetMouse()->GetId() == it->GetDevice()->GetId())
                {
                  SpinCtrlBufferSizeHipFire->SetValue(wxAtoi(wxString(it2->GetBufferSize().c_str(), wxConvUTF8)));

                  wsf = wxString(it2->GetFilter().c_str(), wxConvUTF8);

                  if(wsf.ToDouble(&f))
                  {
                    TextCtrlFilterHipFire->SetValue(wsf);
                    SpinCtrlFilterHipFire->SetValue(f*100);
                  }
                  break;
                }
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
          else if(it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_3")))
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
              old_axes[aindex] = *it;
              button->SetLabel(wxString(it->GetEvent()->GetId().c_str(), wxConvUTF8));
              string tt(it->GetEvent()->GetId());
              wxButton bt;
              wxColour colour = bt.GetBackgroundColour();
              if(!it->GetLabel().empty())
              {
                  tt.append(" [");
                  tt.append(it->GetLabel());
                  tt.append("]");
                  if(tt.find(", not found") != string::npos || tt.find(", duplicate") != string::npos)
                  {
                    colour = wxColour (255, 0, 0);
                  }
              }
              button->SetBackgroundColour(colour);
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
  for(std::list<ControlMapper>::iterator it = AxisMappers[1]->begin(); it!=AxisMappers[1]->end(); ++it)
  {
      if(it->GetDevice()->GetType() == "joystick")
      {
        continue;
      }

      button = getAxisButton(it->GetGenericAxisName());

      if(button == NULL)
      {
          if(it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_2")))
          {
              SpinCtrlDeadZoneADS->SetValue(wxAtoi(wxString(it->GetEvent()->GetDeadZone().c_str(), wxConvUTF8)));
              ChoiceDeadZoneShapeADS->SetStringSelection(wxString(gettext(it->GetEvent()->GetShape().c_str()), wxConvUTF8));

              mouseOptions = configFile.GetController(0)->GetConfiguration(1)->GetMouseOptionsList();
              for(std::list<MouseOptions>::iterator it2 = mouseOptions->begin(); it2!=mouseOptions->end(); ++it2)
              {
                if(it2->GetMouse()->GetName() == it->GetDevice()->GetName()
                   && it2->GetMouse()->GetId() == it->GetDevice()->GetId())
                {
                  SpinCtrlBufferSizeADS->SetValue(wxAtoi(wxString(it2->GetBufferSize().c_str(), wxConvUTF8)));

                  wsf = wxString(it2->GetFilter().c_str(), wxConvUTF8);

                  if(wsf.ToDouble(&f))
                  {
                    TextCtrlFilterADS->SetValue(wsf);
                    SpinCtrlFilterADS->SetValue(f*100);
                  }
                  break;
                }
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
          else if(it->CompareAxisProps(ControlMapper::GetAxisProps("rel_axis_3")))
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

  if(warn)
  {
    wxMessageBox(_("Profiles 1 & 2 do not contain the same bindings.\nEditing this conf with gimx-fpsconfig can mess the bindings."), _("Info"), wxICON_INFORMATION);
  }

  MenuItemSave->Enable(true);
}

void fpsconfigFrame::OnMenuOpen(wxCommandEvent& event)
{
    FileDialog1->SetDirectory(default_directory);

    if ( FileDialog1->ShowModal() != wxID_OK ) return;

    wxString FileName = FileDialog1->GetPath();
    if ( FileName.IsEmpty() ) return;

    configFile.ReadConfigFile(string(FileName.mb_str(wxConvUTF8)));

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

    if(str.Replace(wxT(","), wxT(".")))
    {
        text->SetValue(str);
    }

    if(!str.ToDouble(&value))
    {
        if (text == TextCtrlFilterHipFire || text == TextCtrlFilterADS)
        {
            text->SetValue(wxT("0.00"));
        }
        else
        {
            text->SetValue(wxT("1.00"));
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
                text->SetValue(wxT("100.00"));
            }
            else if(value < -100)
            {
                value = -100;
                ivalue = value * 100;
                text->SetValue(wxT("-100.00"));
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
                text->SetValue(wxT("100.00"));
            }
            else if(value < -100)
            {
                value = -100;
                ivalue = value * 100;
                text->SetValue(wxT("-100.00"));
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
                text->SetValue(wxT("2.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(wxT("0.00"));
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
                text->SetValue(wxT("2.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(wxT("0.00"));
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
                text->SetValue(wxT("1.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(wxT("0.00"));
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
                text->SetValue(wxT("1.00"));
            }
            else if(value < 0)
            {
                value = 0;
                ivalue = value * 100;
                text->SetValue(wxT("0.00"));
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

  updater* u = updater::getInstance();
  u->SetParams(VERSION_URL, VERSION_FILE, INFO_VERSION, DOWNLOAD_URL, DOWNLOAD_FILE);

  ret = u->CheckVersion();

  if (ret > 0)
  {
    int answer = wxMessageBox(_("Update available.\nStart installation?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    wxBusyInfo wait(_("Downloading update..."));
    if (u->Update() < 0)
    {
      wxMessageBox(_("Can't retrieve update file!"), _("Error"), wxICON_ERROR);
    }
    else
    {
      exit(0);
    }
  }
  else if (ret < 0)
  {
    wxMessageBox(_("Can't check version!"), _("Error"), wxICON_ERROR);
  }
  else
  {
    wxMessageBox(_("GIMX is up-to-date!"), _("Info"), wxICON_INFORMATION);
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

  wxNumberEntryDialog dialog(this, wxT(""), _("Enter a number:"), _("Destination mouse DPI"), current_dpi, 100, MAX_DPI);

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
    wxMessageBox( _("No config opened!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxFileDialog FileDialog(this, _("Select the reference config."), default_directory, wxEmptyString, _("XML files (*.xml)|*.xml"), wxFD_DEFAULT_STYLE);

  if ( FileDialog.ShowModal() != wxID_OK ) return;

  wxString FileName = FileDialog.GetPath();
  if ( FileName.IsEmpty() ) return;

  if(configFile.AutoBind(string(FileName.mb_str(wxConvUTF8))) < 0)
  {
    wxMessageBox(_("Can't auto-bind controls!"), _("Error"), wxICON_ERROR);
  }
  else
  {
    LoadConfig();
    wxMessageBox(_("Auto-bind done!"), _("Info"), wxICON_INFORMATION);
  }
}
// comparison, not case sensitive.
bool compare_nocase (string first, string second)
{
  unsigned int i=0;
  while ( (i<first.length()) && (i<second.length()) )
  {
    if (tolower(first[i])<tolower(second[i])) return true;
    else if (tolower(first[i])>tolower(second[i])) return false;
    ++i;
  }
  if (first.length()<second.length()) return true;
  else return false;
}

void fpsconfigFrame::readLabels()
{
  wxDir dir(default_directory);
  list<string> blabels;
  list<string> alabels;

  if(!dir.IsOpened())
  {
    cout << "Warning: can't open " << string(default_directory.mb_str(wxConvUTF8)) << endl;
    return;
  }

  wxString file;
  wxString filepath;
  wxString filespec = wxT("*.xml");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    filepath = default_directory + file;
    ConfigurationFile::GetLabels(string(filepath.mb_str(wxConvUTF8)), blabels, alabels);
  }

  blabels.sort(compare_nocase);
  alabels.sort(compare_nocase);

  for(list<string>::iterator it = blabels.begin(); it != blabels.end(); ++it)
  {
    b_labels.Add(wxString(it->c_str(), wxConvUTF8));
  }

  for(list<string>::iterator it = alabels.begin(); it != alabels.end(); ++it)
  {
    a_labels.Add(wxString(it->c_str(), wxConvUTF8));
  }
}

void fpsconfigFrame::OnMenuOpenConfigDirectory(wxCommandEvent& event)
{
#ifdef WIN32
  default_directory.Replace(wxT("/"), wxT("\\"));
  wxExecute(wxT("explorer ") + default_directory, wxEXEC_ASYNC, NULL);
#else
  wxExecute(wxT("xdg-open ") + default_directory, wxEXEC_ASYNC, NULL);
#endif
}

void fpsconfigFrame::OnMenuItemWindowEventsSelected(wxCommandEvent& event)
{
    if(MenuItemWindowEvents->IsChecked())
    {
      evcatch->SetWindowEvents(true);
    }
    else
    {
      evcatch->SetWindowEvents(false);
    }
}

pair<Device, Event> fpsconfigFrame::selectEvent()
{
    if (evcatch->GetEvents()->size() == 1)
    {
        return (*evcatch->GetEvents())[0];
    }

    wxArrayString choices;

    for(vector<pair<Device, Event> >::iterator it = evcatch->GetEvents()->begin(); it != evcatch->GetEvents()->end(); ++it)
    {
        ostringstream ios;
        ios << it->second.GetType() << " " << it->second.GetId();
        choices.Add(wxString(ios.str().c_str(), wxConvUTF8));
    }

    wxSingleChoiceDialog dialog(this, _("Select the event."), _("Events"), choices);

    if (dialog.ShowModal() == wxID_OK)
    {
        int selection = dialog.GetSelection();

        if (selection >= 0 && (size_t)selection < evcatch->GetEvents()->size())
        {
            return (*evcatch->GetEvents())[selection];
        }
    }

    return make_pair(Device(), Event());
}
