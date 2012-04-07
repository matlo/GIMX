/***************************************************************
 * Name:      sixaxis_emu_guiMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2010-11-09
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixaxis_emu_guiMain.h"
#include <wx/msgdlg.h>
#include <stdio.h>
#include <sys/types.h>
#include <SDL/SDL.h>
#include "wx/numdlg.h"
#include <math.h>

//(*InternalHeaders(sixaxis_emu_guiFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/aboutdlg.h>
#include "config.h"
#include <locale.h>
#include <wx/filename.h>
#include <wx/dir.h>

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

//(*IdInit(sixaxis_emu_guiFrame)
const long sixaxis_emu_guiFrame::ID_STATICTEXT35 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT27 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT36 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT37 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHECKBOX1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT28 = wxNewId();
const long sixaxis_emu_guiFrame::ID_SPINCTRL5 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON10 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT58 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT59 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT60 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT61 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON13 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON14 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT24 = wxNewId();
const long sixaxis_emu_guiFrame::ID_SPINCTRL3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT62 = wxNewId();
const long sixaxis_emu_guiFrame::ID_SPINCTRL1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT67 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT68 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT69 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT70 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON15 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON16 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT19 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT48 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT49 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT50 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT51 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON11 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON12 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT26 = wxNewId();
const long sixaxis_emu_guiFrame::ID_SPINCTRL4 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT25 = wxNewId();
const long sixaxis_emu_guiFrame::ID_SPINCTRL2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT9 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON17 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON18 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT23 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_PANEL1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT4 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT29 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT10 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT5 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT6 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT8 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT20 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT7 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT38 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT30 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT39 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE4 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT40 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON8 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE5 = wxNewId();
const long sixaxis_emu_guiFrame::ID_GRID1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON4 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON6 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_PANEL2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT11 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT31 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT12 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT13 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT14 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT21 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT16 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT15 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT17 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT18 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT22 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT33 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT41 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT32 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT42 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE7 = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATICTEXT43 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON9 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE8 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL8 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL9 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL10 = wxNewId();
const long sixaxis_emu_guiFrame::ID_CHOICE1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_TEXTCTRL2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_GRID2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON7 = wxNewId();
const long sixaxis_emu_guiFrame::ID_BUTTON5 = wxNewId();
const long sixaxis_emu_guiFrame::ID_PANEL3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_NOTEBOOK1 = wxNewId();
const long sixaxis_emu_guiFrame::idMenuNew = wxNewId();
const long sixaxis_emu_guiFrame::idMenuOpen = wxNewId();
const long sixaxis_emu_guiFrame::idMenuSave = wxNewId();
const long sixaxis_emu_guiFrame::idMenuSaveAs = wxNewId();
const long sixaxis_emu_guiFrame::idMenuQuit = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM12 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM18 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM17 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM19 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM23 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM20 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM22 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM21 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM1 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM2 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM3 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM4 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM5 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM6 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM7 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM8 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM9 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM10 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM11 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM13 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM14 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM15 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM16 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM24 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM25 = wxNewId();
const long sixaxis_emu_guiFrame::ID_MENUITEM26 = wxNewId();
const long sixaxis_emu_guiFrame::idMenuAbout = wxNewId();
const long sixaxis_emu_guiFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(sixaxis_emu_guiFrame,wxFrame)
    //(*EventTable(sixaxis_emu_guiFrame)
    //*)
END_EVENT_TABLE()


void sixaxis_emu_guiFrame::fillButtonAxisChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(_("rstick left"));
    choice->Append(_("rstick right"));
    choice->Append(_("rstick up"));
    choice->Append(_("rstick down"));
    choice->Append(_("lstick left"));
    choice->Append(_("lstick right"));
    choice->Append(_("lstick up"));
    choice->Append(_("lstick down"));
    choice->Append(_("up"));
    choice->Append(_("down"));
    choice->Append(_("right"));
    choice->Append(_("left"));
    choice->Append(_("r1"));
    choice->Append(_("r2"));
    choice->Append(_("l1"));
    choice->Append(_("l2"));
    choice->Append(_("circle"));
    choice->Append(_("square"));
    choice->Append(_("cross"));
    choice->Append(_("triangle"));
    choice->SetSelection(choice->FindString(previous));
}

void sixaxis_emu_guiFrame::fillAxisAxisChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(_("rstick x"));
    choice->Append(_("rstick y"));
    choice->Append(_("lstick x"));
    choice->Append(_("lstick y"));
    choice->Append(_("up"));
    choice->Append(_("down"));
    choice->Append(_("right"));
    choice->Append(_("left"));
    choice->Append(_("r1"));
    choice->Append(_("r2"));
    choice->Append(_("l1"));
    choice->Append(_("l2"));
    choice->Append(_("circle"));
    choice->Append(_("square"));
    choice->Append(_("cross"));
    choice->Append(_("triangle"));
    choice->SetSelection(choice->FindString(previous));
}

void sixaxis_emu_guiFrame::fillButtonChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(_("up"));
    choice->Append(_("down"));
    choice->Append(_("right"));
    choice->Append(_("left"));
    choice->Append(_("r1"));
    choice->Append(_("r2"));
    choice->Append(_("r3"));
    choice->Append(_("l1"));
    choice->Append(_("l2"));
    choice->Append(_("l3"));
    choice->Append(_("circle"));
    choice->Append(_("square"));
    choice->Append(_("cross"));
    choice->Append(_("triangle"));
    choice->Append(_("start"));
    choice->Append(_("select"));
    choice->Append(_("PS"));
    choice->SetSelection(choice->FindString(previous));
}

sixaxis_emu_guiFrame::sixaxis_emu_guiFrame(wxString file,wxWindow* parent,wxWindowID id)
{
    //(*Initialize(sixaxis_emu_guiFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer16;
    wxMenuItem* MenuItem2;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer7;
    wxStaticBoxSizer* StaticBoxSizer8;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxGridSizer* GridSizer1;
    wxStaticBoxSizer* StaticBoxSizer6;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxMenuItem* MenuItem6;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer12;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer17;
    wxMenu* Menu2;
    wxStaticBoxSizer* StaticBoxSizer5;
    
    Create(parent, wxID_ANY, _("Gimx-config"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    GridSizer1 = new wxGridSizer(1, 1, 0, 0);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxSize(-1,570), 0, _T("ID_NOTEBOOK1"));
    Panel1 = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer10 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Profile trigger"));
    FlexGridSizer13 = new wxFlexGridSizer(1, 9, 0, 0);
    ProfileTriggerDeviceType = new wxStaticText(Panel1, ID_STATICTEXT35, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer13->Add(ProfileTriggerDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceName = new wxStaticText(Panel1, ID_STATICTEXT27, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer13->Add(ProfileTriggerDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceId = new wxStaticText(Panel1, ID_STATICTEXT36, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer13->Add(ProfileTriggerDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerButtonId = new wxStaticText(Panel1, ID_STATICTEXT37, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer13->Add(ProfileTriggerButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(Panel1, ID_BUTTON1, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer13->Add(Button1, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    CheckBox1 = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Switch back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox1->SetValue(false);
    FlexGridSizer13->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName1 = new wxStaticText(Panel1, ID_STATICTEXT28, _("Delay (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer13->Add(VarName1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDelay = new wxSpinCtrl(Panel1, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(65,-1), 0, 0, 1000, 0, _T("ID_SPINCTRL5"));
    ProfileTriggerDelay->SetValue(_T("0"));
    FlexGridSizer13->Add(ProfileTriggerDelay, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button10 = new wxButton(Panel1, ID_BUTTON10, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    FlexGridSizer13->Add(Button10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer13, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Stick intensity"));
    FlexGridSizer18 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Left"));
    FlexGridSizer12 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Increase"));
    FlexGridSizer14 = new wxFlexGridSizer(2, 6, 0, 0);
    LSIncDeviceType = new wxStaticText(Panel1, ID_STATICTEXT58, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer14->Add(LSIncDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncDeviceName = new wxStaticText(Panel1, ID_STATICTEXT59, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer14->Add(LSIncDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncDeviceId = new wxStaticText(Panel1, ID_STATICTEXT60, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer14->Add(LSIncDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncButtonId = new wxStaticText(Panel1, ID_STATICTEXT61, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer14->Add(LSIncButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncAutoDetect = new wxButton(Panel1, ID_BUTTON13, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    FlexGridSizer14->Add(LSIncAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    LSIncDelete = new wxButton(Panel1, ID_BUTTON14, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON14"));
    FlexGridSizer14->Add(LSIncDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName2 = new wxStaticText(Panel1, ID_STATICTEXT24, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer12->Add(VarName2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDeadzone = new wxSpinCtrl(Panel1, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL3"));
    LSDeadzone->SetValue(_T("0"));
    FlexGridSizer12->Add(LSDeadzone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName3 = new wxStaticText(Panel1, ID_STATICTEXT62, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer12->Add(VarName3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSSteps = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL1"));
    LSSteps->SetValue(_T("1"));
    FlexGridSizer12->Add(LSSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Decrease"));
    FlexGridSizer15 = new wxFlexGridSizer(1, 6, 0, 0);
    LSDecDeviceType = new wxStaticText(Panel1, ID_STATICTEXT67, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT67"));
    FlexGridSizer15->Add(LSDecDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecDeviceName = new wxStaticText(Panel1, ID_STATICTEXT68, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT68"));
    FlexGridSizer15->Add(LSDecDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecDeviceId = new wxStaticText(Panel1, ID_STATICTEXT69, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT69"));
    FlexGridSizer15->Add(LSDecDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecButtonId = new wxStaticText(Panel1, ID_STATICTEXT70, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT70"));
    FlexGridSizer15->Add(LSDecButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecAutoDetect = new wxButton(Panel1, ID_BUTTON15, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON15"));
    FlexGridSizer15->Add(LSDecAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    LSDecDelete = new wxButton(Panel1, ID_BUTTON16, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON16"));
    FlexGridSizer15->Add(LSDecDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName4 = new wxStaticText(Panel1, ID_STATICTEXT19, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer12->Add(VarName4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSShape = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    LSShape->SetSelection( LSShape->Append(_("Circle")) );
    LSShape->Append(_("Rectangle"));
    FlexGridSizer12->Add(LSShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18->Add(StaticBoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Right"));
    FlexGridSizer16 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticBoxSizer7 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Increase"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 6, 0, 0);
    RSIncDeviceType = new wxStaticText(Panel1, ID_STATICTEXT48, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer2->Add(RSIncDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncDeviceName = new wxStaticText(Panel1, ID_STATICTEXT49, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer2->Add(RSIncDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncDeviceId = new wxStaticText(Panel1, ID_STATICTEXT50, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    FlexGridSizer2->Add(RSIncDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncButtonId = new wxStaticText(Panel1, ID_STATICTEXT51, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer2->Add(RSIncButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncAutoDetect = new wxButton(Panel1, ID_BUTTON11, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer2->Add(RSIncAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    RSIncDelete = new wxButton(Panel1, ID_BUTTON12, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer2->Add(RSIncDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName5 = new wxStaticText(Panel1, ID_STATICTEXT26, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer16->Add(VarName5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDeadZone = new wxSpinCtrl(Panel1, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL4"));
    RSDeadZone->SetValue(_T("0"));
    FlexGridSizer16->Add(RSDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName6 = new wxStaticText(Panel1, ID_STATICTEXT25, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer16->Add(VarName6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSSteps = new wxSpinCtrl(Panel1, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL2"));
    RSSteps->SetValue(_T("1"));
    FlexGridSizer16->Add(RSSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Decrease"));
    FlexGridSizer17 = new wxFlexGridSizer(1, 6, 0, 0);
    RSDecDeviceType = new wxStaticText(Panel1, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer17->Add(RSDecDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecDeviceName = new wxStaticText(Panel1, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer17->Add(RSDecDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecDeviceId = new wxStaticText(Panel1, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer17->Add(RSDecDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecButtonId = new wxStaticText(Panel1, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer17->Add(RSDecButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecAutoDetect = new wxButton(Panel1, ID_BUTTON17, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON17"));
    FlexGridSizer17->Add(RSDecAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    RSDecDelete = new wxButton(Panel1, ID_BUTTON18, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON18"));
    FlexGridSizer17->Add(RSDecDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName7 = new wxStaticText(Panel1, ID_STATICTEXT23, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer16->Add(VarName7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSShape = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    RSShape->SetSelection( RSShape->Append(_("Circle")) );
    RSShape->Append(_("Rectangle"));
    FlexGridSizer16->Add(RSShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18->Add(StaticBoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer10);
    FlexGridSizer10->Fit(Panel1);
    FlexGridSizer10->SetSizeHints(Panel1);
    Panel2 = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer9->AddGrowableRow(1);
    FlexGridSizer1 = new wxFlexGridSizer(2, 8, 0, 0);
    VarName8 = new wxStaticText(Panel2, ID_STATICTEXT4, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(VarName8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName9 = new wxStaticText(Panel2, ID_STATICTEXT29, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    FlexGridSizer1->Add(VarName9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName10 = new wxStaticText(Panel2, ID_STATICTEXT10, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer1->Add(VarName10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName11 = new wxStaticText(Panel2, ID_STATICTEXT5, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(VarName11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName12 = new wxStaticText(Panel2, ID_STATICTEXT6, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer1->Add(VarName12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName14 = new wxStaticText(Panel2, ID_STATICTEXT8, _("Threshold"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer1->Add(VarName14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName15 = new wxStaticText(Panel2, ID_STATICTEXT20, wxEmptyString, wxDefaultPosition, wxSize(31,21), 0, _T("ID_STATICTEXT20"));
    FlexGridSizer1->Add(VarName15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName13 = new wxStaticText(Panel2, ID_STATICTEXT7, _("Button"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer1->Add(VarName13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceType = new wxStaticText(Panel2, ID_STATICTEXT38, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer1->Add(ButtonTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceName = new wxStaticText(Panel2, ID_STATICTEXT30, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer1->Add(ButtonTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceId = new wxStaticText(Panel2, ID_STATICTEXT39, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer1->Add(ButtonTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabEventType = new wxChoice(Panel2, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    ButtonTabEventType->SetSelection( ButtonTabEventType->Append(_("button")) );
    ButtonTabEventType->Append(_("axis up"));
    ButtonTabEventType->Append(_("axis down"));
    FlexGridSizer1->Add(ButtonTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabEventId = new wxStaticText(Panel2, ID_STATICTEXT40, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer1->Add(ButtonTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabThreshold = new wxTextCtrl(Panel2, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    ButtonTabThreshold->Disable();
    FlexGridSizer1->Add(ButtonTabThreshold, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabAutoDetect = new wxButton(Panel2, ID_BUTTON8, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer1->Add(ButtonTabAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    ButtonTabButtonId = new wxChoice(Panel2, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    FlexGridSizer1->Add(ButtonTabButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    ButtonTabControls = new wxGrid(Panel2, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
    ButtonTabControls->CreateGrid(0,7);
    ButtonTabControls->EnableEditing(false);
    ButtonTabControls->EnableGridLines(true);
    ButtonTabControls->SetRowLabelSize(25);
    ButtonTabControls->SetColLabelValue(0, _("Device type"));
    ButtonTabControls->SetColLabelValue(1, _("Device name"));
    ButtonTabControls->SetColLabelValue(2, _("Device id"));
    ButtonTabControls->SetColLabelValue(3, _("Event type"));
    ButtonTabControls->SetColLabelValue(4, _("Event id"));
    ButtonTabControls->SetColLabelValue(5, _("Threshold"));
    ButtonTabControls->SetColLabelValue(6, _("Button id"));
    ButtonTabControls->SetDefaultCellFont( ButtonTabControls->GetFont() );
    ButtonTabControls->SetDefaultCellTextColour( ButtonTabControls->GetForegroundColour() );
    FlexGridSizer8->Add(ButtonTabControls, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(2, 1, 0, 0);
    ButtonTabAdd = new wxButton(Panel2, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer6->Add(ButtonTabAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabRemove = new wxButton(Panel2, ID_BUTTON6, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer6->Add(ButtonTabRemove, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabModify = new wxButton(Panel2, ID_BUTTON2, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer6->Add(ButtonTabModify, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer9);
    FlexGridSizer9->Fit(Panel2);
    FlexGridSizer9->SetSizeHints(Panel2);
    Panel3 = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer3 = new wxFlexGridSizer(2, 12, 0, 0);
    VarName16 = new wxStaticText(Panel3, ID_STATICTEXT11, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer3->Add(VarName16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(Panel3, ID_STATICTEXT31, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName17 = new wxStaticText(Panel3, ID_STATICTEXT12, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer3->Add(VarName17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName18 = new wxStaticText(Panel3, ID_STATICTEXT13, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer3->Add(VarName18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName19 = new wxStaticText(Panel3, ID_STATICTEXT14, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer3->Add(VarName19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName24 = new wxStaticText(Panel3, ID_STATICTEXT21, wxEmptyString, wxDefaultPosition, wxSize(30,17), 0, _T("ID_STATICTEXT21"));
    FlexGridSizer3->Add(VarName24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName21 = new wxStaticText(Panel3, ID_STATICTEXT16, _("Axis"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer3->Add(VarName21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName20 = new wxStaticText(Panel3, ID_STATICTEXT15, _("DZ"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    VarName20->SetToolTip(_("Dead zone"));
    FlexGridSizer3->Add(VarName20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName22 = new wxStaticText(Panel3, ID_STATICTEXT17, _("Sens."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    VarName22->SetToolTip(_("Sensitivity"));
    FlexGridSizer3->Add(VarName22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName23 = new wxStaticText(Panel3, ID_STATICTEXT18, _("Accel."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    VarName23->SetToolTip(_("Acceleration"));
    FlexGridSizer3->Add(VarName23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName25 = new wxStaticText(Panel3, ID_STATICTEXT22, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer3->Add(VarName25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VarName26 = new wxStaticText(Panel3, ID_STATICTEXT33, _("Smoothing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer3->Add(VarName26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceType = new wxStaticText(Panel3, ID_STATICTEXT41, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer3->Add(AxisTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceName = new wxStaticText(Panel3, ID_STATICTEXT32, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer3->Add(AxisTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceId = new wxStaticText(Panel3, ID_STATICTEXT42, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer3->Add(AxisTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabEventType = new wxChoice(Panel3, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    AxisTabEventType->SetSelection( AxisTabEventType->Append(_("axis")) );
    AxisTabEventType->Append(_("button"));
    FlexGridSizer3->Add(AxisTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabEventId = new wxStaticText(Panel3, ID_STATICTEXT43, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer3->Add(AxisTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAutoDetect = new wxButton(Panel3, ID_BUTTON9, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer3->Add(AxisTabAutoDetect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAxisId = new wxChoice(Panel3, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE8"));
    FlexGridSizer3->Add(AxisTabAxisId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeadZone = new wxTextCtrl(Panel3, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    AxisTabDeadZone->Disable();
    AxisTabDeadZone->SetToolTip(_("Dead zone [0..64]"));
    FlexGridSizer3->Add(AxisTabDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabSensitivity = new wxTextCtrl(Panel3, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(59,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    AxisTabSensitivity->Disable();
    AxisTabSensitivity->SetToolTip(_("Sensitivity [-100.00..100.00]"));
    FlexGridSizer3->Add(AxisTabSensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAcceleration = new wxTextCtrl(Panel3, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(55,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
    AxisTabAcceleration->Disable();
    AxisTabAcceleration->SetToolTip(_("Acceleration [0.00..2.00]"));
    FlexGridSizer3->Add(AxisTabAcceleration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabShape = new wxChoice(Panel3, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    AxisTabShape->SetSelection( AxisTabShape->Append(wxEmptyString) );
    AxisTabShape->Append(_("Circle"));
    AxisTabShape->Append(_("Rectangle"));
    AxisTabShape->Disable();
    FlexGridSizer3->Add(AxisTabShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(1, 2, 0, 0);
    AxisTabBufferSize = new wxTextCtrl(Panel3, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    AxisTabBufferSize->Disable();
    AxisTabBufferSize->SetToolTip(_("Buffer size [1..30]"));
    FlexGridSizer11->Add(AxisTabBufferSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabFilter = new wxTextCtrl(Panel3, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(41,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    AxisTabFilter->Disable();
    AxisTabFilter->SetToolTip(_("Filter [0.00..1.00]"));
    FlexGridSizer11->Add(AxisTabFilter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(1, 2, 0, 0);
    Grid2 = new wxGrid(Panel3, ID_GRID2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID2"));
    Grid2->CreateGrid(0,12);
    Grid2->EnableEditing(false);
    Grid2->EnableGridLines(true);
    Grid2->SetRowLabelSize(25);
    Grid2->SetDefaultColSize(100, true);
    Grid2->SetColLabelValue(0, _("Device type"));
    Grid2->SetColLabelValue(1, _("Device name"));
    Grid2->SetColLabelValue(2, _("Device id"));
    Grid2->SetColLabelValue(3, _("Event type"));
    Grid2->SetColLabelValue(4, _("Event id"));
    Grid2->SetColLabelValue(5, _("Axis id"));
    Grid2->SetColLabelValue(6, _("Dead zone"));
    Grid2->SetColLabelValue(7, _("Multiplier"));
    Grid2->SetColLabelValue(8, _("Exponent"));
    Grid2->SetColLabelValue(9, _("Shape"));
    Grid2->SetColLabelValue(10, _("Buffer Size"));
    Grid2->SetColLabelValue(11, _("Filter"));
    Grid2->SetDefaultCellFont( Grid2->GetFont() );
    Grid2->SetDefaultCellTextColour( Grid2->GetForegroundColour() );
    FlexGridSizer4->Add(Grid2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer7 = new wxFlexGridSizer(2, 1, 0, 0);
    Button3 = new wxButton(Panel3, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button7 = new wxButton(Panel3, ID_BUTTON7, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer7->Add(Button7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button5 = new wxButton(Panel3, ID_BUTTON5, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer7->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel3->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel3);
    FlexGridSizer5->SetSizeHints(Panel3);
    Notebook1->AddPage(Panel1, _("Overall"), false);
    Notebook1->AddPage(Panel2, _("Button"), true);
    Notebook1->AddPage(Panel3, _("Axis"), false);
    GridSizer1->Add(Notebook1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(GridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu1, idMenuNew, _("New\tCtrl-N"), _("Create a new configuration"), wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem6 = new wxMenuItem(Menu1, idMenuOpen, _("Open\tCtrl-O"), _("Open a configuration file"), wxITEM_NORMAL);
    Menu1->Append(MenuItem6);
    MenuItem4 = new wxMenuItem(Menu1, idMenuSave, _("Save\tCtrl-S"), _("Save configuration"), wxITEM_NORMAL);
    Menu1->Append(MenuItem4);
    MenuItem4->Enable(false);
    MenuItem5 = new wxMenuItem(Menu1, idMenuSaveAs, _("Save As..."), _("Save configuration to specified file"), wxITEM_NORMAL);
    Menu1->Append(MenuItem5);
    Menu1->AppendSeparator();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu5 = new wxMenu();
    MenuItem18 = new wxMenuItem(Menu5, ID_MENUITEM12, _("Copy Profile"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem18);
    MenuItem24 = new wxMenuItem(Menu5, ID_MENUITEM18, _("Copy Controller"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem24);
    Menu5->AppendSeparator();
    MenuItem23 = new wxMenuItem(Menu5, ID_MENUITEM17, _("Paste Profile"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem23);
    MenuItem25 = new wxMenuItem(Menu5, ID_MENUITEM19, _("Paste Controller"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem25);
    Menu5->AppendSeparator();
    MenuItem29 = new wxMenuItem(Menu5, ID_MENUITEM23, _("Set Mouse DPI"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem29);
    Menu5->AppendSeparator();
    MenuItem26 = new wxMenuItem(Menu5, ID_MENUITEM20, _("Replace Mouse"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem26);
    MenuItem26->Enable(false);
    MenuItem28 = new wxMenuItem(Menu5, ID_MENUITEM22, _("Replace Mouse DPI"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem28);
    MenuItem27 = new wxMenuItem(Menu5, ID_MENUITEM21, _("Replace Keyboard"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem27);
    MenuItem27->Enable(false);
    MenuBar1->Append(Menu5, _("Edit"));
    Menu3 = new wxMenu();
    MenuItem7 = new wxMenuItem(Menu3, ID_MENUITEM1, _("1"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem7);
    MenuItem8 = new wxMenuItem(Menu3, ID_MENUITEM2, _("2"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem8);
    MenuItem9 = new wxMenuItem(Menu3, ID_MENUITEM3, _("3"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem9);
    MenuItem10 = new wxMenuItem(Menu3, ID_MENUITEM4, _("4"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem10);
    MenuItem11 = new wxMenuItem(Menu3, ID_MENUITEM5, _("5"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem11);
    MenuItem12 = new wxMenuItem(Menu3, ID_MENUITEM6, _("6"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem12);
    MenuItem13 = new wxMenuItem(Menu3, ID_MENUITEM7, _("7"), wxEmptyString, wxITEM_RADIO);
    Menu3->Append(MenuItem13);
    MenuBar1->Append(Menu3, _("Controller"));
    Menu4 = new wxMenu();
    MenuItem14 = new wxMenuItem(Menu4, ID_MENUITEM8, _("1"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem14);
    MenuItem15 = new wxMenuItem(Menu4, ID_MENUITEM9, _("2"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem15);
    MenuItem17 = new wxMenuItem(Menu4, ID_MENUITEM10, _("3"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem17);
    MenuItem16 = new wxMenuItem(Menu4, ID_MENUITEM11, _("4"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem16);
    MenuItem19 = new wxMenuItem(Menu4, ID_MENUITEM13, _("5"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem19);
    MenuItem20 = new wxMenuItem(Menu4, ID_MENUITEM14, _("6"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem20);
    MenuItem21 = new wxMenuItem(Menu4, ID_MENUITEM15, _("7"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem21);
    MenuItem22 = new wxMenuItem(Menu4, ID_MENUITEM16, _("8"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem22);
    MenuBar1->Append(Menu4, _("Profile"));
    Menu6 = new wxMenu();
    MenuItem30 = new wxMenuItem(Menu6, ID_MENUITEM24, _("Multiple Mice and Keyboards"), wxEmptyString, wxITEM_CHECK);
    Menu6->Append(MenuItem30);
    MenuItem31 = new wxMenuItem(Menu6, ID_MENUITEM25, _("Link controls"), wxEmptyString, wxITEM_CHECK);
    Menu6->Append(MenuItem31);
    MenuItem31->Check(true);
    MenuBar1->Append(Menu6, _("Advanced"));
    Menu2 = new wxMenu();
    MenuUpdate = new wxMenuItem(Menu2, ID_MENUITEM26, _("Update"), wxEmptyString, wxITEM_NORMAL);
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
    FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("XML files (*.xml)|*.xml"), wxFD_DEFAULT_STYLE|wxFD_OPEN, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    GridSizer1->Fit(this);
    GridSizer1->SetSizeHints(this);
    
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton1Click1);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton10Click1);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnLSIncAutoDetectClick1);
    Connect(ID_BUTTON14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnLSIncDeleteClick);
    Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnLSDeadzoneChange);
    Connect(ID_BUTTON15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnLSDecAutoDetectClick);
    Connect(ID_BUTTON16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnLSDecDeleteClick);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnRSIncAutoDetectClick1);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnRSIncDeleteClick);
    Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnRSDeadZoneChange);
    Connect(ID_BUTTON17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnRSDecAutoDetectClick);
    Connect(ID_BUTTON18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnRSDecDeleteClick);
    Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonTabEventTypeSelect1);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonTabAutoDetectClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonAdd1Click);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonTabRemoveClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonModifyButton);
    Connect(ID_CHOICE7,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonTabEventTypeSelect);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnAxisTabAutoDetectClick);
    Connect(ID_TEXTCTRL8,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL9,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL10,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrl);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnAxisTabShapeSelect);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrl);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton3Click);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton7Click);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonModifyAxis);
    Connect(idMenuNew,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemNew);
    Connect(idMenuOpen,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuOpen);
    Connect(idMenuSave,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuSave);
    Connect(idMenuSaveAs,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuSaveAs);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnQuit);
    Connect(ID_MENUITEM12,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemCopyConfiguration);
    Connect(ID_MENUITEM18,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemCopyController);
    Connect(ID_MENUITEM17,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemPasteConfiguration);
    Connect(ID_MENUITEM19,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemPasteController);
    Connect(ID_MENUITEM23,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuSetMouseDPI);
    Connect(ID_MENUITEM20,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuReplaceMouse);
    Connect(ID_MENUITEM22,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuReplaceMouseDPI);
    Connect(ID_MENUITEM21,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuReplaceKeyboard);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController1);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController2);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController3);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController4);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController5);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController6);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemController7);
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration1);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration2);
    Connect(ID_MENUITEM10,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration3);
    Connect(ID_MENUITEM11,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration4);
    Connect(ID_MENUITEM13,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration5);
    Connect(ID_MENUITEM14,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration6);
    Connect(ID_MENUITEM15,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration7);
    Connect(ID_MENUITEM16,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuItemConfiguration8);
    Connect(ID_MENUITEM24,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuMultipleMK);
    Connect(ID_MENUITEM26,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnMenuUpdate);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnAbout);
    //*)

    fillButtonChoice(ButtonTabButtonId);
    fillAxisAxisChoice(AxisTabAxisId);

    currentController = 0;
    currentConfiguration = 0;

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

#ifdef WIN32
    MenuItem8->Enable(false);
    MenuItem9->Enable(false);
    MenuItem10->Enable(false);
    MenuItem11->Enable(false);
    MenuItem12->Enable(false);
    MenuItem13->Enable(false);
    MenuItem24->Enable(false);
    MenuItem25->Enable(false);
#endif

    ButtonTabControls->AutoSizeColumns();
    Grid2->AutoSizeColumns();

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
        wxfile.Append(file.SubString(0, file.Length() - 1));
      }

      if(::wxFileExists(wxfile))
      {
        configFile.ReadConfigFile(wxfile);
        MenuItem30->Check(configFile.MultipleMK());
        if(MenuItem30->IsChecked())
        {
            MenuItem26->Enable(true);
            MenuItem27->Enable(true);
        }
        else
        {
            MenuItem26->Enable(false);
            MenuItem27->Enable(false);
        }
        load_current();
        refresh_gui();
        Menu1->Enable(idMenuSave, true);
      }
      else
      {
        wxMessageBox( wxT("Cannot open config file: ") + wxString(file, wxConvUTF8), wxT("Error"), wxICON_ERROR);
      }

      if(file.EndsWith(_("*")))
      {
        wxMessageBox( wxT("This file is read-only."), wxT("Info"), wxICON_INFORMATION);
        Menu1->Enable(idMenuSave, false);
      }
    }

    configFile.SetEvCatch(&evcatch);
}

sixaxis_emu_guiFrame::~sixaxis_emu_guiFrame()
{
    //(*Destroy(sixaxis_emu_guiFrame)
    //*)
}

void sixaxis_emu_guiFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void sixaxis_emu_guiFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(_(INFO_DESCR)) + wxString(_("\n")) + wxString(_(INFO_YEAR)) + wxString(_(" ")) + wxString(_(INFO_DEV)) + wxString(_(" ")) + wxString(_(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void sixaxis_emu_guiFrame::OnMenuItemNew(wxCommandEvent& event)
{
    configFile = ConfigurationFile();

    currentController = 0;
    currentConfiguration = 0;
    Menu3->Check(ID_MENUITEM1, true);
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    refresh_gui();
    Menu1->Enable(idMenuSave, false);
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void sixaxis_emu_guiFrame::OnButtonAdd1Click(wxCommandEvent& event)
{
    if(ButtonTabEventId->GetLabel() == wxEmptyString)
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(ButtonTabButtonId->GetStringSelection() == wxEmptyString)
    {
        wxMessageBox( wxT("Please select a Button!"), wxT("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(ButtonTabDeviceType->GetLabel(), ButtonTabDeviceName->GetLabel(), ButtonTabDeviceId->GetLabel(), ButtonTabEventType->GetStringSelection(), ButtonTabEventId->GetLabel(), -1, -1);

    if(!check.IsEmpty())
    {
      int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        return;
      }
    }

    ButtonTabControls->InsertRows();
    ButtonTabControls->SetCellValue(0, 0, ButtonTabDeviceType->GetLabel());
    ButtonTabControls->SetCellValue(0, 1, ButtonTabDeviceName->GetLabel());
    ButtonTabControls->SetCellValue(0, 2, ButtonTabDeviceId->GetLabel());
    ButtonTabControls->SetCellValue(0, 3, ButtonTabEventType->GetStringSelection());
    ButtonTabControls->SetCellValue(0, 4, ButtonTabEventId->GetLabel());
    ButtonTabControls->SetCellValue(0, 5, ButtonTabThreshold->GetValue());
    ButtonTabControls->SetCellValue(0, 6, ButtonTabButtonId->GetStringSelection());
    ButtonTabControls->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton3Click(wxCommandEvent& event)
{
    if(AxisTabEventId->GetLabel() == wxEmptyString)
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(AxisTabAxisId->GetStringSelection() == wxEmptyString)
    {
        wxMessageBox( wxT("Please select an Axis!"), wxT("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(AxisTabDeviceType->GetLabel(), AxisTabDeviceName->GetLabel(), AxisTabDeviceId->GetLabel(), AxisTabEventType->GetStringSelection(), AxisTabEventId->GetLabel(), -1, -1);

    if(!check.IsEmpty())
    {
      int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        return;
      }
    }

    Grid2->InsertRows();
    Grid2->SetCellValue(0, 0, AxisTabDeviceType->GetLabel());
    Grid2->SetCellValue(0, 1, AxisTabDeviceName->GetLabel());
    Grid2->SetCellValue(0, 2, AxisTabDeviceId->GetLabel());
    Grid2->SetCellValue(0, 3, AxisTabEventType->GetStringSelection());
    Grid2->SetCellValue(0, 4, AxisTabEventId->GetLabel());
    Grid2->SetCellValue(0, 5, AxisTabAxisId->GetStringSelection());
    Grid2->SetCellValue(0, 6, AxisTabDeadZone->GetValue());
    Grid2->SetCellValue(0, 7, AxisTabSensitivity->GetValue());
    Grid2->SetCellValue(0, 8, AxisTabAcceleration->GetValue());
    Grid2->SetCellValue(0, 9, AxisTabShape->GetStringSelection());
    Grid2->SetCellValue(0, 10, AxisTabBufferSize->GetValue());
    Grid2->SetCellValue(0, 11, AxisTabFilter->GetValue());
    Grid2->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::DeleteLinkedRows(wxGrid* grid, int row)
{
  if(grid == ButtonTabControls)
  {
    wxString old_device_type = ButtonTabControls->GetCellValue(row, 0);
    wxString old_device_name = ButtonTabControls->GetCellValue(row, 1);
    wxString old_device_id = ButtonTabControls->GetCellValue(row, 2);
    wxString old_event_type = ButtonTabControls->GetCellValue(row, 3);
    wxString old_event_id = ButtonTabControls->GetCellValue(row, 4);
    wxString old_button_id = ButtonTabControls->GetCellValue(row, 6);

    Controller* controller = configFile.GetController(currentController);

    for(unsigned int k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      if(k == currentConfiguration)
      {
        continue;
      }

      Configuration* config = controller->GetConfiguration(k);

      std::list<ButtonMapper>* buttonMappers = config->GetButtonMapperList();
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); )
      {
        if (it->GetDevice()->GetType() == old_device_type
            && it->GetDevice()->GetName() == old_device_name
            && it->GetDevice()->GetId() == old_device_id
            && it->GetEvent()->GetType() == old_event_type
            && it->GetEvent()->GetId() == old_event_id
            && it->GetButton() == old_button_id)
        {
          it = buttonMappers ->erase(it);
        }
        else
        {
          it++;
        }
      }
    }
  }
  else if(grid == Grid2)
  {
    wxString old_device_type = Grid2->GetCellValue(row, 0);
    wxString old_device_name = Grid2->GetCellValue(row, 1);
    wxString old_device_id = Grid2->GetCellValue(row, 2);
    wxString old_event_type = Grid2->GetCellValue(row, 3);
    wxString old_event_id = Grid2->GetCellValue(row, 4);
    wxString old_axis_id = Grid2->GetCellValue(row, 5);

    Controller* controller = configFile.GetController(currentController);

    for(unsigned int k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      if(k == currentConfiguration)
      {
        continue;
      }

      Configuration* config = controller->GetConfiguration(k);

      std::list<AxisMapper>* axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); )
      {
        if (it->GetDevice()->GetType() == old_device_type
            && it->GetDevice()->GetName() == old_device_name
            && it->GetDevice()->GetId() == old_device_id
            && it->GetEvent()->GetType() == old_event_type
            && it->GetEvent()->GetId() == old_event_id
            && it->GetAxis() == old_axis_id)
        {
          it = axisMappers ->erase(it);
        }
        else
        {
          it++;
        }
      }
    }
  }
}

void sixaxis_emu_guiFrame::DeleteSelectedRows(wxGrid* grid)
{
    unsigned int first;
    unsigned int nbRows;
    wxArrayInt array = grid->GetSelectedRows();

    if(array.GetCount())
    {
        first = array[0];
    }
    else
    {
        return;
    }

    while(array.GetCount())
    {
        if(MenuItem31->IsChecked())
        {
          DeleteLinkedRows(grid, array.Item(0));
        }
        grid->DeleteRows(array.Item(0), 1);
        array = grid->GetSelectedRows();
    }

    nbRows = grid->GetNumberRows();

    if(nbRows > first)
    {
        grid->SelectRow(first);
    }
    else if(nbRows)
    {
        grid->SelectRow(nbRows-1);
    }

    grid->AutoSizeColumns();
}

void sixaxis_emu_guiFrame::OnButtonTabRemoveClick(wxCommandEvent& event)
{
    sixaxis_emu_guiFrame::DeleteSelectedRows(ButtonTabControls);
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton7Click(wxCommandEvent& event)
{
    sixaxis_emu_guiFrame::DeleteSelectedRows(Grid2);
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButtonTabEventTypeSelect(wxCommandEvent& event)
{
    AxisTabDeviceType->SetLabel(wxEmptyString);
    AxisTabDeviceName->SetLabel(wxEmptyString);
    AxisTabDeviceId->SetLabel(wxEmptyString);
    AxisTabEventId->SetLabel(wxEmptyString);
    AxisTabDeadZone->Disable();
	  AxisTabDeadZone->SetValue(wxEmptyString);
  	AxisTabSensitivity->Disable();
	  AxisTabSensitivity->SetValue(wxEmptyString);
	  AxisTabAcceleration->Disable();
	  AxisTabAcceleration->SetValue(wxEmptyString);
	  AxisTabShape->SetSelection(0);
	  AxisTabShape->Disable();
	  AxisTabBufferSize->Disable();
	  AxisTabBufferSize->SetValue(wxEmptyString);
	  AxisTabFilter->Disable();
	  AxisTabFilter->SetValue(wxEmptyString);

    if(AxisTabEventType->GetStringSelection() == _("button"))
    {
        fillButtonAxisChoice(AxisTabAxisId);
    }
    else
    {
        fillAxisAxisChoice(AxisTabAxisId);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButtonTabEventTypeSelect1(wxCommandEvent& event)
{
    ButtonTabDeviceType->SetLabel(wxEmptyString);
    ButtonTabDeviceName->SetLabel(wxEmptyString);
    ButtonTabDeviceId->SetLabel(wxEmptyString);
    ButtonTabEventId->SetLabel(wxEmptyString);

    if(ButtonTabEventType->GetStringSelection() == _("button"))
    {
        ButtonTabThreshold->Disable();
        ButtonTabThreshold->SetValue(wxEmptyString);
    }
    else
    {
        ButtonTabThreshold->Enable();
        ButtonTabThreshold->SetValue(_("10"));
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::auto_detect(wxStaticText* device_type, wxStaticText* device_name, wxStaticText* device_id, wxString event_type, wxStaticText* event_id)
{
    evcatch.run(wxEmptyString, event_type);

    device_type->SetLabel(wxString(evcatch.GetDeviceType()));

    if(MenuItem30->IsChecked() || evcatch.GetDeviceType() == _("joystick"))
    {
      device_name->SetLabel(wxString(evcatch.GetDeviceName()));
      device_id->SetLabel( wxString(evcatch.GetDeviceId()));
    }
    else
    {
      device_name->SetLabel(wxEmptyString);
      device_id->SetLabel(_("0"));
    }

    event_id->SetLabel( wxString(evcatch.GetEventId()));
}

void sixaxis_emu_guiFrame::OnButton1Click1(wxCommandEvent& event)
{
    Button1->Enable(false);

    wxString old_device_type = ProfileTriggerDeviceType->GetLabel();
    wxString old_device_name = ProfileTriggerDeviceName->GetLabel();
    wxString old_device_id = ProfileTriggerDeviceId->GetLabel();
    wxString old_event_id = ProfileTriggerButtonId->GetLabel();
    Device* dev;
    Event* ev;

    auto_detect(ProfileTriggerDeviceType, ProfileTriggerDeviceName, ProfileTriggerDeviceId, _("button"), ProfileTriggerButtonId);

    if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
    {
      int k;

      Controller* controller = configFile.GetController(currentController);

      for(k=0; k<MAX_CONFIGURATIONS; ++k)
      {
        Configuration* config = controller->GetConfiguration(k);

        dev = config->GetTrigger()->GetDevice();
        ev = config->GetTrigger()->GetEvent();

        if(dev->GetType() == old_device_type && dev->GetType() == old_device_type && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
        {
          dev->SetType(ProfileTriggerDeviceType->GetLabel());
          dev->SetName(ProfileTriggerDeviceName->GetLabel());
          dev->SetId(ProfileTriggerDeviceId->GetLabel());
          ev->SetId(ProfileTriggerButtonId->GetLabel());
        }
      }
    }

    Button1->Enable(true);

    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButtonTabAutoDetectClick(wxCommandEvent& event)
{
    ButtonTabAutoDetect->Enable(false);

    auto_detect(ButtonTabDeviceType, ButtonTabDeviceName, ButtonTabDeviceId, ButtonTabEventType->GetStringSelection(), ButtonTabEventId);

    if(evcatch.GetEventType() == _("button"))
    {
        ButtonTabThreshold->Disable();
        ButtonTabThreshold->SetValue(wxEmptyString);
    }
    else
    {
        ButtonTabThreshold->Enable();
        ButtonTabThreshold->SetValue(_("10"));
    }

    fillButtonChoice(ButtonTabButtonId);

    refresh_gui();

    ButtonTabAutoDetect->Enable(true);
}

void sixaxis_emu_guiFrame::OnAxisTabAutoDetectClick(wxCommandEvent& event)
{
    wxString old_device_type = AxisTabDeviceType->GetLabel();
    wxString old_device_name = AxisTabDeviceName->GetLabel();
    wxString old_device_id = AxisTabDeviceId->GetLabel();

    AxisTabAutoDetect->Enable(false);

    auto_detect(AxisTabDeviceType, AxisTabDeviceName, AxisTabDeviceId, AxisTabEventType->GetStringSelection(), AxisTabEventId);

  	if(old_device_type != AxisTabDeviceType->GetLabel()
       || old_device_name != AxisTabDeviceName->GetLabel()
       || old_device_id != AxisTabDeviceId->GetLabel())
    {
        if(evcatch.GetEventType() == _("button"))
        {
            AxisTabDeadZone->Disable();
            AxisTabDeadZone->SetValue(wxEmptyString);
            AxisTabSensitivity->Disable();
            AxisTabSensitivity->SetValue(wxEmptyString);
            AxisTabAcceleration->Disable();
            AxisTabAcceleration->SetValue(wxEmptyString);
            AxisTabShape->Disable();
            AxisTabShape->SetSelection(0);
            fillButtonAxisChoice(AxisTabAxisId);
        }
        else
        {
      	  AxisTabDeadZone->Enable();
          AxisTabSensitivity->Enable();
          AxisTabAcceleration->Enable();
          AxisTabAcceleration->SetValue(_("1.00"));
          AxisTabShape->Enable();
          AxisTabShape->SetSelection(1);
          if(evcatch.GetDeviceType() == _("mouse"))
          {
              AxisTabDeadZone->SetValue(_("20"));
              AxisTabSensitivity->SetValue(_("1.00"));
              AxisTabBufferSize->Enable();
              AxisTabBufferSize->SetValue(_("1"));
              AxisTabFilter->Enable();
              AxisTabFilter->SetValue(_("0.00"));
          }
          else if(evcatch.GetDeviceType() == _("joystick"))
          {
              AxisTabDeadZone->SetValue(_("0"));
              if(!AxisTabAxisId->GetStringSelection().Contains(_("stick")))
              {
                  AxisTabAcceleration->SetValue(_("0.008"));
              }
              else
              {
                  AxisTabAcceleration->SetValue(_("0.004"));
              }
        	    AxisTabBufferSize->Disable();
              AxisTabBufferSize->SetValue(wxEmptyString);
              AxisTabFilter->Disable();
              AxisTabFilter->SetValue(wxEmptyString);
          }
          fillAxisAxisChoice(AxisTabAxisId);
        }
    }

    refresh_gui();

    AxisTabAutoDetect->Enable(true);
}

void sixaxis_emu_guiFrame::save_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Save Trigger
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetType(ProfileTriggerDeviceType->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetName(ProfileTriggerDeviceName->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetId(ProfileTriggerDeviceId->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->SetId(ProfileTriggerButtonId->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetSwitchBack(CheckBox1->GetValue()?_("yes"):_("no"));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetDelay(ProfileTriggerDelay->GetValue());
    //Save Intensity
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    intensityList->erase(intensityList->begin(), intensityList->end());
    //Save left stick Intensity
    intensityList->push_back(Intensity(_("left_stick"),
        LSIncDeviceType->GetLabel(), LSIncDeviceId->GetLabel(), LSIncDeviceName->GetLabel(), LSIncButtonId->GetLabel(),
        LSDecDeviceType->GetLabel(), LSDecDeviceId->GetLabel(), LSDecDeviceName->GetLabel(), LSDecButtonId->GetLabel(),
        LSDeadzone->GetValue(), LSSteps->GetValue(), LSShape->GetStringSelection()));
    //Save right stick intensity
    intensityList->push_back(Intensity(_("right_stick"),
        RSIncDeviceType->GetLabel(), RSIncDeviceId->GetLabel(), RSIncDeviceName->GetLabel(), RSIncButtonId->GetLabel(),
        RSDecDeviceType->GetLabel(), RSDecDeviceId->GetLabel(), RSDecDeviceName->GetLabel(), RSDecButtonId->GetLabel(),
        RSDeadZone->GetValue(), RSSteps->GetValue(), RSShape->GetStringSelection()));
    //Save ButtonMappers
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    buttonMappers->erase(buttonMappers->begin(), buttonMappers->end());
    for(int i=0; i<ButtonTabControls->GetNumberRows(); i++)
    {
        //ButtonMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString threshold, wxString button)
        buttonMappers->push_front(ButtonMapper(ButtonTabControls->GetCellValue(i, 0), ButtonTabControls->GetCellValue(i, 2), ButtonTabControls->GetCellValue(i, 1), ButtonTabControls->GetCellValue(i, 3), ButtonTabControls->GetCellValue(i, 4), ButtonTabControls->GetCellValue(i, 5), ButtonTabControls->GetCellValue(i, 6), wxEmptyString));//todo
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    axisMappers->erase(axisMappers->begin(), axisMappers->end());
    for(int i=0; i<Grid2->GetNumberRows(); i++)
    {
        //AxisMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString axis, wxString deadZone, wxString multiplier, wxString exponent);
        axisMappers->push_front(AxisMapper(Grid2->GetCellValue(i, 0), Grid2->GetCellValue(i, 2), Grid2->GetCellValue(i, 1), Grid2->GetCellValue(i, 3), Grid2->GetCellValue(i, 4), Grid2->GetCellValue(i, 5), Grid2->GetCellValue(i, 6), Grid2->GetCellValue(i, 7), Grid2->GetCellValue(i, 8), Grid2->GetCellValue(i, 9), Grid2->GetCellValue(i, 10), Grid2->GetCellValue(i, 11), wxEmptyString));//todo
    }

}

void sixaxis_emu_guiFrame::load_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Load Trigger
    ProfileTriggerDeviceType->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetType());
    ProfileTriggerDeviceName->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetName());
    ProfileTriggerDeviceId->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetId());
    ProfileTriggerButtonId->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->GetId());
    if(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetSwitchBack() == _("yes"))
    {
        CheckBox1->SetValue(true);
    }
    else
    {
        CheckBox1->SetValue(false);
    }
    ProfileTriggerDelay->SetValue(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDelay());
    //Load left & right stick intensities
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    LSIncDeviceType->SetLabel(wxEmptyString);
    LSIncDeviceName->SetLabel(wxEmptyString);
    LSIncDeviceId->SetLabel(wxEmptyString);
    LSIncButtonId->SetLabel(wxEmptyString);
    LSDecDeviceType->SetLabel(wxEmptyString);
    LSDecDeviceName->SetLabel(wxEmptyString);
    LSDecDeviceId->SetLabel(wxEmptyString);
    LSDecButtonId->SetLabel(wxEmptyString);
    LSDeadzone->SetValue(0);
    LSSteps->SetValue(1);
    LSShape->SetSelection(0);
    RSIncDeviceType->SetLabel(wxEmptyString);
    RSIncDeviceName->SetLabel(wxEmptyString);
    RSIncDeviceId->SetLabel(wxEmptyString);
    RSIncButtonId->SetLabel(wxEmptyString);
    RSDecDeviceType->SetLabel(wxEmptyString);
    RSDecDeviceName->SetLabel(wxEmptyString);
    RSDecDeviceId->SetLabel(wxEmptyString);
    RSDecButtonId->SetLabel(wxEmptyString);
    RSDeadZone->SetValue(0);
    RSSteps->SetValue(1);
    RSShape->SetSelection(0);
    for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
    {
      if(it->GetControl() == _("left_stick"))
      {
        LSIncDeviceType->SetLabel(it->GetDeviceUp()->GetType());
        LSIncDeviceName->SetLabel(it->GetDeviceUp()->GetName());
        LSIncDeviceId->SetLabel(it->GetDeviceUp()->GetId());
        LSIncButtonId->SetLabel(it->GetEventUp()->GetId());
        LSDecDeviceType->SetLabel(it->GetDeviceDown()->GetType());
        LSDecDeviceName->SetLabel(it->GetDeviceDown()->GetName());
        LSDecDeviceId->SetLabel(it->GetDeviceDown()->GetId());
        LSDecButtonId->SetLabel(it->GetEventDown()->GetId());
        LSDeadzone->SetValue(it->GetDeadZone());
        LSSteps->SetValue(it->GetSteps());
        LSShape->SetSelection(LSShape->FindString(it->GetShape()));
      }
      else if(it->GetControl() == _("right_stick"))
      {
        RSIncDeviceType->SetLabel(it->GetDeviceUp()->GetType());
        RSIncDeviceName->SetLabel(it->GetDeviceUp()->GetName());
        RSIncDeviceId->SetLabel(it->GetDeviceUp()->GetId());
        RSIncButtonId->SetLabel(it->GetEventUp()->GetId());
        RSDecDeviceType->SetLabel(it->GetDeviceDown()->GetType());
        RSDecDeviceName->SetLabel(it->GetDeviceDown()->GetName());
        RSDecDeviceId->SetLabel(it->GetDeviceDown()->GetId());
        RSDecButtonId->SetLabel(it->GetEventDown()->GetId());
        RSDeadZone->SetValue(it->GetDeadZone());
        RSSteps->SetValue(it->GetSteps());
        RSShape->SetSelection(RSShape->FindString(it->GetShape()));
      }
    }
    //Load ButtonMappers
    ButtonTabControls->DeleteRows(0, ButtonTabControls->GetNumberRows());
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); it++)
    {
        ButtonTabControls->InsertRows();
        ButtonTabControls->SetCellValue(0, 0, it->GetDevice()->GetType());
        ButtonTabControls->SetCellValue(0, 1, it->GetDevice()->GetName());
        ButtonTabControls->SetCellValue(0, 2, it->GetDevice()->GetId());
        ButtonTabControls->SetCellValue(0, 3, it->GetEvent()->GetType());
        ButtonTabControls->SetCellValue(0, 4, it->GetEvent()->GetId());
        ButtonTabControls->SetCellValue(0, 5, it->GetEvent()->GetThreshold());
        ButtonTabControls->SetCellValue(0, 6, it->GetButton());
        ButtonTabControls->AutoSizeColumns();
    }
    //Load AxisMappers
    Grid2->DeleteRows(0, Grid2->GetNumberRows());
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
    {
        Grid2->InsertRows();
        Grid2->SetCellValue(0, 0, it->GetDevice()->GetType());
        Grid2->SetCellValue(0, 1, it->GetDevice()->GetName());
        Grid2->SetCellValue(0, 2, it->GetDevice()->GetId());
        Grid2->SetCellValue(0, 3, it->GetEvent()->GetType());
        Grid2->SetCellValue(0, 4, it->GetEvent()->GetId());
        Grid2->SetCellValue(0, 5, it->GetAxis());
        Grid2->SetCellValue(0, 6, it->GetEvent()->GetDeadZone());
        Grid2->SetCellValue(0, 7, it->GetEvent()->GetMultiplier());
        Grid2->SetCellValue(0, 8, it->GetEvent()->GetExponent());
        Grid2->SetCellValue(0, 9, it->GetEvent()->GetShape());
        Grid2->SetCellValue(0, 10, it->GetEvent()->GetBufferSize());
        Grid2->SetCellValue(0, 11, it->GetEvent()->GetFilter());
        Grid2->AutoSizeColumns();
    }
}

void sixaxis_emu_guiFrame::refresh_gui()
{
    Panel1->Layout();
    Panel2->Layout();
    Panel3->Layout();
    Refresh();
}

void sixaxis_emu_guiFrame::OnMenuOpen(wxCommandEvent& event)
{
    if ( FileDialog1->ShowModal() != wxID_OK ) return;

    wxString FileName = FileDialog1->GetPath();
    if ( FileName.IsEmpty() ) return;

    configFile.ReadConfigFile(FileName);

    MenuItem30->Check(configFile.MultipleMK());
    if(MenuItem30->IsChecked())
    {
        MenuItem26->Enable(true);
        MenuItem27->Enable(true);
    }
    else
    {
        MenuItem26->Enable(false);
        MenuItem27->Enable(false);
    }

    currentController = 0;
    currentConfiguration = 0;
    Menu3->Check(ID_MENUITEM1, true);
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    refresh_gui();
    Menu1->Enable(idMenuSave, true);
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void sixaxis_emu_guiFrame::OnMenuItemController1(wxCommandEvent& event)
{
    save_current();
    currentController = 0;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController2(wxCommandEvent& event)
{
    save_current();
    currentController = 1;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController3(wxCommandEvent& event)
{
    save_current();
    currentController = 2;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController4(wxCommandEvent& event)
{
    save_current();
    currentController = 3;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController5(wxCommandEvent& event)
{
    save_current();
    currentController = 4;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController6(wxCommandEvent& event)
{
    save_current();
    currentController = 5;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemController7(wxCommandEvent& event)
{
    save_current();
    currentController = 6;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration1(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 0;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration2(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 1;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration3(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 2;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration4(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 3;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration5(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 4;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration6(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 5;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration7(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 6;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration8(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 7;
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuSave(wxCommandEvent& event)
{
    wxString end;
    save_current();
    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(wxT("Can't save ") + configFile.GetFilePath(), wxT("Error"), wxICON_ERROR);
    }
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void sixaxis_emu_guiFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _T("Save Config file"), _T(""), _T(""), _T("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(FileName);

    OnMenuSave(event);

    Menu1->Enable(idMenuSave, true);
}

void sixaxis_emu_guiFrame::OnButtonModifyButton(wxCommandEvent& event)
{
    wxArrayInt array = ButtonTabControls->GetSelectedRows();
    int count = array.GetCount();

    if(ButtonTabModify->GetLabel() == _("Modify"))
    {
        if(count == 0)
        {
            wxMessageBox( wxT("Please select a line of the table."), wxT("Info"), wxICON_INFORMATION);
            return;
        }
        else if(count > 1)
        {
            wxMessageBox( wxT("Please select a SINGLE line of the table."), wxT("Info"), wxICON_INFORMATION);
            return;
        }
        grid1mod = array.Item(0);

        ButtonTabDeviceType->SetLabel(ButtonTabControls->GetCellValue(grid1mod, 0));
        ButtonTabDeviceName->SetLabel(ButtonTabControls->GetCellValue(grid1mod, 1));
        ButtonTabDeviceId->SetLabel(ButtonTabControls->GetCellValue(grid1mod, 2));
        ButtonTabEventType->SetSelection(ButtonTabEventType->FindString(ButtonTabControls->GetCellValue(grid1mod, 3)));
        ButtonTabEventId->SetLabel(ButtonTabControls->GetCellValue(grid1mod, 4));
        ButtonTabThreshold->SetValue(ButtonTabControls->GetCellValue(grid1mod, 5));
        fillButtonChoice(ButtonTabButtonId);
        ButtonTabButtonId->SetSelection(ButtonTabButtonId->FindString(ButtonTabControls->GetCellValue(grid1mod, 6)));
        ButtonTabAdd->Disable();
        ButtonTabRemove->Disable();
        ButtonTabModify->SetLabel(_("Apply"));
    }
    else
    {
        if (ButtonTabEventId->GetLabel() == wxEmptyString)
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(ButtonTabButtonId->GetStringSelection() == wxEmptyString)
        {
            wxMessageBox( wxT("Please select a Button!"), wxT("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(ButtonTabDeviceType->GetLabel(), ButtonTabDeviceName->GetLabel(), ButtonTabDeviceId->GetLabel(), ButtonTabEventType->GetStringSelection(), ButtonTabEventId->GetLabel(), grid1mod, -1);

        if(!check.IsEmpty())
        {
          int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            return;
          }
        }

        if(MenuItem31->IsChecked())
        {
          updateButtonConfigurations();
        }
        ButtonTabControls->SetCellValue(grid1mod, 0, ButtonTabDeviceType->GetLabel());
        ButtonTabControls->SetCellValue(grid1mod, 1, ButtonTabDeviceName->GetLabel());
        ButtonTabControls->SetCellValue(grid1mod, 2, ButtonTabDeviceId->GetLabel());
        ButtonTabControls->SetCellValue(grid1mod, 3, ButtonTabEventType->GetStringSelection());
        ButtonTabControls->SetCellValue(grid1mod, 4, ButtonTabEventId->GetLabel());
        ButtonTabControls->SetCellValue(grid1mod, 5, ButtonTabThreshold->GetValue());
        ButtonTabControls->SetCellValue(grid1mod, 6, ButtonTabButtonId->GetStringSelection());
        ButtonTabAdd->Enable();
        ButtonTabRemove->Enable();
        ButtonTabModify->SetLabel(_("Modify"));
    }
    ButtonTabControls->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::updateButtonConfigurations()
{
    int k;

    std::list<ButtonMapper>* buttonMappers;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      buttonMappers = config->GetButtonMapperList();
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); it++)
      {
          if(it->GetDevice()->GetType() == ButtonTabControls->GetCellValue(grid1mod, 0)
              && it->GetDevice()->GetName() == ButtonTabControls->GetCellValue(grid1mod, 1)
              && it->GetDevice()->GetId() == ButtonTabControls->GetCellValue(grid1mod, 2)
              && it->GetEvent()->GetType() == ButtonTabControls->GetCellValue(grid1mod, 3)
              && it->GetEvent()->GetId() == ButtonTabControls->GetCellValue(grid1mod, 4)
              && it->GetButton() == ButtonTabControls->GetCellValue(grid1mod, 6))
          {
              it->GetDevice()->SetType(ButtonTabDeviceType->GetLabel());
              it->GetDevice()->SetId(ButtonTabDeviceId->GetLabel());
              it->GetDevice()->SetName(ButtonTabDeviceName->GetLabel());
              it->GetEvent()->SetType(ButtonTabEventType->GetStringSelection());
              it->GetEvent()->SetId(ButtonTabEventId->GetLabel());
              it->GetEvent()->SetThreshold(ButtonTabThreshold->GetValue());
              it->SetButton(ButtonTabButtonId->GetStringSelection());
          }
      }
    }
}

void sixaxis_emu_guiFrame::OnButtonModifyAxis(wxCommandEvent& event)
{
    wxArrayInt array = Grid2->GetSelectedRows();
    int count = array.GetCount();

    if(Button5->GetLabel() == _("Modify"))
    {
        if(count == 0)
        {
            wxMessageBox( wxT("Please select a line of the table."), wxT("Info"), wxICON_INFORMATION);
            return;
        }
        else if(count > 1)
        {
            wxMessageBox( wxT("Please select a SINGLE line of the table."), wxT("Info"), wxICON_INFORMATION);
            return;
        }
        grid2mod = array.Item(0);

        AxisTabDeviceType->SetLabel(Grid2->GetCellValue(grid2mod, 0));
        AxisTabDeviceName->SetLabel(Grid2->GetCellValue(grid2mod, 1));
        AxisTabDeviceId->SetLabel(Grid2->GetCellValue(grid2mod, 2));
        AxisTabEventType->SetSelection(AxisTabEventType->FindString(Grid2->GetCellValue(grid2mod, 3)));
        AxisTabEventId->SetLabel(Grid2->GetCellValue(grid2mod, 4));
        AxisTabDeadZone->SetValue(Grid2->GetCellValue(grid2mod, 6));
        AxisTabSensitivity->SetValue(Grid2->GetCellValue(grid2mod, 7));
        AxisTabAcceleration->SetValue(Grid2->GetCellValue(grid2mod, 8));
        AxisTabBufferSize->SetValue(Grid2->GetCellValue(grid2mod, 10));
        AxisTabFilter->SetValue(Grid2->GetCellValue(grid2mod, 11));
        if(AxisTabEventType->GetStringSelection() == _("button"))
        {
            AxisTabDeadZone->Disable();
            AxisTabSensitivity->Disable();
            AxisTabAcceleration->Disable();
            AxisTabShape->Disable();
            AxisTabShape->SetSelection(0);
            AxisTabBufferSize->Disable();
            AxisTabFilter->Disable();
            fillButtonAxisChoice(AxisTabAxisId);
        }
        else
        {
            AxisTabDeadZone->Enable();
            AxisTabSensitivity->Enable();
            AxisTabAcceleration->Enable();
            AxisTabShape->SetSelection(AxisTabShape->FindString(Grid2->GetCellValue(grid2mod, 9)));
            AxisTabShape->Enable();
			      if(AxisTabDeviceType->GetLabel() == _("mouse"))
			      {
                AxisTabBufferSize->Enable();
                AxisTabFilter->Enable();
      			}
            fillAxisAxisChoice(AxisTabAxisId);
        }
        AxisTabAxisId->SetSelection(AxisTabAxisId->FindString(Grid2->GetCellValue(grid2mod, 5)));
        Button3->Disable();
        Button7->Disable();
        Button5->SetLabel(_("Apply"));
    }
    else
    {
        if (AxisTabEventId->GetLabel() == wxEmptyString)
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(AxisTabAxisId->GetStringSelection() == wxEmptyString)
        {
            wxMessageBox( wxT("Please select an Axis!"), wxT("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(AxisTabDeviceType->GetLabel(), AxisTabDeviceName->GetLabel(), AxisTabDeviceId->GetLabel(), AxisTabEventType->GetStringSelection(), AxisTabEventId->GetLabel(), -1, grid2mod);

        if(!check.IsEmpty())
        {
          int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            return;
          }
        }

        if(MenuItem31->IsChecked())
        {
          updateAxisConfigurations();
        }
        Grid2->SetCellValue(grid2mod, 0, AxisTabDeviceType->GetLabel());
        Grid2->SetCellValue(grid2mod, 1, AxisTabDeviceName->GetLabel());
        Grid2->SetCellValue(grid2mod, 2, AxisTabDeviceId->GetLabel());
        Grid2->SetCellValue(grid2mod, 3, AxisTabEventType->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 4, AxisTabEventId->GetLabel());
        Grid2->SetCellValue(grid2mod, 5, AxisTabAxisId->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 6, AxisTabDeadZone->GetValue());
        Grid2->SetCellValue(grid2mod, 7, AxisTabSensitivity->GetValue());
        Grid2->SetCellValue(grid2mod, 8, AxisTabAcceleration->GetValue());
        Grid2->SetCellValue(grid2mod, 9, AxisTabShape->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 10, AxisTabBufferSize->GetValue());
        Grid2->SetCellValue(grid2mod, 11, AxisTabFilter->GetValue());
        Button3->Enable();
        Button7->Enable();
        Button5->SetLabel(_("Modify"));
    }
    Grid2->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::updateAxisConfigurations()
{
    int k;

    std::list<AxisMapper>* axisMappers;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
      {
          if(it->GetDevice()->GetType() == Grid2->GetCellValue(grid2mod, 0)
              && it->GetDevice()->GetName() == Grid2->GetCellValue(grid2mod, 1)
              && it->GetDevice()->GetId() == Grid2->GetCellValue(grid2mod, 2)
              && it->GetEvent()->GetType() == Grid2->GetCellValue(grid2mod, 3)
              && it->GetEvent()->GetId() == Grid2->GetCellValue(grid2mod, 4)
              && it->GetAxis() == Grid2->GetCellValue(grid2mod, 5))
          {
              it->GetDevice()->SetType(AxisTabDeviceType->GetLabel());
              it->GetDevice()->SetId(AxisTabDeviceId->GetLabel());
              it->GetDevice()->SetName(AxisTabDeviceName->GetLabel());
              it->GetEvent()->SetType(AxisTabEventType->GetStringSelection());
              it->GetEvent()->SetId(AxisTabEventId->GetLabel());
              it->SetAxis(AxisTabAxisId->GetStringSelection());
          }
      }
    }
}

void sixaxis_emu_guiFrame::OnAxisTabShapeSelect(wxCommandEvent& event)
{
    if(AxisTabEventType->GetStringSelection() == _("axis"))
    {
        if(AxisTabShape->GetStringSelection() == wxEmptyString)
        {
            AxisTabShape->SetSelection(1);
        }
    }
}

void sixaxis_emu_guiFrame::OnMenuItemCopyConfiguration(wxCommandEvent& event)
{
  save_current();
  tempConfiguration = *configFile.GetController(currentController)->GetConfiguration(currentConfiguration);
}

void sixaxis_emu_guiFrame::OnMenuItemPasteConfiguration(wxCommandEvent& event)
{
  configFile.GetController(currentController)->SetConfiguration(tempConfiguration, currentConfiguration);
  load_current();
  refresh_gui();
  ButtonTabModify->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  Button3->Enable();
  ButtonTabAdd->Enable();
  ButtonTabRemove->Enable();
  Button7->Enable();
}

void sixaxis_emu_guiFrame::OnMenuItemCopyController(wxCommandEvent& event)
{
  save_current();
  tempController = *configFile.GetController(currentController);
}

void sixaxis_emu_guiFrame::OnMenuItemPasteController(wxCommandEvent& event)
{
  configFile.SetController(tempController, currentController);
  load_current();
  refresh_gui();
  ButtonTabModify->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  Button3->Enable();
  ButtonTabAdd->Enable();
  ButtonTabRemove->Enable();
  Button7->Enable();
}

void sixaxis_emu_guiFrame::OnButton10Click1(wxCommandEvent& event)
{
    ProfileTriggerDeviceType->SetLabel(wxEmptyString);
    ProfileTriggerDeviceName->SetLabel(wxEmptyString);
    ProfileTriggerDeviceId->SetLabel(wxEmptyString);
    ProfileTriggerButtonId->SetLabel(wxEmptyString);
    CheckBox1->SetValue(false);

    refresh_gui();
}

void sixaxis_emu_guiFrame::replaceDevice(wxString device_type)
{
    int k;
    wxString device_name = wxEmptyString;
    wxString device_id = _("0");

    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;

    if(MenuItem30->IsChecked())
    {
	      evcatch.run(device_type, _("button"));
        device_name = evcatch.GetDeviceName();
        device_id = evcatch.GetDeviceId();
    }

    save_current();

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      Trigger* trigger = config->GetTrigger();

      if(trigger->GetDevice()->GetType() == device_type)
      {
        trigger->GetDevice()->SetId(device_id);
        trigger->GetDevice()->SetName(device_name);
      }

      intensityList = config->GetIntensityList();
      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
          if(it->GetDeviceUp()->GetType() == device_type)
          {
              it->GetDeviceUp()->SetId(device_id);
              it->GetDeviceUp()->SetName(device_name);
          }
          if(it->GetDeviceDown()->GetType() == device_type)
          {
              it->GetDeviceDown()->SetId(device_id);
              it->GetDeviceDown()->SetName(device_name);
          }
      }

      buttonMappers = config->GetButtonMapperList();
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); it++)
      {
          if(it->GetDevice()->GetType() == device_type)
          {
              it->GetDevice()->SetId(device_id);
              it->GetDevice()->SetName(device_name);
          }
      }
      axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
      {
        if(it->GetDevice()->GetType() == device_type)
        {
            it->GetDevice()->SetId(device_id);
            it->GetDevice()->SetName(device_name);
        }
      }
    }

    load_current();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuReplaceMouse(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the mouse in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(_("mouse"));
}

void sixaxis_emu_guiFrame::OnMenuReplaceKeyboard(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the keyboard in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(_("keyboard"));
}

void sixaxis_emu_guiFrame::OnMenuReplaceMouseDPI(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will adjust multipliers in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    int k;
    wxString device_name = wxEmptyString;
    wxString device_id = _("0");
    wxString device_type = _("mouse");

    std::list<AxisMapper>* axisMappers;

    int old_value, new_value;

    old_value = configFile.GetController(currentController)->GetMouseDPI();

    if(!old_value)
    {
      old_value = 2000;
    }

    wxNumberEntryDialog dialog1(this, wxT(""), wxT("Enter a number:"), wxT("Old mouse DPI value"), old_value, 100, 10000);
    if (dialog1.ShowModal() == wxID_OK)
    {
        old_value = dialog1.GetValue();

        wxNumberEntryDialog dialog2(this, wxT(""), wxT("Enter a number:"), wxT("New mouse DPI value"), 2000, 100, 10000);

        if (dialog2.ShowModal() == wxID_OK)
        {
            new_value = dialog2.GetValue();

            if(MenuItem30->IsChecked())
            {
                evcatch.run(device_type, _("button"));
                device_name = evcatch.GetDeviceName();
                device_id = evcatch.GetDeviceId();
            }

            save_current();

            Controller* controller = configFile.GetController(currentController);

            for(k=0; k<MAX_CONFIGURATIONS; ++k)
            {
              Configuration* config = controller->GetConfiguration(k);

              axisMappers = config->GetAxisMapperList();
              for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); it++)
              {
                if(it->GetDevice()->GetType() == device_type
                    || (MenuItem30->IsChecked() && it->GetDevice()->GetName() == device_name && it->GetDevice()->GetId() == device_id)
                )
                {
                    double val, exp;
                    wxString sval = it->GetEvent()->GetMultiplier();
                    wxString sexp = it->GetEvent()->GetExponent();
                    if(sval.ToDouble(&val) && sexp.ToDouble(&exp))
                    {
                        val = val * pow((double)old_value / new_value, exp);
                        sval.Printf(wxT("%.2f"), val);
                        it->GetEvent()->SetMultiplier(sval);
                    }
                }
              }
            }

            configFile.GetController(currentController)->SetMouseDPI(new_value);

            load_current();
            refresh_gui();
        }
    }
}

void sixaxis_emu_guiFrame::OnLSIncAutoDetectClick1(wxCommandEvent& event)
{
  LSIncAutoDetect->Enable(false);

  wxString old_device_type = LSIncDeviceType->GetLabel();
  wxString old_device_name = LSIncDeviceName->GetLabel();
  wxString old_device_id = LSIncDeviceId->GetLabel();
  wxString old_event_id = LSIncButtonId->GetLabel();

  auto_detect(LSIncDeviceType, LSIncDeviceName, LSIncDeviceId, _("button"), LSIncButtonId);

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("left_stick"))
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(LSIncDeviceType->GetLabel());
            dev->SetName(LSIncDeviceName->GetLabel());
            dev->SetId(LSIncDeviceId->GetLabel());
            ev->SetId(LSIncButtonId->GetLabel());
          }
        }
      }
    }
  }

  LSIncAutoDetect->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnLSIncDeleteClick(wxCommandEvent& event)
{
  wxString old_device_type = LSIncDeviceType->GetLabel();
  wxString old_device_name = LSIncDeviceName->GetLabel();
  wxString old_device_id = LSIncDeviceId->GetLabel();
  wxString old_event_id = LSIncButtonId->GetLabel();

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("left_stick"))
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(wxEmptyString);
            dev->SetName(wxEmptyString);
            dev->SetId(wxEmptyString);
            ev->SetId(wxEmptyString);
          }
        }
      }
    }
  }

  LSIncDeviceType->SetLabel(wxEmptyString);
  LSIncDeviceName->SetLabel(wxEmptyString);
  LSIncDeviceId->SetLabel(wxEmptyString);
  LSIncButtonId->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnRSIncAutoDetectClick1(wxCommandEvent& event)
{
  RSIncAutoDetect->Enable(false);

  wxString old_device_type = RSIncDeviceType->GetLabel();
  wxString old_device_name = RSIncDeviceName->GetLabel();
  wxString old_device_id = RSIncDeviceId->GetLabel();
  wxString old_event_id = RSIncButtonId->GetLabel();

  auto_detect(RSIncDeviceType, RSIncDeviceName, RSIncDeviceId, _("button"), RSIncButtonId);

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("right_stick"))
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(RSIncDeviceType->GetLabel());
            dev->SetName(RSIncDeviceName->GetLabel());
            dev->SetId(RSIncDeviceId->GetLabel());
            ev->SetId(RSIncButtonId->GetLabel());
          }
        }
      }
    }
  }

  RSIncAutoDetect->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnRSIncDeleteClick(wxCommandEvent& event)
{
  wxString old_device_type = RSIncDeviceType->GetLabel();
  wxString old_device_name = RSIncDeviceName->GetLabel();
  wxString old_device_id = RSIncDeviceId->GetLabel();
  wxString old_event_id = RSIncButtonId->GetLabel();

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("right_stick"))
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(wxEmptyString);
            dev->SetName(wxEmptyString);
            dev->SetId(wxEmptyString);
            ev->SetId(wxEmptyString);
          }
        }
      }
    }
  }

  RSIncDeviceType->SetLabel(wxEmptyString);
  RSIncDeviceName->SetLabel(wxEmptyString);
  RSIncDeviceId->SetLabel(wxEmptyString);
  RSIncButtonId->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnLSDeadzoneChange(wxSpinEvent& event)
{
    int max = 127-LSDeadzone->GetValue();
    if(max)
    {
        LSSteps->SetValue(LSSteps->GetValue()*LSSteps->GetMax()/max);
        LSSteps->SetRange(1, max);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnRSDeadZoneChange(wxSpinEvent& event)
{
    int max = 127-RSDeadZone->GetValue();
    if(max)
    {
        RSSteps->SetValue(RSSteps->GetValue()*RSSteps->GetMax()/max);
        RSSteps->SetRange(1, max);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnLSDecAutoDetectClick(wxCommandEvent& event)
{
  LSDecAutoDetect->Enable(false);

  wxString old_device_type = LSDecDeviceType->GetLabel();
  wxString old_device_name = LSDecDeviceName->GetLabel();
  wxString old_device_id = LSDecDeviceId->GetLabel();
  wxString old_event_id = LSDecButtonId->GetLabel();

  auto_detect(LSDecDeviceType, LSDecDeviceName, LSDecDeviceId, _("button"), LSDecButtonId);

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("left_stick"))
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(LSDecDeviceType->GetLabel());
            dev->SetName(LSDecDeviceName->GetLabel());
            dev->SetId(LSDecDeviceId->GetLabel());
            ev->SetId(LSDecButtonId->GetLabel());
          }
        }
      }
    }
  }

  LSDecAutoDetect->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnLSDecDeleteClick(wxCommandEvent& event)
{
  wxString old_device_type = LSDecDeviceType->GetLabel();
  wxString old_device_name = LSDecDeviceName->GetLabel();
  wxString old_device_id = LSDecDeviceId->GetLabel();
  wxString old_event_id = LSDecButtonId->GetLabel();

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("left_stick"))
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(wxEmptyString);
            dev->SetName(wxEmptyString);
            dev->SetId(wxEmptyString);
            ev->SetId(wxEmptyString);
          }
        }
      }
    }
  }

  LSDecDeviceType->SetLabel(wxEmptyString);
  LSDecDeviceName->SetLabel(wxEmptyString);
  LSDecDeviceId->SetLabel(wxEmptyString);
  LSDecButtonId->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnRSDecAutoDetectClick(wxCommandEvent& event)
{
  RSDecAutoDetect->Enable(false);

  wxString old_device_type = RSDecDeviceType->GetLabel();
  wxString old_device_name = RSDecDeviceName->GetLabel();
  wxString old_device_id = RSDecDeviceId->GetLabel();
  wxString old_event_id = RSDecButtonId->GetLabel();

  auto_detect(RSDecDeviceType, RSDecDeviceName, RSDecDeviceId, _("button"), RSDecButtonId);

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("right_stick"))
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(RSDecDeviceType->GetLabel());
            dev->SetName(RSDecDeviceName->GetLabel());
            dev->SetId(RSDecDeviceId->GetLabel());
            ev->SetId(RSDecButtonId->GetLabel());
          }
        }
      }
    }
  }

  RSDecAutoDetect->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnRSDecDeleteClick(wxCommandEvent& event)
{
  wxString old_device_type = RSDecDeviceType->GetLabel();
  wxString old_device_name = RSDecDeviceName->GetLabel();
  wxString old_device_id = RSDecDeviceId->GetLabel();
  wxString old_event_id = RSDecButtonId->GetLabel();

  if(old_device_type != wxEmptyString && MenuItem31->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
      {
        if(it->GetControl() == _("right_stick"))
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(wxEmptyString);
            dev->SetName(wxEmptyString);
            dev->SetId(wxEmptyString);
            ev->SetId(wxEmptyString);
          }
        }
      }
    }
  }

  RSDecDeviceType->SetLabel(wxEmptyString);
  RSDecDeviceName->SetLabel(wxEmptyString);
  RSDecDeviceId->SetLabel(wxEmptyString);
  RSDecButtonId->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuSetMouseDPI(wxCommandEvent& event)
{
    wxNumberEntryDialog dialog1(this, wxT(""), wxT("Enter a number:"), wxT("Mouse DPI value"), configFile.GetController(currentController)->GetMouseDPI(), 0, 10000);
    if (dialog1.ShowModal() == wxID_OK)
    {
       configFile.GetController(currentController)->SetMouseDPI(dialog1.GetValue());
    }
}

void sixaxis_emu_guiFrame::OnTextCtrl(wxCommandEvent& event)
{
    wxString str;
    wxTextCtrl* text;
    double value;
    long lvalue;

    text = (wxTextCtrl*)event.GetEventObject();
    str = text->GetValue();

    if(str.IsEmpty())
    {
        return;
    }

    if(str.Replace(_(","), _(".")))
    {
        text->SetValue(str);
    }

    if(text == AxisTabDeadZone)
    {
        if(!str.ToLong(&lvalue))
        {
            text->SetValue(_("20"));
        }
        else if(lvalue < 0)
        {
            text->SetValue(_("0"));
        }
        else if(lvalue > 64)
        {
            text->SetValue(_("64"));
        }
    }
    else if(text == AxisTabSensitivity)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(_("1.00"));
        }
    }
    else if(text == AxisTabAcceleration)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(_("1.00"));
        }
        else if(value < 0)
        {
            text->SetValue(_("0.00"));
        }
        else if(value > 2)
        {
            text->SetValue(_("2.00"));
        }
    }
    else if(text == AxisTabBufferSize)
    {
        if(!str.ToLong(&lvalue))
        {
            text->SetValue(_("1"));
        }
        else if(lvalue < 1)
        {
            text->SetValue(_("1"));
        }
        else if(lvalue > 30)
        {
            text->SetValue(_("30"));
        }
    }
    else if(text == AxisTabFilter)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(_("0.00"));
        }
        else if(value < 0)
        {
            text->SetValue(_("0.00"));
        }
        else if(value > 1)
        {
            text->SetValue(_("1.00"));
        }
    }
}

wxString sixaxis_emu_guiFrame::isAlreadyUsed(wxString device_type, wxString device_name, wxString device_id, wxString event_type, wxString event_id, int gridIndex1, int gridIndex2)
{
  for(int i=0; i<ButtonTabControls->GetNumberRows(); i++)
  {
    if(gridIndex1 >= 0 && i == gridIndex1)
    {
      continue;
    }

    if(ButtonTabControls->GetCellValue(i, 0) == device_type
       && (ButtonTabControls->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || ButtonTabControls->GetCellValue(i, 1) == device_name)
       && ButtonTabControls->GetCellValue(i, 2) == device_id
       && ButtonTabControls->GetCellValue(i, 3) == event_type
       && ButtonTabControls->GetCellValue(i, 4) == event_id)
    {
      return _("This control is already used in the current profile (Button tab).\nContinue?");
    }
  }

  for(int i=0; i<Grid2->GetNumberRows(); i++)
  {
    if(gridIndex2 >= 0 && i == gridIndex2)
    {
      continue;
    }

    if(Grid2->GetCellValue(i, 0) == device_type
       && (Grid2->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || Grid2->GetCellValue(i, 1) == device_name)
       && Grid2->GetCellValue(i, 2) == device_id
       && Grid2->GetCellValue(i, 3) == event_type
       && Grid2->GetCellValue(i, 4) == event_id)
    {
      return _("This control is already used in the current profile (Axis tab).\nContinue?");
    }
  }

  if(event_type == _("button"))
  {
    if(device_type == LSIncDeviceType->GetLabel()
       && (LSIncDeviceName->GetLabel().IsEmpty() || device_name.IsEmpty() || LSIncDeviceName->GetLabel() == device_name)
       && LSIncDeviceId->GetLabel() == device_id
       && LSIncButtonId->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Left > Increase.\nContinue?");
    }
    if(device_type == LSDecDeviceType->GetLabel()
       && (LSDecDeviceName->GetLabel().IsEmpty() || device_name.IsEmpty() || LSDecDeviceName->GetLabel() == device_name)
       && LSDecDeviceId->GetLabel() == device_id
       && LSDecButtonId->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Left > Decrease.\nContinue?");
    }

    if(device_type == RSIncDeviceType->GetLabel()
       && (RSIncDeviceName->GetLabel().IsEmpty() || device_name.IsEmpty() || RSIncDeviceName->GetLabel() == device_name)
       && RSIncDeviceId->GetLabel() == device_id
       && RSIncButtonId->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Right > Increase.\nContinue?");
    }
    if(device_type == RSDecDeviceType->GetLabel()
       && (RSDecDeviceName->GetLabel().IsEmpty() || device_name.IsEmpty() || RSDecDeviceName->GetLabel() == device_name)
       && RSDecDeviceId->GetLabel() == device_id
       && RSIncButtonId->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Right > Decrease.\nContinue?");
    }
  }

  return wxEmptyString;
}

void sixaxis_emu_guiFrame::OnMenuMultipleMK(wxCommandEvent& event)
{
    if(MenuItem30->IsChecked())
    {
        MenuItem26->Enable(true);
        MenuItem27->Enable(true);
    }
    else
    {
        MenuItem26->Enable(false);
        MenuItem27->Enable(false);
    }
    replaceDevice(_("mouse"));
    replaceDevice(_("keyboard"));
}

void sixaxis_emu_guiFrame::OnMenuUpdate(wxCommandEvent& event)
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
