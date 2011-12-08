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
#ifndef WIN32
#include <pwd.h>
#endif
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

#ifndef WIN32
char* homedir;
#endif

wxString default_directory;

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
    StaticText35 = new wxStaticText(Panel1, ID_STATICTEXT35, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer13->Add(StaticText35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText27 = new wxStaticText(Panel1, ID_STATICTEXT27, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer13->Add(StaticText27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText36 = new wxStaticText(Panel1, ID_STATICTEXT36, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer13->Add(StaticText36, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText37 = new wxStaticText(Panel1, ID_STATICTEXT37, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer13->Add(StaticText37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(Panel1, ID_BUTTON1, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer13->Add(Button1, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    CheckBox1 = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Switch back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox1->SetValue(false);
    FlexGridSizer13->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText28 = new wxStaticText(Panel1, ID_STATICTEXT28, _("Delay (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer13->Add(StaticText28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl5 = new wxSpinCtrl(Panel1, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(65,-1), 0, 0, 1000, 0, _T("ID_SPINCTRL5"));
    SpinCtrl5->SetValue(_T("0"));
    FlexGridSizer13->Add(SpinCtrl5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    StaticText58 = new wxStaticText(Panel1, ID_STATICTEXT58, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer14->Add(StaticText58, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText59 = new wxStaticText(Panel1, ID_STATICTEXT59, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer14->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText60 = new wxStaticText(Panel1, ID_STATICTEXT60, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer14->Add(StaticText60, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText61 = new wxStaticText(Panel1, ID_STATICTEXT61, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer14->Add(StaticText61, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button13 = new wxButton(Panel1, ID_BUTTON13, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    FlexGridSizer14->Add(Button13, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    Button14 = new wxButton(Panel1, ID_BUTTON14, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON14"));
    FlexGridSizer14->Add(Button14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText24 = new wxStaticText(Panel1, ID_STATICTEXT24, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer12->Add(StaticText24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl3 = new wxSpinCtrl(Panel1, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL3"));
    SpinCtrl3->SetValue(_T("0"));
    FlexGridSizer12->Add(SpinCtrl3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText62 = new wxStaticText(Panel1, ID_STATICTEXT62, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer12->Add(StaticText62, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl1 = new wxSpinCtrl(Panel1, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL1"));
    SpinCtrl1->SetValue(_T("1"));
    FlexGridSizer12->Add(SpinCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Decrease"));
    FlexGridSizer15 = new wxFlexGridSizer(1, 6, 0, 0);
    StaticText67 = new wxStaticText(Panel1, ID_STATICTEXT67, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT67"));
    FlexGridSizer15->Add(StaticText67, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText68 = new wxStaticText(Panel1, ID_STATICTEXT68, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT68"));
    FlexGridSizer15->Add(StaticText68, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText69 = new wxStaticText(Panel1, ID_STATICTEXT69, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT69"));
    FlexGridSizer15->Add(StaticText69, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText70 = new wxStaticText(Panel1, ID_STATICTEXT70, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT70"));
    FlexGridSizer15->Add(StaticText70, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button15 = new wxButton(Panel1, ID_BUTTON15, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON15"));
    FlexGridSizer15->Add(Button15, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    Button16 = new wxButton(Panel1, ID_BUTTON16, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON16"));
    FlexGridSizer15->Add(Button16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText19 = new wxStaticText(Panel1, ID_STATICTEXT19, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer12->Add(StaticText19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice2 = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice2->SetSelection( Choice2->Append(_("Circle")) );
    Choice2->Append(_("Rectangle"));
    FlexGridSizer12->Add(Choice2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18->Add(StaticBoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Right"));
    FlexGridSizer16 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticBoxSizer7 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Increase"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 6, 0, 0);
    StaticText48 = new wxStaticText(Panel1, ID_STATICTEXT48, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer2->Add(StaticText48, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText49 = new wxStaticText(Panel1, ID_STATICTEXT49, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer2->Add(StaticText49, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText50 = new wxStaticText(Panel1, ID_STATICTEXT50, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    FlexGridSizer2->Add(StaticText50, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText51 = new wxStaticText(Panel1, ID_STATICTEXT51, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer2->Add(StaticText51, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button11 = new wxButton(Panel1, ID_BUTTON11, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer2->Add(Button11, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    Button12 = new wxButton(Panel1, ID_BUTTON12, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer2->Add(Button12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText26 = new wxStaticText(Panel1, ID_STATICTEXT26, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer16->Add(StaticText26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl4 = new wxSpinCtrl(Panel1, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL4"));
    SpinCtrl4->SetValue(_T("0"));
    FlexGridSizer16->Add(SpinCtrl4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText25 = new wxStaticText(Panel1, ID_STATICTEXT25, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer16->Add(StaticText25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl2 = new wxSpinCtrl(Panel1, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL2"));
    SpinCtrl2->SetValue(_T("1"));
    FlexGridSizer16->Add(SpinCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Decrease"));
    FlexGridSizer17 = new wxFlexGridSizer(1, 6, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer17->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer17->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer17->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer17->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button17 = new wxButton(Panel1, ID_BUTTON17, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON17"));
    FlexGridSizer17->Add(Button17, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    Button18 = new wxButton(Panel1, ID_BUTTON18, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON18"));
    FlexGridSizer17->Add(Button18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText23 = new wxStaticText(Panel1, ID_STATICTEXT23, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer16->Add(StaticText23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice3 = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    Choice3->SetSelection( Choice3->Append(_("Circle")) );
    Choice3->Append(_("Rectangle"));
    FlexGridSizer16->Add(Choice3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    StaticText4 = new wxStaticText(Panel2, ID_STATICTEXT4, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText29 = new wxStaticText(Panel2, ID_STATICTEXT29, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    FlexGridSizer1->Add(StaticText29, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(Panel2, ID_STATICTEXT10, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(Panel2, ID_STATICTEXT5, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(Panel2, ID_STATICTEXT6, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(Panel2, ID_STATICTEXT8, _("Threshold"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText20 = new wxStaticText(Panel2, ID_STATICTEXT20, wxEmptyString, wxDefaultPosition, wxSize(31,21), 0, _T("ID_STATICTEXT20"));
    FlexGridSizer1->Add(StaticText20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(Panel2, ID_STATICTEXT7, _("Button"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText38 = new wxStaticText(Panel2, ID_STATICTEXT38, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer1->Add(StaticText38, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(Panel2, ID_STATICTEXT30, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer1->Add(StaticText30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText39 = new wxStaticText(Panel2, ID_STATICTEXT39, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer1->Add(StaticText39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice4 = new wxChoice(Panel2, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    Choice4->SetSelection( Choice4->Append(_("button")) );
    Choice4->Append(_("axis up"));
    Choice4->Append(_("axis down"));
    FlexGridSizer1->Add(Choice4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText40 = new wxStaticText(Panel2, ID_STATICTEXT40, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer1->Add(StaticText40, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl3 = new wxTextCtrl(Panel2, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    TextCtrl3->Disable();
    FlexGridSizer1->Add(TextCtrl3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button8 = new wxButton(Panel2, ID_BUTTON8, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer1->Add(Button8, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    Choice5 = new wxChoice(Panel2, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    FlexGridSizer1->Add(Choice5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    Grid1 = new wxGrid(Panel2, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
    Grid1->CreateGrid(0,7);
    Grid1->EnableEditing(false);
    Grid1->EnableGridLines(true);
    Grid1->SetRowLabelSize(25);
    Grid1->SetColLabelValue(0, _("Device type"));
    Grid1->SetColLabelValue(1, _("Device name"));
    Grid1->SetColLabelValue(2, _("Device id"));
    Grid1->SetColLabelValue(3, _("Event type"));
    Grid1->SetColLabelValue(4, _("Event id"));
    Grid1->SetColLabelValue(5, _("Threshold"));
    Grid1->SetColLabelValue(6, _("Button id"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizer8->Add(Grid1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(2, 1, 0, 0);
    Button4 = new wxButton(Panel2, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer6->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button6 = new wxButton(Panel2, ID_BUTTON6, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer6->Add(Button6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button2 = new wxButton(Panel2, ID_BUTTON2, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer6->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer9);
    FlexGridSizer9->Fit(Panel2);
    FlexGridSizer9->SetSizeHints(Panel2);
    Panel3 = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer3 = new wxFlexGridSizer(2, 12, 0, 0);
    StaticText11 = new wxStaticText(Panel3, ID_STATICTEXT11, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer3->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(Panel3, ID_STATICTEXT31, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer3->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText12 = new wxStaticText(Panel3, ID_STATICTEXT12, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer3->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(Panel3, ID_STATICTEXT13, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer3->Add(StaticText13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14 = new wxStaticText(Panel3, ID_STATICTEXT14, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer3->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(Panel3, ID_STATICTEXT21, wxEmptyString, wxDefaultPosition, wxSize(30,17), 0, _T("ID_STATICTEXT21"));
    FlexGridSizer3->Add(StaticText21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText16 = new wxStaticText(Panel3, ID_STATICTEXT16, _("Axis"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer3->Add(StaticText16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText15 = new wxStaticText(Panel3, ID_STATICTEXT15, _("DZ"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    StaticText15->SetToolTip(_("Dead zone"));
    FlexGridSizer3->Add(StaticText15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(Panel3, ID_STATICTEXT17, _("Sens."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    StaticText17->SetToolTip(_("Sensitivity"));
    FlexGridSizer3->Add(StaticText17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText18 = new wxStaticText(Panel3, ID_STATICTEXT18, _("Accel."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    StaticText18->SetToolTip(_("Acceleration"));
    FlexGridSizer3->Add(StaticText18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(Panel3, ID_STATICTEXT22, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer3->Add(StaticText22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText33 = new wxStaticText(Panel3, ID_STATICTEXT33, _("Smoothing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer3->Add(StaticText33, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText41 = new wxStaticText(Panel3, ID_STATICTEXT41, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer3->Add(StaticText41, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText32 = new wxStaticText(Panel3, ID_STATICTEXT32, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer3->Add(StaticText32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText42 = new wxStaticText(Panel3, ID_STATICTEXT42, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer3->Add(StaticText42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice7 = new wxChoice(Panel3, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    Choice7->SetSelection( Choice7->Append(_("axis")) );
    Choice7->Append(_("button"));
    FlexGridSizer3->Add(Choice7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText43 = new wxStaticText(Panel3, ID_STATICTEXT43, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer3->Add(StaticText43, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button9 = new wxButton(Panel3, ID_BUTTON9, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer3->Add(Button9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice8 = new wxChoice(Panel3, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE8"));
    FlexGridSizer3->Add(Choice8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl8 = new wxTextCtrl(Panel3, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    TextCtrl8->Disable();
    TextCtrl8->SetToolTip(_("Dead zone [0..64]"));
    FlexGridSizer3->Add(TextCtrl8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl9 = new wxTextCtrl(Panel3, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(59,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    TextCtrl9->Disable();
    TextCtrl9->SetToolTip(_("Sensitivity [-100.00..100.00]"));
    FlexGridSizer3->Add(TextCtrl9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl10 = new wxTextCtrl(Panel3, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(55,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
    TextCtrl10->Disable();
    TextCtrl10->SetToolTip(_("Acceleration [0.00..2.00]"));
    FlexGridSizer3->Add(TextCtrl10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(Panel3, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice1->SetSelection( Choice1->Append(wxEmptyString) );
    Choice1->Append(_("Circle"));
    Choice1->Append(_("Rectangle"));
    Choice1->Disable();
    FlexGridSizer3->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(1, 2, 0, 0);
    TextCtrl1 = new wxTextCtrl(Panel3, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrl1->Disable();
    TextCtrl1->SetToolTip(_("Buffer size [1..30]"));
    FlexGridSizer11->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl2 = new wxTextCtrl(Panel3, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(41,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrl2->Disable();
    TextCtrl2->SetToolTip(_("Filter [0.00..1.00]"));
    FlexGridSizer11->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton13Click1);
    Connect(ID_BUTTON14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton14Click);
    Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnSpinCtrl3Change);
    Connect(ID_BUTTON15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton15Click);
    Connect(ID_BUTTON16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton16Click);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton11Click1);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton12Click);
    Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnSpinCtrl4Change);
    Connect(ID_BUTTON17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton17Click);
    Connect(ID_BUTTON18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton18Click);
    Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnChoice4Select1);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton8Click);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonAdd1Click);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton6Click);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButtonModifyButton);
    Connect(ID_CHOICE7,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnChoice4Select);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnButton9Click);
    Connect(ID_TEXTCTRL8,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL9,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL10,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrlText);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnChoice1Select);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrlText);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnTextCtrlText);
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
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixaxis_emu_guiFrame::OnAbout);
    //*)

    fillButtonChoice(Choice5);
    fillAxisAxisChoice(Choice8);

    currentController = 0;
    currentConfiguration = 0;

#ifndef WIN32
    if(!getuid())
    {
    	int answer = wxMessageBox(_("It's not recommended to run as root user. Continue?"), _("Confirm"), wxYES_NO);
		if (answer == wxNO)
		{
			exit(0);
		}
    }

    homedir = getpwuid(getuid())->pw_dir;

    string cmd = "";
    cmd.append("test -d ");
    cmd.append(homedir);
    cmd.append("/.emuclient || cp -r /etc/emuclient ");
    cmd.append(homedir);
    cmd.append("/.emuclient");
    if(system(cmd.c_str()) < 0)
    {
        wxMessageBox( wxT("Cannot open emuclient config directory!"), wxT("Error"), wxICON_ERROR);
    }

    cmd.erase();
    cmd.append(homedir);
    cmd.append("/.emuclient/config");

#else
    string cmd = "config";
    MenuItem8->Enable(false);
    MenuItem9->Enable(false);
    MenuItem10->Enable(false);
    MenuItem11->Enable(false);
    MenuItem12->Enable(false);
    MenuItem13->Enable(false);
    MenuItem24->Enable(false);
    MenuItem25->Enable(false);
#endif

    default_directory = wxString(cmd.c_str(), wxConvUTF8);

    FileDialog1->SetDirectory(default_directory);

    Grid1->AutoSizeColumns();
    Grid2->AutoSizeColumns();

    if(!file.IsEmpty())
    {
      wxString wxfile;
#ifndef WIN32
      wxfile.Append(wxString(homedir, wxConvUTF8));
      wxfile.Append(_("/.emuclient/config/"));
#else
      wxfile.Append(_("config/"));
#endif
      wxfile.Append(file);
      if(::wxFileExists(wxfile))
      {
        configFile.ReadConfigFile(wxfile);
        MenuItem30->Check(configFile.MultipleMK());
        wxCommandEvent event;
        OnMenuMultipleMK(event);
        load_current();
        refresh_gui();
        Menu1->Enable(idMenuSave, true);
      }
      else
      {
        wxMessageBox( wxT("Cannot open config file: ") + wxString(file, wxConvUTF8), wxT("Error"), wxICON_ERROR);
      }
    }

    configFile.SetEvCatch(&evcatch);

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
}

void sixaxis_emu_guiFrame::OnButtonAdd1Click(wxCommandEvent& event)
{
    if(StaticText40->GetLabel() == wxEmptyString)
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(Choice5->GetStringSelection() == wxEmptyString)
    {
        wxMessageBox( wxT("Please select a Button!"), wxT("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(StaticText38->GetLabel(), StaticText30->GetLabel(), StaticText39->GetLabel(), Choice4->GetStringSelection(), StaticText40->GetLabel(), -1, -1);

    if(!check.IsEmpty())
    {
      int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        return;
      }
    }

    Grid1->InsertRows();
    Grid1->SetCellValue(0, 0, StaticText38->GetLabel());
    Grid1->SetCellValue(0, 1, StaticText30->GetLabel());
    Grid1->SetCellValue(0, 2, StaticText39->GetLabel());
    Grid1->SetCellValue(0, 3, Choice4->GetStringSelection());
    Grid1->SetCellValue(0, 4, StaticText40->GetLabel());
    Grid1->SetCellValue(0, 5, TextCtrl3->GetValue());
    Grid1->SetCellValue(0, 6, Choice5->GetStringSelection());
    Grid1->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton3Click(wxCommandEvent& event)
{
    if(StaticText43->GetLabel() == wxEmptyString)
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(Choice8->GetStringSelection() == wxEmptyString)
    {
        wxMessageBox( wxT("Please select an Axis!"), wxT("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(StaticText41->GetLabel(), StaticText32->GetLabel(), StaticText42->GetLabel(), Choice7->GetStringSelection(), StaticText43->GetLabel(), -1, -1);

    if(!check.IsEmpty())
    {
      int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        return;
      }
    }

    Grid2->InsertRows();
    Grid2->SetCellValue(0, 0, StaticText41->GetLabel());
    Grid2->SetCellValue(0, 1, StaticText32->GetLabel());
    Grid2->SetCellValue(0, 2, StaticText42->GetLabel());
    Grid2->SetCellValue(0, 3, Choice7->GetStringSelection());
    Grid2->SetCellValue(0, 4, StaticText43->GetLabel());
    Grid2->SetCellValue(0, 5, Choice8->GetStringSelection());
    Grid2->SetCellValue(0, 6, TextCtrl8->GetValue());
    Grid2->SetCellValue(0, 7, TextCtrl9->GetValue());
    Grid2->SetCellValue(0, 8, TextCtrl10->GetValue());
    Grid2->SetCellValue(0, 9, Choice1->GetStringSelection());
    Grid2->SetCellValue(0, 10, TextCtrl1->GetValue());
    Grid2->SetCellValue(0, 11, TextCtrl2->GetValue());
    Grid2->AutoSizeColumns();
    refresh_gui();
}

void sixaxis_emu_guiFrame::DeleteLinkedRows(wxGrid* grid, int row)
{
  if(grid == Grid1)
  {
    wxString old_device_type = Grid1->GetCellValue(row, 0);
    wxString old_device_name = Grid1->GetCellValue(row, 1);
    wxString old_device_id = Grid1->GetCellValue(row, 2);
    wxString old_event_type = Grid1->GetCellValue(row, 3);
    wxString old_event_id = Grid1->GetCellValue(row, 4);
    wxString old_button_id = Grid1->GetCellValue(row, 6);

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

void sixaxis_emu_guiFrame::OnButton6Click(wxCommandEvent& event)
{
    sixaxis_emu_guiFrame::DeleteSelectedRows(Grid1);
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton7Click(wxCommandEvent& event)
{
    sixaxis_emu_guiFrame::DeleteSelectedRows(Grid2);
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnChoice4Select(wxCommandEvent& event)
{
    StaticText41->SetLabel(wxEmptyString);
    StaticText32->SetLabel(wxEmptyString);
    StaticText42->SetLabel(wxEmptyString);
    StaticText43->SetLabel(wxEmptyString);
    TextCtrl8->Disable();
	  TextCtrl8->SetValue(wxEmptyString);
  	TextCtrl9->Disable();
	  TextCtrl9->SetValue(wxEmptyString);
	  TextCtrl10->Disable();
	  TextCtrl10->SetValue(wxEmptyString);
	  Choice1->SetSelection(0);
	  Choice1->Disable();
	  TextCtrl1->Disable();
	  TextCtrl1->SetValue(wxEmptyString);
	  TextCtrl2->Disable();
	  TextCtrl2->SetValue(wxEmptyString);

    if(Choice7->GetStringSelection() == _("button"))
    {
        fillButtonAxisChoice(Choice8);
    }
    else
    {
        fillAxisAxisChoice(Choice8);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnChoice4Select1(wxCommandEvent& event)
{
    StaticText38->SetLabel(wxEmptyString);
    StaticText30->SetLabel(wxEmptyString);
    StaticText39->SetLabel(wxEmptyString);
    StaticText40->SetLabel(wxEmptyString);

    if(Choice4->GetStringSelection() == _("button"))
    {
        TextCtrl3->Disable();
        TextCtrl3->SetValue(wxEmptyString);
    }
    else
    {
        TextCtrl3->Enable();
        TextCtrl3->SetValue(_("10"));
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

    wxString old_device_type = StaticText35->GetLabel();
    wxString old_device_name = StaticText27->GetLabel();
    wxString old_device_id = StaticText36->GetLabel();
    wxString old_event_id = StaticText37->GetLabel();
    Device* dev;
    Event* ev;

    auto_detect(StaticText35, StaticText27, StaticText36, _("button"), StaticText37);

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
          dev->SetType(StaticText35->GetLabel());
          dev->SetName(StaticText27->GetLabel());
          dev->SetId(StaticText36->GetLabel());
          ev->SetId(StaticText37->GetLabel());
        }
      }
    }

    Button1->Enable(true);

    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton8Click(wxCommandEvent& event)
{
    Button8->Enable(false);

    auto_detect(StaticText38, StaticText30, StaticText39, Choice4->GetStringSelection(), StaticText40);

    if(evcatch.GetEventType() == _("button"))
    {
        TextCtrl3->Disable();
        TextCtrl3->SetValue(wxEmptyString);
    }
    else
    {
        TextCtrl3->Enable();
        TextCtrl3->SetValue(_("10"));
    }

    fillButtonChoice(Choice5);

    refresh_gui();

    Button8->Enable(true);
}

void sixaxis_emu_guiFrame::OnButton9Click(wxCommandEvent& event)
{
    wxString old_device_type = StaticText41->GetLabel();
    wxString old_device_name = StaticText32->GetLabel();
    wxString old_device_id = StaticText42->GetLabel();

    Button9->Enable(false);

    auto_detect(StaticText41, StaticText32, StaticText42, Choice7->GetStringSelection(), StaticText43);

  	if(old_device_type != StaticText41->GetLabel()
       || old_device_name != StaticText32->GetLabel()
       || old_device_id != StaticText42->GetLabel())
    {
        if(evcatch.GetEventType() == _("button"))
        {
            TextCtrl8->Disable();
            TextCtrl8->SetValue(wxEmptyString);
            TextCtrl9->Disable();
            TextCtrl9->SetValue(wxEmptyString);
            TextCtrl10->Disable();
            TextCtrl10->SetValue(wxEmptyString);
            Choice1->Disable();
            Choice1->SetSelection(0);
            fillButtonAxisChoice(Choice8);
        }
        else
        {
      	  TextCtrl8->Enable();
          TextCtrl9->Enable();
          TextCtrl10->Enable();
          TextCtrl10->SetValue(_("1.00"));
          Choice1->Enable();
          Choice1->SetSelection(1);
          if(evcatch.GetDeviceType() == _("mouse"))
          {
              TextCtrl8->SetValue(_("20"));
              TextCtrl9->SetValue(_("1.00"));
              TextCtrl1->Enable();
              TextCtrl1->SetValue(_("1"));
              TextCtrl2->Enable();
              TextCtrl2->SetValue(_("0.00"));
          }
          else if(evcatch.GetDeviceType() == _("joystick"))
          {
              TextCtrl8->SetValue(_("0"));
              if(!Choice8->GetStringSelection().Contains(_("stick")))
              {
                  TextCtrl9->SetValue(_("0.008"));
              }
              else
              {
                  TextCtrl9->SetValue(_("0.004"));
              }
        	    TextCtrl1->Disable();
              TextCtrl1->SetValue(wxEmptyString);
              TextCtrl2->Disable();
              TextCtrl2->SetValue(wxEmptyString);
          }
          fillAxisAxisChoice(Choice8);
        }
    }

    refresh_gui();

    Button9->Enable(true);
}

void sixaxis_emu_guiFrame::save_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Save Trigger
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetType(StaticText35->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetName(StaticText27->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetId(StaticText36->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->SetId(StaticText37->GetLabel());
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetSwitchBack(CheckBox1->GetValue()?_("yes"):_("no"));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetDelay(SpinCtrl5->GetValue());
    //Save Intensity
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    intensityList->erase(intensityList->begin(), intensityList->end());
    //Save left stick Intensity
    intensityList->push_back(Intensity(_("left_stick"),
        StaticText58->GetLabel(), StaticText60->GetLabel(), StaticText59->GetLabel(), StaticText61->GetLabel(),
        StaticText67->GetLabel(), StaticText69->GetLabel(), StaticText68->GetLabel(), StaticText70->GetLabel(),
        SpinCtrl3->GetValue(), SpinCtrl1->GetValue(), Choice2->GetStringSelection()));
    //Save right stick intensity
    intensityList->push_back(Intensity(_("right_stick"),
        StaticText48->GetLabel(), StaticText50->GetLabel(), StaticText49->GetLabel(), StaticText51->GetLabel(),
        StaticText1->GetLabel(), StaticText3->GetLabel(), StaticText2->GetLabel(), StaticText9->GetLabel(),
        SpinCtrl4->GetValue(), SpinCtrl2->GetValue(), Choice3->GetStringSelection()));
    //Save ButtonMappers
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    buttonMappers->erase(buttonMappers->begin(), buttonMappers->end());
    for(int i=0; i<Grid1->GetNumberRows(); i++)
    {
        //ButtonMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString threshold, wxString button)
        buttonMappers->push_front(ButtonMapper(Grid1->GetCellValue(i, 0), Grid1->GetCellValue(i, 2), Grid1->GetCellValue(i, 1), Grid1->GetCellValue(i, 3), Grid1->GetCellValue(i, 4), Grid1->GetCellValue(i, 5), Grid1->GetCellValue(i, 6)));
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    axisMappers->erase(axisMappers->begin(), axisMappers->end());
    for(int i=0; i<Grid2->GetNumberRows(); i++)
    {
        //AxisMapper(wxString dtype, wxString did, wxString dname, wxString etype, wxString eid, wxString axis, wxString deadZone, wxString multiplier, wxString exponent);
        axisMappers->push_front(AxisMapper(Grid2->GetCellValue(i, 0), Grid2->GetCellValue(i, 2), Grid2->GetCellValue(i, 1), Grid2->GetCellValue(i, 3), Grid2->GetCellValue(i, 4), Grid2->GetCellValue(i, 5), Grid2->GetCellValue(i, 6), Grid2->GetCellValue(i, 7), Grid2->GetCellValue(i, 8), Grid2->GetCellValue(i, 9), Grid2->GetCellValue(i, 10), Grid2->GetCellValue(i, 11)));
    }

}

void sixaxis_emu_guiFrame::load_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Load Trigger
    StaticText35->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetType());
    StaticText27->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetName());
    StaticText36->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetId());
    StaticText37->SetLabel(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->GetId());
    if(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetSwitchBack() == _("yes"))
    {
        CheckBox1->SetValue(true);
    }
    else
    {
        CheckBox1->SetValue(false);
    }
    SpinCtrl5->SetValue(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDelay());
    //Load left & right stick intensities
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    StaticText58->SetLabel(wxEmptyString);
    StaticText59->SetLabel(wxEmptyString);
    StaticText60->SetLabel(wxEmptyString);
    StaticText61->SetLabel(wxEmptyString);
    StaticText67->SetLabel(wxEmptyString);
    StaticText68->SetLabel(wxEmptyString);
    StaticText69->SetLabel(wxEmptyString);
    StaticText70->SetLabel(wxEmptyString);
    SpinCtrl3->SetValue(0);
    SpinCtrl1->SetValue(1);
    Choice2->SetSelection(0);
    StaticText48->SetLabel(wxEmptyString);
    StaticText49->SetLabel(wxEmptyString);
    StaticText50->SetLabel(wxEmptyString);
    StaticText51->SetLabel(wxEmptyString);
    StaticText1->SetLabel(wxEmptyString);
    StaticText2->SetLabel(wxEmptyString);
    StaticText3->SetLabel(wxEmptyString);
    StaticText9->SetLabel(wxEmptyString);
    SpinCtrl4->SetValue(0);
    SpinCtrl2->SetValue(1);
    Choice3->SetSelection(0);
    for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); it++)
    {
      if(it->GetControl() == _("left_stick"))
      {
        StaticText58->SetLabel(it->GetDeviceUp()->GetType());
        StaticText59->SetLabel(it->GetDeviceUp()->GetName());
        StaticText60->SetLabel(it->GetDeviceUp()->GetId());
        StaticText61->SetLabel(it->GetEventUp()->GetId());
        StaticText67->SetLabel(it->GetDeviceDown()->GetType());
        StaticText68->SetLabel(it->GetDeviceDown()->GetName());
        StaticText69->SetLabel(it->GetDeviceDown()->GetId());
        StaticText70->SetLabel(it->GetEventDown()->GetId());
        SpinCtrl3->SetValue(it->GetDeadZone());
        SpinCtrl1->SetValue(it->GetSteps());
        Choice2->SetSelection(Choice2->FindString(it->GetShape()));
      }
      else if(it->GetControl() == _("right_stick"))
      {
        StaticText48->SetLabel(it->GetDeviceUp()->GetType());
        StaticText49->SetLabel(it->GetDeviceUp()->GetName());
        StaticText50->SetLabel(it->GetDeviceUp()->GetId());
        StaticText51->SetLabel(it->GetEventUp()->GetId());
        StaticText1->SetLabel(it->GetDeviceDown()->GetType());
        StaticText2->SetLabel(it->GetDeviceDown()->GetName());
        StaticText3->SetLabel(it->GetDeviceDown()->GetId());
        StaticText9->SetLabel(it->GetEventDown()->GetId());
        SpinCtrl4->SetValue(it->GetDeadZone());
        SpinCtrl2->SetValue(it->GetSteps());
        Choice3->SetSelection(Choice3->FindString(it->GetShape()));
      }
    }
    //Load ButtonMappers
    Grid1->DeleteRows(0, Grid1->GetNumberRows());
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); it++)
    {
        Grid1->InsertRows();
        Grid1->SetCellValue(0, 0, it->GetDevice()->GetType());
        Grid1->SetCellValue(0, 1, it->GetDevice()->GetName());
        Grid1->SetCellValue(0, 2, it->GetDevice()->GetId());
        Grid1->SetCellValue(0, 3, it->GetEvent()->GetType());
        Grid1->SetCellValue(0, 4, it->GetEvent()->GetId());
        Grid1->SetCellValue(0, 5, it->GetEvent()->GetThreshold());
        Grid1->SetCellValue(0, 6, it->GetButton());
        Grid1->AutoSizeColumns();
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
    OnMenuMultipleMK(event);

    currentController = 0;
    currentConfiguration = 0;
    Menu3->Check(ID_MENUITEM1, true);
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    refresh_gui();
    Menu1->Enable(idMenuSave, true);
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
}

void sixaxis_emu_guiFrame::OnMenuItemController1(wxCommandEvent& event)
{
    save_current();
    currentController = 0;
    currentConfiguration = 0;
    Menu4->Check(ID_MENUITEM8, true);
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration1(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 0;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration2(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 1;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration3(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 2;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration4(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 3;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration5(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 4;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration6(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 5;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration7(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 6;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuItemConfiguration8(wxCommandEvent& event)
{
    save_current();
    currentConfiguration = 7;
    load_current();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
    Button7->Enable();
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnMenuSave(wxCommandEvent& event)
{
    wxString end;
    save_current();
    configFile.WriteConfigFile();
    Button2->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    Button4->Enable();
    Button6->Enable();
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
    wxArrayInt array = Grid1->GetSelectedRows();
    int count = array.GetCount();

    if(Button2->GetLabel() == _("Modify"))
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

        StaticText38->SetLabel(Grid1->GetCellValue(grid1mod, 0));
        StaticText30->SetLabel(Grid1->GetCellValue(grid1mod, 1));
        StaticText39->SetLabel(Grid1->GetCellValue(grid1mod, 2));
        Choice4->SetSelection(Choice4->FindString(Grid1->GetCellValue(grid1mod, 3)));
        StaticText40->SetLabel(Grid1->GetCellValue(grid1mod, 4));
        TextCtrl3->SetValue(Grid1->GetCellValue(grid1mod, 5));
        fillButtonChoice(Choice5);
        Choice5->SetSelection(Choice5->FindString(Grid1->GetCellValue(grid1mod, 6)));
        Button4->Disable();
        Button6->Disable();
        Button2->SetLabel(_("Apply"));
    }
    else
    {
        if (StaticText40->GetLabel() == wxEmptyString)
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(Choice5->GetStringSelection() == wxEmptyString)
        {
            wxMessageBox( wxT("Please select a Button!"), wxT("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(StaticText38->GetLabel(), StaticText30->GetLabel(), StaticText39->GetLabel(), Choice4->GetStringSelection(), StaticText40->GetLabel(), grid1mod, -1);

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
        Grid1->SetCellValue(grid1mod, 0, StaticText38->GetLabel());
        Grid1->SetCellValue(grid1mod, 1, StaticText30->GetLabel());
        Grid1->SetCellValue(grid1mod, 2, StaticText39->GetLabel());
        Grid1->SetCellValue(grid1mod, 3, Choice4->GetStringSelection());
        Grid1->SetCellValue(grid1mod, 4, StaticText40->GetLabel());
        Grid1->SetCellValue(grid1mod, 5, TextCtrl3->GetValue());
        Grid1->SetCellValue(grid1mod, 6, Choice5->GetStringSelection());
        Button4->Enable();
        Button6->Enable();
        Button2->SetLabel(_("Modify"));
    }
    Grid1->AutoSizeColumns();
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
          if(it->GetDevice()->GetType() == Grid1->GetCellValue(grid1mod, 0)
              && it->GetDevice()->GetName() == Grid1->GetCellValue(grid1mod, 1)
              && it->GetDevice()->GetId() == Grid1->GetCellValue(grid1mod, 2)
              && it->GetEvent()->GetType() == Grid1->GetCellValue(grid1mod, 3)
              && it->GetEvent()->GetId() == Grid1->GetCellValue(grid1mod, 4)
              && it->GetButton() == Grid1->GetCellValue(grid1mod, 6))
          {
              it->GetDevice()->SetType(StaticText38->GetLabel());
              it->GetDevice()->SetId(StaticText39->GetLabel());
              it->GetDevice()->SetName(StaticText30->GetLabel());
              it->GetEvent()->SetType(Choice4->GetStringSelection());
              it->GetEvent()->SetId(StaticText40->GetLabel());
              it->GetEvent()->SetThreshold(TextCtrl3->GetValue());
              it->SetButton(Choice5->GetStringSelection());
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

        StaticText41->SetLabel(Grid2->GetCellValue(grid2mod, 0));
        StaticText32->SetLabel(Grid2->GetCellValue(grid2mod, 1));
        StaticText42->SetLabel(Grid2->GetCellValue(grid2mod, 2));
        Choice7->SetSelection(Choice7->FindString(Grid2->GetCellValue(grid2mod, 3)));
        StaticText43->SetLabel(Grid2->GetCellValue(grid2mod, 4));
        TextCtrl8->SetValue(Grid2->GetCellValue(grid2mod, 6));
        TextCtrl9->SetValue(Grid2->GetCellValue(grid2mod, 7));
        TextCtrl10->SetValue(Grid2->GetCellValue(grid2mod, 8));
        TextCtrl1->SetValue(Grid2->GetCellValue(grid2mod, 10));
        TextCtrl2->SetValue(Grid2->GetCellValue(grid2mod, 11));
        if(Choice7->GetStringSelection() == _("button"))
        {
            TextCtrl8->Disable();
            TextCtrl9->Disable();
            TextCtrl10->Disable();
            Choice1->Disable();
            Choice1->SetSelection(0);
            TextCtrl1->Disable();
            TextCtrl2->Disable();
            fillButtonAxisChoice(Choice8);
        }
        else
        {
            TextCtrl8->Enable();
            TextCtrl9->Enable();
            TextCtrl10->Enable();
            Choice1->SetSelection(Choice1->FindString(Grid2->GetCellValue(grid2mod, 9)));
            Choice1->Enable();
			      if(StaticText41->GetLabel() == _("mouse"))
			      {
                TextCtrl1->Enable();
                TextCtrl2->Enable();
      			}
            fillAxisAxisChoice(Choice8);
        }
        Choice8->SetSelection(Choice8->FindString(Grid2->GetCellValue(grid2mod, 5)));
        Button3->Disable();
        Button7->Disable();
        Button5->SetLabel(_("Apply"));
    }
    else
    {
        if (StaticText43->GetLabel() == wxEmptyString)
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(Choice8->GetStringSelection() == wxEmptyString)
        {
            wxMessageBox( wxT("Please select an Axis!"), wxT("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(StaticText41->GetLabel(), StaticText32->GetLabel(), StaticText42->GetLabel(), Choice7->GetStringSelection(), StaticText43->GetLabel(), -1, grid2mod);

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
        Grid2->SetCellValue(grid2mod, 0, StaticText41->GetLabel());
        Grid2->SetCellValue(grid2mod, 1, StaticText32->GetLabel());
        Grid2->SetCellValue(grid2mod, 2, StaticText42->GetLabel());
        Grid2->SetCellValue(grid2mod, 3, Choice7->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 4, StaticText43->GetLabel());
        Grid2->SetCellValue(grid2mod, 5, Choice8->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 6, TextCtrl8->GetValue());
        Grid2->SetCellValue(grid2mod, 7, TextCtrl9->GetValue());
        Grid2->SetCellValue(grid2mod, 8, TextCtrl10->GetValue());
        Grid2->SetCellValue(grid2mod, 9, Choice1->GetStringSelection());
        Grid2->SetCellValue(grid2mod, 10, TextCtrl1->GetValue());
        Grid2->SetCellValue(grid2mod, 11, TextCtrl2->GetValue());
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
              it->GetDevice()->SetType(StaticText41->GetLabel());
              it->GetDevice()->SetId(StaticText42->GetLabel());
              it->GetDevice()->SetName(StaticText32->GetLabel());
              it->GetEvent()->SetType(Choice7->GetStringSelection());
              it->GetEvent()->SetId(StaticText43->GetLabel());
              it->SetAxis(Choice8->GetStringSelection());
          }
      }
    }
}

void sixaxis_emu_guiFrame::OnChoice1Select(wxCommandEvent& event)
{
    if(Choice7->GetStringSelection() == _("axis"))
    {
        if(Choice1->GetStringSelection() == wxEmptyString)
        {
            Choice1->SetSelection(1);
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
  Button2->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  Button3->Enable();
  Button4->Enable();
  Button6->Enable();
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
  Button2->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  Button3->Enable();
  Button4->Enable();
  Button6->Enable();
  Button7->Enable();
}

void sixaxis_emu_guiFrame::OnButton10Click1(wxCommandEvent& event)
{
    StaticText35->SetLabel(wxEmptyString);
    StaticText27->SetLabel(wxEmptyString);
    StaticText36->SetLabel(wxEmptyString);
    StaticText37->SetLabel(wxEmptyString);
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

    evcatch.run(device_type, _("button"));
    if(MenuItem30->IsChecked())
    {
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

void sixaxis_emu_guiFrame::OnButton13Click1(wxCommandEvent& event)
{
  Button13->Enable(false);

  wxString old_device_type = StaticText58->GetLabel();
  wxString old_device_name = StaticText59->GetLabel();
  wxString old_device_id = StaticText60->GetLabel();
  wxString old_event_id = StaticText61->GetLabel();

  auto_detect(StaticText58, StaticText59, StaticText60, _("button"), StaticText61);

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
            dev->SetType(StaticText58->GetLabel());
            dev->SetName(StaticText59->GetLabel());
            dev->SetId(StaticText60->GetLabel());
            ev->SetId(StaticText61->GetLabel());
          }
        }
      }
    }
  }

  Button13->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton14Click(wxCommandEvent& event)
{
  wxString old_device_type = StaticText58->GetLabel();
  wxString old_device_name = StaticText59->GetLabel();
  wxString old_device_id = StaticText60->GetLabel();
  wxString old_event_id = StaticText61->GetLabel();

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

  StaticText58->SetLabel(wxEmptyString);
  StaticText59->SetLabel(wxEmptyString);
  StaticText60->SetLabel(wxEmptyString);
  StaticText61->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton11Click1(wxCommandEvent& event)
{
  Button11->Enable(false);

  wxString old_device_type = StaticText48->GetLabel();
  wxString old_device_name = StaticText49->GetLabel();
  wxString old_device_id = StaticText50->GetLabel();
  wxString old_event_id = StaticText51->GetLabel();

  auto_detect(StaticText48, StaticText49, StaticText50, _("button"), StaticText51);

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
            dev->SetType(StaticText48->GetLabel());
            dev->SetName(StaticText49->GetLabel());
            dev->SetId(StaticText50->GetLabel());
            ev->SetId(StaticText51->GetLabel());
          }
        }
      }
    }
  }

  Button11->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton12Click(wxCommandEvent& event)
{
  wxString old_device_type = StaticText48->GetLabel();
  wxString old_device_name = StaticText49->GetLabel();
  wxString old_device_id = StaticText50->GetLabel();
  wxString old_event_id = StaticText51->GetLabel();

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

  StaticText48->SetLabel(wxEmptyString);
  StaticText49->SetLabel(wxEmptyString);
  StaticText50->SetLabel(wxEmptyString);
  StaticText51->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnSpinCtrl3Change(wxSpinEvent& event)
{
    int max = 127-SpinCtrl3->GetValue();
    if(max)
    {
        SpinCtrl1->SetValue(SpinCtrl1->GetValue()*SpinCtrl1->GetMax()/max);
        SpinCtrl1->SetRange(1, max);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnSpinCtrl4Change(wxSpinEvent& event)
{
    int max = 127-SpinCtrl4->GetValue();
    if(max)
    {
        SpinCtrl2->SetValue(SpinCtrl2->GetValue()*SpinCtrl2->GetMax()/max);
        SpinCtrl2->SetRange(1, max);
    }
    refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton15Click(wxCommandEvent& event)
{
  Button15->Enable(false);

  wxString old_device_type = StaticText67->GetLabel();
  wxString old_device_name = StaticText68->GetLabel();
  wxString old_device_id = StaticText69->GetLabel();
  wxString old_event_id = StaticText70->GetLabel();

  auto_detect(StaticText67, StaticText68, StaticText69, _("button"), StaticText70);

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
            dev->SetType(StaticText67->GetLabel());
            dev->SetName(StaticText68->GetLabel());
            dev->SetId(StaticText69->GetLabel());
            ev->SetId(StaticText70->GetLabel());
          }
        }
      }
    }
  }

  Button15->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton16Click(wxCommandEvent& event)
{
  wxString old_device_type = StaticText67->GetLabel();
  wxString old_device_name = StaticText68->GetLabel();
  wxString old_device_id = StaticText69->GetLabel();
  wxString old_event_id = StaticText70->GetLabel();

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

  StaticText67->SetLabel(wxEmptyString);
  StaticText68->SetLabel(wxEmptyString);
  StaticText69->SetLabel(wxEmptyString);
  StaticText70->SetLabel(wxEmptyString);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton17Click(wxCommandEvent& event)
{
  Button17->Enable(false);

  wxString old_device_type = StaticText1->GetLabel();
  wxString old_device_name = StaticText2->GetLabel();
  wxString old_device_id = StaticText3->GetLabel();
  wxString old_event_id = StaticText9->GetLabel();

  auto_detect(StaticText1, StaticText2, StaticText3, _("button"), StaticText9);

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
            dev->SetType(StaticText1->GetLabel());
            dev->SetName(StaticText2->GetLabel());
            dev->SetId(StaticText3->GetLabel());
            ev->SetId(StaticText9->GetLabel());
          }
        }
      }
    }
  }

  Button17->Enable(true);

  refresh_gui();
}

void sixaxis_emu_guiFrame::OnButton18Click(wxCommandEvent& event)
{
  wxString old_device_type = StaticText1->GetLabel();
  wxString old_device_name = StaticText2->GetLabel();
  wxString old_device_id = StaticText3->GetLabel();
  wxString old_event_id = StaticText9->GetLabel();

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

  StaticText1->SetLabel(wxEmptyString);
  StaticText2->SetLabel(wxEmptyString);
  StaticText3->SetLabel(wxEmptyString);
  StaticText9->SetLabel(wxEmptyString);

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

void sixaxis_emu_guiFrame::OnTextCtrlText(wxCommandEvent& event)
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

    if(text == TextCtrl8)
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
    else if(text == TextCtrl9)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(_("1.00"));
        }
    }
    else if(text == TextCtrl10)
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
    else if(text == TextCtrl1)
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
    else if(text == TextCtrl2)
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
  for(int i=0; i<Grid1->GetNumberRows(); i++)
  {
    if(gridIndex1 >= 0 && i == gridIndex1)
    {
      continue;
    }

    if(Grid1->GetCellValue(i, 0) == device_type
       && (Grid1->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || Grid1->GetCellValue(i, 1) == device_name)
       && Grid1->GetCellValue(i, 2) == device_id
       && Grid1->GetCellValue(i, 3) == event_type
       && Grid1->GetCellValue(i, 4) == event_id)
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
    if(device_type == StaticText58->GetLabel()
       && (StaticText59->GetLabel().IsEmpty() || device_name.IsEmpty() || StaticText59->GetLabel() == device_name)
       && StaticText60->GetLabel() == device_id
       && StaticText61->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Left > Increase.\nContinue?");
    }
    if(device_type == StaticText67->GetLabel()
       && (StaticText68->GetLabel().IsEmpty() || device_name.IsEmpty() || StaticText68->GetLabel() == device_name)
       && StaticText69->GetLabel() == device_id
       && StaticText70->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Left > Decrease.\nContinue?");
    }

    if(device_type == StaticText48->GetLabel()
       && (StaticText49->GetLabel().IsEmpty() || device_name.IsEmpty() || StaticText49->GetLabel() == device_name)
       && StaticText50->GetLabel() == device_id
       && StaticText51->GetLabel() == event_id)
    {
      return _("This control is already used in the current profile:\nOverall tab > Stick intensity > Right > Increase.\nContinue?");
    }
    if(device_type == StaticText1->GetLabel()
       && (StaticText2->GetLabel().IsEmpty() || device_name.IsEmpty() || StaticText2->GetLabel() == device_name)
       && StaticText3->GetLabel() == device_id
       && StaticText4->GetLabel() == event_id)
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
}
