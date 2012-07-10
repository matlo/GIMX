/***************************************************************
 * Name:      configMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2010-11-09
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "configMain.h"
#include <wx/msgdlg.h>
#include <stdio.h>
#include <sys/types.h>
#include <SDL/SDL.h>
#include "wx/numdlg.h"
#include <math.h>
#include <sstream>
#include <iomanip>

//(*InternalHeaders(configFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/aboutdlg.h>
#include "config.h"
#include <locale.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include <algorithm>

#include "../shared/updater/updater.h"
#include "../directories.h"

using namespace std;

//(*IdInit(configFrame)
const long configFrame::ID_STATICTEXT35 = wxNewId();
const long configFrame::ID_STATICTEXT27 = wxNewId();
const long configFrame::ID_STATICTEXT36 = wxNewId();
const long configFrame::ID_STATICTEXT37 = wxNewId();
const long configFrame::ID_BUTTON1 = wxNewId();
const long configFrame::ID_CHECKBOX1 = wxNewId();
const long configFrame::ID_STATICTEXT28 = wxNewId();
const long configFrame::ID_SPINCTRL5 = wxNewId();
const long configFrame::ID_BUTTON10 = wxNewId();
const long configFrame::ID_STATICTEXT58 = wxNewId();
const long configFrame::ID_STATICTEXT59 = wxNewId();
const long configFrame::ID_STATICTEXT60 = wxNewId();
const long configFrame::ID_STATICTEXT61 = wxNewId();
const long configFrame::ID_BUTTON13 = wxNewId();
const long configFrame::ID_BUTTON14 = wxNewId();
const long configFrame::ID_STATICTEXT24 = wxNewId();
const long configFrame::ID_SPINCTRL3 = wxNewId();
const long configFrame::ID_STATICTEXT62 = wxNewId();
const long configFrame::ID_SPINCTRL1 = wxNewId();
const long configFrame::ID_STATICTEXT67 = wxNewId();
const long configFrame::ID_STATICTEXT68 = wxNewId();
const long configFrame::ID_STATICTEXT69 = wxNewId();
const long configFrame::ID_STATICTEXT70 = wxNewId();
const long configFrame::ID_BUTTON15 = wxNewId();
const long configFrame::ID_BUTTON16 = wxNewId();
const long configFrame::ID_STATICTEXT19 = wxNewId();
const long configFrame::ID_CHOICE2 = wxNewId();
const long configFrame::ID_STATICTEXT48 = wxNewId();
const long configFrame::ID_STATICTEXT49 = wxNewId();
const long configFrame::ID_STATICTEXT50 = wxNewId();
const long configFrame::ID_STATICTEXT51 = wxNewId();
const long configFrame::ID_BUTTON11 = wxNewId();
const long configFrame::ID_BUTTON12 = wxNewId();
const long configFrame::ID_STATICTEXT26 = wxNewId();
const long configFrame::ID_SPINCTRL4 = wxNewId();
const long configFrame::ID_STATICTEXT25 = wxNewId();
const long configFrame::ID_SPINCTRL2 = wxNewId();
const long configFrame::ID_STATICTEXT1 = wxNewId();
const long configFrame::ID_STATICTEXT2 = wxNewId();
const long configFrame::ID_STATICTEXT3 = wxNewId();
const long configFrame::ID_STATICTEXT9 = wxNewId();
const long configFrame::ID_BUTTON17 = wxNewId();
const long configFrame::ID_BUTTON18 = wxNewId();
const long configFrame::ID_STATICTEXT23 = wxNewId();
const long configFrame::ID_CHOICE3 = wxNewId();
const long configFrame::ID_PANEL1 = wxNewId();
const long configFrame::ID_STATICTEXT4 = wxNewId();
const long configFrame::ID_STATICTEXT29 = wxNewId();
const long configFrame::ID_STATICTEXT10 = wxNewId();
const long configFrame::ID_STATICTEXT5 = wxNewId();
const long configFrame::ID_STATICTEXT6 = wxNewId();
const long configFrame::ID_STATICTEXT8 = wxNewId();
const long configFrame::ID_STATICTEXT20 = wxNewId();
const long configFrame::ID_STATICTEXT7 = wxNewId();
const long configFrame::ID_STATICTEXT34 = wxNewId();
const long configFrame::ID_STATICTEXT38 = wxNewId();
const long configFrame::ID_STATICTEXT30 = wxNewId();
const long configFrame::ID_STATICTEXT39 = wxNewId();
const long configFrame::ID_CHOICE4 = wxNewId();
const long configFrame::ID_STATICTEXT40 = wxNewId();
const long configFrame::ID_TEXTCTRL3 = wxNewId();
const long configFrame::ID_BUTTON8 = wxNewId();
const long configFrame::ID_CHOICE5 = wxNewId();
const long configFrame::ID_COMBOBOX1 = wxNewId();
const long configFrame::ID_GRID1 = wxNewId();
const long configFrame::ID_BUTTON4 = wxNewId();
const long configFrame::ID_BUTTON6 = wxNewId();
const long configFrame::ID_BUTTON2 = wxNewId();
const long configFrame::ID_PANEL2 = wxNewId();
const long configFrame::ID_STATICTEXT11 = wxNewId();
const long configFrame::ID_STATICTEXT31 = wxNewId();
const long configFrame::ID_STATICTEXT12 = wxNewId();
const long configFrame::ID_STATICTEXT13 = wxNewId();
const long configFrame::ID_STATICTEXT14 = wxNewId();
const long configFrame::ID_STATICTEXT21 = wxNewId();
const long configFrame::ID_STATICTEXT16 = wxNewId();
const long configFrame::ID_STATICTEXT15 = wxNewId();
const long configFrame::ID_STATICTEXT17 = wxNewId();
const long configFrame::ID_STATICTEXT18 = wxNewId();
const long configFrame::ID_STATICTEXT22 = wxNewId();
const long configFrame::ID_STATICTEXT33 = wxNewId();
const long configFrame::ID_STATICTEXT44 = wxNewId();
const long configFrame::ID_STATICTEXT41 = wxNewId();
const long configFrame::ID_STATICTEXT32 = wxNewId();
const long configFrame::ID_STATICTEXT42 = wxNewId();
const long configFrame::ID_CHOICE7 = wxNewId();
const long configFrame::ID_STATICTEXT43 = wxNewId();
const long configFrame::ID_BUTTON9 = wxNewId();
const long configFrame::ID_CHOICE8 = wxNewId();
const long configFrame::ID_TEXTCTRL8 = wxNewId();
const long configFrame::ID_TEXTCTRL9 = wxNewId();
const long configFrame::ID_TEXTCTRL10 = wxNewId();
const long configFrame::ID_CHOICE1 = wxNewId();
const long configFrame::ID_TEXTCTRL1 = wxNewId();
const long configFrame::ID_TEXTCTRL2 = wxNewId();
const long configFrame::ID_COMBOBOX2 = wxNewId();
const long configFrame::ID_GRID2 = wxNewId();
const long configFrame::ID_BUTTON3 = wxNewId();
const long configFrame::ID_BUTTON7 = wxNewId();
const long configFrame::ID_BUTTON5 = wxNewId();
const long configFrame::ID_PANEL3 = wxNewId();
const long configFrame::ID_NOTEBOOK1 = wxNewId();
const long configFrame::idMenuNew = wxNewId();
const long configFrame::idMenuOpen = wxNewId();
const long configFrame::idMenuSave = wxNewId();
const long configFrame::idMenuSaveAs = wxNewId();
const long configFrame::idMenuQuit = wxNewId();
const long configFrame::ID_MENUITEM12 = wxNewId();
const long configFrame::ID_MENUITEM18 = wxNewId();
const long configFrame::ID_MENUITEM17 = wxNewId();
const long configFrame::ID_MENUITEM19 = wxNewId();
const long configFrame::ID_MENUITEM23 = wxNewId();
const long configFrame::ID_MENUITEM20 = wxNewId();
const long configFrame::ID_MENUITEM22 = wxNewId();
const long configFrame::ID_MENUITEM21 = wxNewId();
const long configFrame::ID_MENUITEM1 = wxNewId();
const long configFrame::ID_MENUITEM2 = wxNewId();
const long configFrame::ID_MENUITEM3 = wxNewId();
const long configFrame::ID_MENUITEM4 = wxNewId();
const long configFrame::ID_MENUITEM5 = wxNewId();
const long configFrame::ID_MENUITEM6 = wxNewId();
const long configFrame::ID_MENUITEM7 = wxNewId();
const long configFrame::ID_MENUITEM8 = wxNewId();
const long configFrame::ID_MENUITEM9 = wxNewId();
const long configFrame::ID_MENUITEM10 = wxNewId();
const long configFrame::ID_MENUITEM11 = wxNewId();
const long configFrame::ID_MENUITEM13 = wxNewId();
const long configFrame::ID_MENUITEM14 = wxNewId();
const long configFrame::ID_MENUITEM15 = wxNewId();
const long configFrame::ID_MENUITEM16 = wxNewId();
const long configFrame::ID_MENUITEM24 = wxNewId();
const long configFrame::ID_MENUITEM25 = wxNewId();
const long configFrame::ID_MENUITEM27 = wxNewId();
const long configFrame::ID_MENUITEM26 = wxNewId();
const long configFrame::idMenuAbout = wxNewId();
const long configFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(configFrame,wxFrame)
    //(*EventTable(configFrame)
    //*)
END_EVENT_TABLE()


void configFrame::fillButtonAxisChoice(wxChoice* choice)
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

void configFrame::fillAxisAxisChoice(wxChoice* choice)
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

void configFrame::fillButtonChoice(wxChoice* choice)
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

void configFrame::readLabels()
{
  wxDir dir(default_directory);
  list<string> button_labels;
  list<string> axis_labels;

  if(!dir.IsOpened())
  {
    cout << "Warning: can't open " << string(default_directory.mb_str()) << endl;
    return;
  }

  wxString file;
  wxString filepath;
  wxString filespec = wxT("*.xml");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    filepath = default_directory + file;
    ConfigurationFile::GetLabels(string(filepath.mb_str()), button_labels, axis_labels);
  }

  button_labels.sort(compare_nocase);
  axis_labels.sort(compare_nocase);

  for(list<string>::iterator it = button_labels.begin(); it != button_labels.end(); ++it)
  {
    ButtonTabLabel->Append(wxString(it->c_str(), wxConvUTF8));
  }

  for(list<string>::iterator it = axis_labels.begin(); it != axis_labels.end(); ++it)
  {
    AxisTabLabel->Append(wxString(it->c_str(), wxConvUTF8));
  }
}

configFrame::configFrame(wxString file,wxWindow* parent,wxWindowID id)
{
    //(*Initialize(configFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxMenu* MenuHelp;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItemAbout;
    wxFlexGridSizer* FlexGridSizer16;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer7;
    wxMenuItem* MenuItemOpen;
    wxStaticBoxSizer* StaticBoxSizer8;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxGridSizer* GridSizer1;
    wxStaticBoxSizer* StaticBoxSizer6;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer12;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer17;
    wxStaticBoxSizer* StaticBoxSizer5;

    Create(parent, wxID_ANY, _("Gimx-config"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    GridSizer1 = new wxGridSizer(1, 1, 0, 0);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxSize(-1,570), 0, _T("ID_NOTEBOOK1"));
    PanelOverall = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer10 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Profile trigger"));
    FlexGridSizer13 = new wxFlexGridSizer(1, 9, 0, 0);
    ProfileTriggerDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT35, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer13->Add(ProfileTriggerDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT27, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer13->Add(ProfileTriggerDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT36, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer13->Add(ProfileTriggerDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT37, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer13->Add(ProfileTriggerButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonAutoDetect = new wxButton(PanelOverall, ID_BUTTON1, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer13->Add(ButtonAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    CheckBoxSwitchBack = new wxCheckBox(PanelOverall, ID_CHECKBOX1, _("Switch back"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxSwitchBack->SetValue(false);
    FlexGridSizer13->Add(CheckBoxSwitchBack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDelayPanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT28, _("Delay (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer13->Add(StaticTextDelayPanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDelay = new wxSpinCtrl(PanelOverall, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(65,-1), 0, 0, 1000, 0, _T("ID_SPINCTRL5"));
    ProfileTriggerDelay->SetValue(_T("0"));
    FlexGridSizer13->Add(ProfileTriggerDelay, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonDelete = new wxButton(PanelOverall, ID_BUTTON10, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    FlexGridSizer13->Add(ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer13, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Stick intensity"));
    FlexGridSizer18 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Left"));
    FlexGridSizer12 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Increase"));
    FlexGridSizer14 = new wxFlexGridSizer(2, 6, 0, 0);
    LSIncDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT58, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer14->Add(LSIncDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT59, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer14->Add(LSIncDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT60, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer14->Add(LSIncDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT61, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer14->Add(LSIncButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSIncAutoDetect = new wxButton(PanelOverall, ID_BUTTON13, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    FlexGridSizer14->Add(LSIncAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    LSIncDelete = new wxButton(PanelOverall, ID_BUTTON14, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON14"));
    FlexGridSizer14->Add(LSIncDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeadZonePanelAxis = new wxStaticText(PanelOverall, ID_STATICTEXT24, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer12->Add(StaticTextDeadZonePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDeadzone = new wxSpinCtrl(PanelOverall, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL3"));
    LSDeadzone->SetValue(_T("0"));
    FlexGridSizer12->Add(LSDeadzone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextStepsPanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT62, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer12->Add(StaticTextStepsPanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSSteps = new wxSpinCtrl(PanelOverall, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL1"));
    LSSteps->SetValue(_T("1"));
    FlexGridSizer12->Add(LSSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Decrease"));
    FlexGridSizer15 = new wxFlexGridSizer(1, 6, 0, 0);
    LSDecDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT67, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT67"));
    FlexGridSizer15->Add(LSDecDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT68, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT68"));
    FlexGridSizer15->Add(LSDecDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT69, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT69"));
    FlexGridSizer15->Add(LSDecDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT70, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT70"));
    FlexGridSizer15->Add(LSDecButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSDecAutoDetect = new wxButton(PanelOverall, ID_BUTTON15, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON15"));
    FlexGridSizer15->Add(LSDecAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    LSDecDelete = new wxButton(PanelOverall, ID_BUTTON16, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON16"));
    FlexGridSizer15->Add(LSDecDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShapePanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT19, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer12->Add(StaticTextShapePanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    LSShape = new wxChoice(PanelOverall, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    LSShape->SetSelection( LSShape->Append(_("Circle")) );
    LSShape->Append(_("Rectangle"));
    FlexGridSizer12->Add(LSShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18->Add(StaticBoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Right"));
    FlexGridSizer16 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticBoxSizer7 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Increase"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 6, 0, 0);
    RSIncDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT48, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer2->Add(RSIncDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT49, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer2->Add(RSIncDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT50, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    FlexGridSizer2->Add(RSIncDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT51, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer2->Add(RSIncButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSIncAutoDetect = new wxButton(PanelOverall, ID_BUTTON11, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer2->Add(RSIncAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    RSIncDelete = new wxButton(PanelOverall, ID_BUTTON12, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer2->Add(RSIncDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeadZonePanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT26, _("Dead zone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer16->Add(StaticTextDeadZonePanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDeadZone = new wxSpinCtrl(PanelOverall, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 127, 0, _T("ID_SPINCTRL4"));
    RSDeadZone->SetValue(_T("0"));
    FlexGridSizer16->Add(RSDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextSteps2PanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT25, _("Steps:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer16->Add(StaticTextSteps2PanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSSteps = new wxSpinCtrl(PanelOverall, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxSize(60,-1), 0, 1, 127, 1, _T("ID_SPINCTRL2"));
    RSSteps->SetValue(_T("1"));
    FlexGridSizer16->Add(RSSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Decrease"));
    FlexGridSizer17 = new wxFlexGridSizer(1, 6, 0, 0);
    RSDecDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer17->Add(RSDecDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer17->Add(RSDecDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer17->Add(RSDecDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer17->Add(RSDecButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSDecAutoDetect = new wxButton(PanelOverall, ID_BUTTON17, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON17"));
    FlexGridSizer17->Add(RSDecAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    RSDecDelete = new wxButton(PanelOverall, ID_BUTTON18, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON18"));
    FlexGridSizer17->Add(RSDecDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(StaticBoxSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShape2PanelOverall = new wxStaticText(PanelOverall, ID_STATICTEXT23, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer16->Add(StaticTextShape2PanelOverall, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RSShape = new wxChoice(PanelOverall, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    RSShape->SetSelection( RSShape->Append(_("Circle")) );
    RSShape->Append(_("Rectangle"));
    FlexGridSizer16->Add(RSShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18->Add(StaticBoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelOverall->SetSizer(FlexGridSizer10);
    FlexGridSizer10->Fit(PanelOverall);
    FlexGridSizer10->SetSizeHints(PanelOverall);
    PanelButton = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer9->AddGrowableRow(1);
    FlexGridSizer1 = new wxFlexGridSizer(2, 9, 0, 0);
    StaticTextDeviceTypePanelButton = new wxStaticText(PanelButton, ID_STATICTEXT4, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticTextDeviceTypePanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeviceNamePanelButton = new wxStaticText(PanelButton, ID_STATICTEXT29, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    FlexGridSizer1->Add(StaticTextDeviceNamePanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeviceIdPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT10, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer1->Add(StaticTextDeviceIdPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEventTypePanelButton = new wxStaticText(PanelButton, ID_STATICTEXT5, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(StaticTextEventTypePanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEventIdPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT6, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer1->Add(StaticTextEventIdPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextThresholdPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT8, _("Threshold"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer1->Add(StaticTextThresholdPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEmptyPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT20, wxEmptyString, wxDefaultPosition, wxSize(31,21), 0, _T("ID_STATICTEXT20"));
    FlexGridSizer1->Add(StaticTextEmptyPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextButtonPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT7, _("Button"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer1->Add(StaticTextButtonPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextLabel = new wxStaticText(PanelButton, ID_STATICTEXT34, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer1->Add(StaticTextLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceType = new wxStaticText(PanelButton, ID_STATICTEXT38, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer1->Add(ButtonTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceName = new wxStaticText(PanelButton, ID_STATICTEXT30, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer1->Add(ButtonTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceId = new wxStaticText(PanelButton, ID_STATICTEXT39, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer1->Add(ButtonTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabEventType = new wxChoice(PanelButton, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    ButtonTabEventType->SetSelection( ButtonTabEventType->Append(_("button")) );
    ButtonTabEventType->Append(_("axis up"));
    ButtonTabEventType->Append(_("axis down"));
    FlexGridSizer1->Add(ButtonTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabEventId = new wxStaticText(PanelButton, ID_STATICTEXT40, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer1->Add(ButtonTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabThreshold = new wxTextCtrl(PanelButton, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    ButtonTabThreshold->Disable();
    FlexGridSizer1->Add(ButtonTabThreshold, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabAutoDetect = new wxButton(PanelButton, ID_BUTTON8, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer1->Add(ButtonTabAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    ButtonTabButtonId = new wxChoice(PanelButton, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    FlexGridSizer1->Add(ButtonTabButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabLabel = new wxComboBox(PanelButton, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
    FlexGridSizer1->Add(ButtonTabLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    GridPanelButton = new wxGrid(PanelButton, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
    GridPanelButton->CreateGrid(0,8);
    GridPanelButton->EnableEditing(false);
    GridPanelButton->EnableGridLines(true);
    GridPanelButton->SetRowLabelSize(25);
    GridPanelButton->SetColLabelValue(0, _("Device type"));
    GridPanelButton->SetColLabelValue(1, _("Device name"));
    GridPanelButton->SetColLabelValue(2, _("Device id"));
    GridPanelButton->SetColLabelValue(3, _("Event type"));
    GridPanelButton->SetColLabelValue(4, _("Event id"));
    GridPanelButton->SetColLabelValue(5, _("Threshold"));
    GridPanelButton->SetColLabelValue(6, _("Button id"));
    GridPanelButton->SetColLabelValue(7, _("Label"));
    GridPanelButton->SetDefaultCellFont( GridPanelButton->GetFont() );
    GridPanelButton->SetDefaultCellTextColour( GridPanelButton->GetForegroundColour() );
    FlexGridSizer8->Add(GridPanelButton, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(2, 1, 0, 0);
    ButtonTabAdd = new wxButton(PanelButton, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer6->Add(ButtonTabAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabRemove = new wxButton(PanelButton, ID_BUTTON6, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer6->Add(ButtonTabRemove, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabModify = new wxButton(PanelButton, ID_BUTTON2, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer6->Add(ButtonTabModify, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer9->Add(FlexGridSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelButton->SetSizer(FlexGridSizer9);
    FlexGridSizer9->Fit(PanelButton);
    FlexGridSizer9->SetSizeHints(PanelButton);
    PanelAxis = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer3 = new wxFlexGridSizer(2, 13, 0, 0);
    StaticTextDeviceTypePanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT11, _("Device type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer3->Add(StaticTextDeviceTypePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeviceNamePanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT31, _("Device name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer3->Add(StaticTextDeviceNamePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDeviceIdPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT12, _("Device id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer3->Add(StaticTextDeviceIdPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEventTypePanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT13, _("Event type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer3->Add(StaticTextEventTypePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEventIdPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT14, _("Event id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer3->Add(StaticTextEventIdPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextEmptyPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT21, wxEmptyString, wxDefaultPosition, wxSize(30,17), 0, _T("ID_STATICTEXT21"));
    FlexGridSizer3->Add(StaticTextEmptyPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextAxisPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT16, _("Axis"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer3->Add(StaticTextAxisPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDZPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT15, _("DZ"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    StaticTextDZPanelAxis->SetToolTip(_("Dead zone"));
    FlexGridSizer3->Add(StaticTextDZPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextSensPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT17, _("Sens."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    StaticTextSensPanelAxis->SetToolTip(_("Sensitivity"));
    FlexGridSizer3->Add(StaticTextSensPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextAccelPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT18, _("Accel."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    StaticTextAccelPanelAxis->SetToolTip(_("Acceleration"));
    FlexGridSizer3->Add(StaticTextAccelPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShapePanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT22, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer3->Add(StaticTextShapePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextSmoothingPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT33, _("Smoothing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer3->Add(StaticTextSmoothingPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(PanelAxis, ID_STATICTEXT44, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceType = new wxStaticText(PanelAxis, ID_STATICTEXT41, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer3->Add(AxisTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceName = new wxStaticText(PanelAxis, ID_STATICTEXT32, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer3->Add(AxisTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceId = new wxStaticText(PanelAxis, ID_STATICTEXT42, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer3->Add(AxisTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabEventType = new wxChoice(PanelAxis, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    AxisTabEventType->SetSelection( AxisTabEventType->Append(_("axis")) );
    AxisTabEventType->Append(_("button"));
    FlexGridSizer3->Add(AxisTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabEventId = new wxStaticText(PanelAxis, ID_STATICTEXT43, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer3->Add(AxisTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAutoDetect = new wxButton(PanelAxis, ID_BUTTON9, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer3->Add(AxisTabAutoDetect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAxisId = new wxChoice(PanelAxis, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE8"));
    FlexGridSizer3->Add(AxisTabAxisId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeadZone = new wxTextCtrl(PanelAxis, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    AxisTabDeadZone->Disable();
    AxisTabDeadZone->SetToolTip(_("Dead zone [0..64]"));
    FlexGridSizer3->Add(AxisTabDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabSensitivity = new wxTextCtrl(PanelAxis, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(59,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    AxisTabSensitivity->Disable();
    AxisTabSensitivity->SetToolTip(_("Sensitivity [-100.00..100.00]"));
    FlexGridSizer3->Add(AxisTabSensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAcceleration = new wxTextCtrl(PanelAxis, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(55,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
    AxisTabAcceleration->Disable();
    AxisTabAcceleration->SetToolTip(_("Acceleration [0.00..2.00]"));
    FlexGridSizer3->Add(AxisTabAcceleration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabShape = new wxChoice(PanelAxis, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    AxisTabShape->SetSelection( AxisTabShape->Append(wxEmptyString) );
    AxisTabShape->Append(_("Circle"));
    AxisTabShape->Append(_("Rectangle"));
    AxisTabShape->Disable();
    FlexGridSizer3->Add(AxisTabShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(1, 2, 0, 0);
    AxisTabBufferSize = new wxTextCtrl(PanelAxis, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    AxisTabBufferSize->Disable();
    AxisTabBufferSize->SetToolTip(_("Buffer size [1..30]"));
    FlexGridSizer11->Add(AxisTabBufferSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabFilter = new wxTextCtrl(PanelAxis, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(41,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    AxisTabFilter->Disable();
    AxisTabFilter->SetToolTip(_("Filter [0.00..1.00]"));
    FlexGridSizer11->Add(AxisTabFilter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabLabel = new wxComboBox(PanelAxis, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX2"));
    FlexGridSizer3->Add(AxisTabLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(1, 2, 0, 0);
    GridPanelAxis = new wxGrid(PanelAxis, ID_GRID2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID2"));
    GridPanelAxis->CreateGrid(0,13);
    GridPanelAxis->EnableEditing(false);
    GridPanelAxis->EnableGridLines(true);
    GridPanelAxis->SetRowLabelSize(25);
    GridPanelAxis->SetDefaultColSize(100, true);
    GridPanelAxis->SetColLabelValue(0, _("Device type"));
    GridPanelAxis->SetColLabelValue(1, _("Device name"));
    GridPanelAxis->SetColLabelValue(2, _("Device id"));
    GridPanelAxis->SetColLabelValue(3, _("Event type"));
    GridPanelAxis->SetColLabelValue(4, _("Event id"));
    GridPanelAxis->SetColLabelValue(5, _("Axis id"));
    GridPanelAxis->SetColLabelValue(6, _("Dead zone"));
    GridPanelAxis->SetColLabelValue(7, _("Sensitivity"));
    GridPanelAxis->SetColLabelValue(8, _("Acceleration"));
    GridPanelAxis->SetColLabelValue(9, _("Shape"));
    GridPanelAxis->SetColLabelValue(10, _("Buffer Size"));
    GridPanelAxis->SetColLabelValue(11, _("Filter"));
    GridPanelAxis->SetColLabelValue(12, _("Label"));
    GridPanelAxis->SetDefaultCellFont( GridPanelAxis->GetFont() );
    GridPanelAxis->SetDefaultCellTextColour( GridPanelAxis->GetForegroundColour() );
    FlexGridSizer4->Add(GridPanelAxis, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer7 = new wxFlexGridSizer(2, 1, 0, 0);
    Button3 = new wxButton(PanelAxis, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button7 = new wxButton(PanelAxis, ID_BUTTON7, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer7->Add(Button7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button5 = new wxButton(PanelAxis, ID_BUTTON5, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer7->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelAxis->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(PanelAxis);
    FlexGridSizer5->SetSizeHints(PanelAxis);
    Notebook1->AddPage(PanelOverall, _("Overall"), false);
    Notebook1->AddPage(PanelButton, _("Button"), true);
    Notebook1->AddPage(PanelAxis, _("Axis"), false);
    GridSizer1->Add(Notebook1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(GridSizer1);
    MenuBar1 = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItemNew = new wxMenuItem(MenuFile, idMenuNew, _("New\tCtrl-N"), _("Create a new configuration"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemNew);
    MenuItemOpen = new wxMenuItem(MenuFile, idMenuOpen, _("Open\tCtrl-O"), _("Open a configuration file"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemOpen);
    MenuItemSave = new wxMenuItem(MenuFile, idMenuSave, _("Save\tCtrl-S"), _("Save configuration"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemSave);
    MenuItemSave->Enable(false);
    MenuItemSaveAs = new wxMenuItem(MenuFile, idMenuSaveAs, _("Save As..."), _("Save configuration to specified file"), wxITEM_NORMAL);
    MenuFile->Append(MenuItemSaveAs);
    MenuFile->AppendSeparator();
    MenuItem1 = new wxMenuItem(MenuFile, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    MenuFile->Append(MenuItem1);
    MenuBar1->Append(MenuFile, _("&File"));
    MenuEdit = new wxMenu();
    MenuItemCopyProfile = new wxMenuItem(MenuEdit, ID_MENUITEM12, _("Copy Profile"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemCopyProfile);
    MenuItemCopyController = new wxMenuItem(MenuEdit, ID_MENUITEM18, _("Copy Controller"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemCopyController);
    MenuEdit->AppendSeparator();
    MenuItemPasteProfile = new wxMenuItem(MenuEdit, ID_MENUITEM17, _("Paste Profile"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemPasteProfile);
    MenuItemPasteController = new wxMenuItem(MenuEdit, ID_MENUITEM19, _("Paste Controller"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemPasteController);
    MenuEdit->AppendSeparator();
    MenuItemSetMouseDPI = new wxMenuItem(MenuEdit, ID_MENUITEM23, _("Set Mouse DPI"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemSetMouseDPI);
    MenuEdit->AppendSeparator();
    MenuItemReplaceMouse = new wxMenuItem(MenuEdit, ID_MENUITEM20, _("Replace Mouse"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemReplaceMouse);
    MenuItemReplaceMouse->Enable(false);
    MenuItemReplaceMouseDPI = new wxMenuItem(MenuEdit, ID_MENUITEM22, _("Convert mouse sensitivity"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemReplaceMouseDPI);
    MenuItemReplaceKeyboard = new wxMenuItem(MenuEdit, ID_MENUITEM21, _("Replace Keyboard"), wxEmptyString, wxITEM_NORMAL);
    MenuEdit->Append(MenuItemReplaceKeyboard);
    MenuItemReplaceKeyboard->Enable(false);
    MenuBar1->Append(MenuEdit, _("Edit"));
    MenuController = new wxMenu();
    MenuController1 = new wxMenuItem(MenuController, ID_MENUITEM1, _("1"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController1);
    MenuController2 = new wxMenuItem(MenuController, ID_MENUITEM2, _("2"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController2);
    MenuController3 = new wxMenuItem(MenuController, ID_MENUITEM3, _("3"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController3);
    MenuController4 = new wxMenuItem(MenuController, ID_MENUITEM4, _("4"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController4);
    MenuController5 = new wxMenuItem(MenuController, ID_MENUITEM5, _("5"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController5);
    MenuController6 = new wxMenuItem(MenuController, ID_MENUITEM6, _("6"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController6);
    MenuController7 = new wxMenuItem(MenuController, ID_MENUITEM7, _("7"), wxEmptyString, wxITEM_RADIO);
    MenuController->Append(MenuController7);
    MenuBar1->Append(MenuController, _("Controller"));
    MenuConfiguration = new wxMenu();
    MenuConfiguration1 = new wxMenuItem(MenuConfiguration, ID_MENUITEM8, _("1"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration1);
    MenuConfiguration2 = new wxMenuItem(MenuConfiguration, ID_MENUITEM9, _("2"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration2);
    MenuConfiguration3 = new wxMenuItem(MenuConfiguration, ID_MENUITEM10, _("3"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration3);
    MenuConfiguration4 = new wxMenuItem(MenuConfiguration, ID_MENUITEM11, _("4"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration4);
    MenuConfiguration5 = new wxMenuItem(MenuConfiguration, ID_MENUITEM13, _("5"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration5);
    MenuConfiguration6 = new wxMenuItem(MenuConfiguration, ID_MENUITEM14, _("6"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration6);
    MenuConfiguration7 = new wxMenuItem(MenuConfiguration, ID_MENUITEM15, _("7"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration7);
    MenuConfiguration8 = new wxMenuItem(MenuConfiguration, ID_MENUITEM16, _("8"), wxEmptyString, wxITEM_RADIO);
    MenuConfiguration->Append(MenuConfiguration8);
    MenuBar1->Append(MenuConfiguration, _("Profile"));
    MenuAdvanced = new wxMenu();
    MenuItemMultipleMiceAndKeyboards = new wxMenuItem(MenuAdvanced, ID_MENUITEM24, _("Multiple Mice and Keyboards"), wxEmptyString, wxITEM_CHECK);
    MenuAdvanced->Append(MenuItemMultipleMiceAndKeyboards);
    MenuItemLinkControls = new wxMenuItem(MenuAdvanced, ID_MENUITEM25, _("Link controls"), wxEmptyString, wxITEM_CHECK);
    MenuAdvanced->Append(MenuItemLinkControls);
    MenuItemLinkControls->Check(true);
    MenuAutoBindControls = new wxMenuItem(MenuAdvanced, ID_MENUITEM27, _("Auto-bind controls"), wxEmptyString, wxITEM_NORMAL);
    MenuAdvanced->Append(MenuAutoBindControls);
    MenuBar1->Append(MenuAdvanced, _("Advanced"));
    MenuHelp = new wxMenu();
    MenuUpdate = new wxMenuItem(MenuHelp, ID_MENUITEM26, _("Update"), wxEmptyString, wxITEM_NORMAL);
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
    FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("XML files (*.xml)|*.xml"), wxFD_DEFAULT_STYLE|wxFD_OPEN, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    GridSizer1->Fit(this);
    GridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonAutoDetectClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonDeleteTrigger);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnLSIncAutoDetectClick);
    Connect(ID_BUTTON14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnLSIncDeleteClick);
    Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&configFrame::OnLSDeadzoneChange);
    Connect(ID_BUTTON15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnLSDecAutoDetectClick);
    Connect(ID_BUTTON16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnLSDecDeleteClick);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnRSIncAutoDetectClick1);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnRSIncDeleteClick);
    Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&configFrame::OnRSDeadZoneChange);
    Connect(ID_BUTTON17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnRSDecAutoDetectClick);
    Connect(ID_BUTTON18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnRSDecDeleteClick);
    Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&configFrame::OnButtonTabEventTypeSelect);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonTabAutoDetectClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonAddPanelButton);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonRemovePanelButton);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonModifyButton);
    Connect(ID_CHOICE7,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&configFrame::OnEventTypeSelectPanelAxis);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnAxisTabAutoDetectClick);
    Connect(ID_TEXTCTRL8,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&configFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL9,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&configFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL10,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&configFrame::OnTextCtrl);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&configFrame::OnAxisTabShapeSelect);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&configFrame::OnTextCtrl);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&configFrame::OnTextCtrl);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonAddPanelAxis);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonRemovePanelAxis);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnButtonModifyAxis);
    Connect(idMenuNew,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemNew);
    Connect(idMenuOpen,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuOpen);
    Connect(idMenuSave,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuSave);
    Connect(idMenuSaveAs,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuSaveAs);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnQuit);
    Connect(ID_MENUITEM12,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemCopyConfiguration);
    Connect(ID_MENUITEM18,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemCopyController);
    Connect(ID_MENUITEM17,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemPasteConfiguration);
    Connect(ID_MENUITEM19,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemPasteController);
    Connect(ID_MENUITEM23,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuSetMouseDPI);
    Connect(ID_MENUITEM20,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuReplaceMouse);
    Connect(ID_MENUITEM22,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuReplaceMouseDPI);
    Connect(ID_MENUITEM21,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuReplaceKeyboard);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemController);
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM10,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM11,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM13,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM14,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM15,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM16,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuItemConfiguration);
    Connect(ID_MENUITEM24,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuMultipleMK);
    Connect(ID_MENUITEM27,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuAutoBindControls);
    Connect(ID_MENUITEM26,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnMenuUpdate);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&configFrame::OnAbout);
    //*)

    GridPanelButton->SetSelectionMode(wxGrid::wxGridSelectRows);
    GridPanelAxis->SetSelectionMode(wxGrid::wxGridSelectRows);

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
    default_directory.Append(_(APP_DIR));

	/* Init user's config directory */
    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( wxT("Can't init ~/.emuclient/config!"), wxT("Error"), wxICON_ERROR);
    }
#endif

    default_directory.Append(_(CONFIG_DIR));

    FileDialog1->SetDirectory(default_directory);

#ifdef WIN32
    MenuController2->Enable(false);
    MenuController3->Enable(false);
    MenuController4->Enable(false);
    MenuController5->Enable(false);
    MenuController6->Enable(false);
    MenuController7->Enable(false);
    MenuItemCopyController->Enable(false);
    MenuItemPasteController->Enable(false);
#endif

    GridPanelButton->AutoSizeColumns();
    GridPanelAxis->AutoSizeColumns();

	  configFile.SetEvCatch(&evcatch);

	  /* Open the file given as argument */
    if(!file.IsEmpty())
    {
      wxString wxfile = default_directory + file;

      if(::wxFileExists(wxfile))
      {
        int ret = configFile.ReadConfigFile(string(wxfile.mb_str()));

        if(ret < 0)
        {
          wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), wxT("Error"), wxICON_ERROR);
        }
        else if(ret > 0)
        {
          wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), wxT("Info"), wxICON_INFORMATION);
        }

        MenuItemMultipleMiceAndKeyboards->Check(configFile.MultipleMK());
        if(MenuItemMultipleMiceAndKeyboards->IsChecked())
        {
            MenuItemReplaceMouse->Enable(true);
            MenuItemReplaceKeyboard->Enable(true);
        }
        else
        {
            MenuItemReplaceMouse->Enable(false);
            MenuItemReplaceKeyboard->Enable(false);
        }
        load_current();
        refresh_gui();
        MenuFile->Enable(idMenuSave, true);
        FileDialog1->SetFilename(file);
      }
      else
      {
        wxMessageBox( wxT("Cannot open config file: ") + file, wxT("Error"), wxICON_ERROR);
      }
    }

    readLabels();
}

configFrame::~configFrame()
{
    //(*Destroy(configFrame)
    //*)
}

void configFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void configFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(_(INFO_DESCR)) + wxString(_("\n")) + wxString(_(INFO_YEAR)) + wxString(_(" ")) + wxString(_(INFO_DEV)) + wxString(_(" ")) + wxString(_(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void configFrame::OnMenuItemNew(wxCommandEvent& event)
{
    FileDialog1->SetFilename(wxEmptyString);
    configFile = ConfigurationFile();

    currentController = 0;
    currentConfiguration = 0;
    MenuController->Check(ID_MENUITEM1, true);
    MenuConfiguration->Check(ID_MENUITEM8, true);
    load_current();
    refresh_gui();
    MenuFile->Enable(idMenuSave, false);
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void configFrame::OnButtonAddPanelButton(wxCommandEvent& event)
{
    if(ButtonTabEventId->GetLabel().IsEmpty())
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(ButtonTabButtonId->GetStringSelection().IsEmpty())
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

    GridPanelButton->InsertRows();
    GridPanelButton->SetCellValue(0, 0, ButtonTabDeviceType->GetLabel());
    GridPanelButton->SetCellValue(0, 1, ButtonTabDeviceName->GetLabel());
    GridPanelButton->SetCellValue(0, 2, ButtonTabDeviceId->GetLabel());
    GridPanelButton->SetCellValue(0, 3, ButtonTabEventType->GetStringSelection());
    GridPanelButton->SetCellValue(0, 4, ButtonTabEventId->GetLabel());
    GridPanelButton->SetCellValue(0, 5, ButtonTabThreshold->GetValue());
    GridPanelButton->SetCellValue(0, 6, ButtonTabButtonId->GetStringSelection());
    GridPanelButton->SetCellValue(0, 7, ButtonTabLabel->GetValue());
    GridPanelButton->AutoSizeColumns();
    refresh_gui();
}

void configFrame::OnButtonAddPanelAxis(wxCommandEvent& event)
{
    if(AxisTabEventId->GetLabel().IsEmpty())
    {
        wxMessageBox( wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
        return;
    }
    if(AxisTabAxisId->GetStringSelection().IsEmpty())
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

    GridPanelAxis->InsertRows();
    GridPanelAxis->SetCellValue(0, 0, AxisTabDeviceType->GetLabel());
    GridPanelAxis->SetCellValue(0, 1, AxisTabDeviceName->GetLabel());
    GridPanelAxis->SetCellValue(0, 2, AxisTabDeviceId->GetLabel());
    GridPanelAxis->SetCellValue(0, 3, AxisTabEventType->GetStringSelection());
    GridPanelAxis->SetCellValue(0, 4, AxisTabEventId->GetLabel());
    GridPanelAxis->SetCellValue(0, 5, AxisTabAxisId->GetStringSelection());
    GridPanelAxis->SetCellValue(0, 6, AxisTabDeadZone->GetValue());
    GridPanelAxis->SetCellValue(0, 7, AxisTabSensitivity->GetValue());
    GridPanelAxis->SetCellValue(0, 8, AxisTabAcceleration->GetValue());
    GridPanelAxis->SetCellValue(0, 9, AxisTabShape->GetStringSelection());
    GridPanelAxis->SetCellValue(0, 10, AxisTabBufferSize->GetValue());
    GridPanelAxis->SetCellValue(0, 11, AxisTabFilter->GetValue());
    GridPanelAxis->SetCellValue(0, 12, AxisTabLabel->GetValue());
    GridPanelAxis->AutoSizeColumns();
    refresh_gui();
}

void configFrame::DeleteLinkedRows(wxGrid* grid, int row)
{
  if(grid == GridPanelButton)
  {
    string old_device_type = string(GridPanelButton->GetCellValue(row, 0).mb_str());
    string old_device_name = string(GridPanelButton->GetCellValue(row, 1).mb_str());
    string old_device_id = string(GridPanelButton->GetCellValue(row, 2).mb_str());
    string old_event_type = string(GridPanelButton->GetCellValue(row, 3).mb_str());
    string old_event_id = string(GridPanelButton->GetCellValue(row, 4).mb_str());
    string old_button_id = string(GridPanelButton->GetCellValue(row, 6).mb_str());

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
          ++it;
        }
      }
    }
  }
  else if(grid == GridPanelAxis)
  {
    string old_device_type = string(GridPanelAxis->GetCellValue(row, 0).mb_str());
    string old_device_name = string(GridPanelAxis->GetCellValue(row, 1).mb_str());
    string old_device_id = string(GridPanelAxis->GetCellValue(row, 2).mb_str());
    string old_event_type = string(GridPanelAxis->GetCellValue(row, 3).mb_str());
    string old_event_id = string(GridPanelAxis->GetCellValue(row, 4).mb_str());
    string old_axis_id = string(GridPanelAxis->GetCellValue(row, 5).mb_str());

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
          ++it;
        }
      }
    }
  }
}

static wxArrayInt GetGridSelectedRows(wxGrid* grid)
{
  wxArrayInt array = grid->GetSelectedRows();
  if(array.IsEmpty())
  {
    wxGridCellCoordsArray topLeft = grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray bottomRight = grid->GetSelectionBlockBottomRight();
    if(!topLeft.IsEmpty() && !bottomRight.IsEmpty())
    {
      for(int i=topLeft[0].GetRow(); i<bottomRight[0].GetRow()+1; ++i)
      {
        array.Add(i);
      }
    }
  }
  return array;
}

void configFrame::DeleteSelectedRows(wxGrid* grid)
{
    unsigned int first;
    unsigned int nbRows;
    wxArrayInt array = GetGridSelectedRows(grid);

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
        if(MenuItemLinkControls->IsChecked())
        {
          DeleteLinkedRows(grid, array.Item(0));
        }
        grid->DeleteRows(array.Item(0), 1);
        array = GetGridSelectedRows(grid);
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

void configFrame::OnButtonRemovePanelButton(wxCommandEvent& event)
{
    configFrame::DeleteSelectedRows(GridPanelButton);
    refresh_gui();
}

void configFrame::OnButtonRemovePanelAxis(wxCommandEvent& event)
{
    configFrame::DeleteSelectedRows(GridPanelAxis);
    refresh_gui();
}

void configFrame::OnEventTypeSelectPanelAxis(wxCommandEvent& event)
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

void configFrame::OnButtonTabEventTypeSelect(wxCommandEvent& event)
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
        if(ButtonTabEventType->GetStringSelection() == _("axis down"))
        {
          ButtonTabThreshold->SetValue(_("-10"));
        }
        else if(ButtonTabEventType->GetStringSelection() == _("axis up"))
        {
          ButtonTabThreshold->SetValue(_("10"));
        }
    }
    refresh_gui();
}

void configFrame::auto_detect(wxStaticText* device_type, wxStaticText* device_name, wxStaticText* device_id, wxString event_type, wxStaticText* event_id)
{
    wxString msg;
    if(event_type == _("button"))
    {
      msg = _("Press a button.");
    }
    else
    {
      msg = _("Move an axis.");
    }
    StatusBar1->SetStatusText(msg);
    evcatch.run("", string(event_type.mb_str()));
    StatusBar1->SetStatusText(wxEmptyString);

    device_type->SetLabel(wxString(evcatch.GetDeviceType().c_str(), wxConvUTF8));

    if(MenuItemMultipleMiceAndKeyboards->IsChecked() || evcatch.GetDeviceType() == "joystick")
    {
      device_name->SetLabel(wxString(evcatch.GetDeviceName().c_str(), wxConvUTF8));
      device_id->SetLabel( wxString(evcatch.GetDeviceId().c_str(), wxConvUTF8));
    }
    else
    {
      device_name->SetLabel(wxEmptyString);
      device_id->SetLabel(_("0"));
    }

    event_id->SetLabel( wxString(evcatch.GetEventId().c_str(), wxConvUTF8));
}

void configFrame::OnButtonAutoDetectClick(wxCommandEvent& event)
{
    ButtonAutoDetect->Enable(false);

    string old_device_type = string(ProfileTriggerDeviceType->GetLabel().mb_str());
    string old_device_name = string(ProfileTriggerDeviceName->GetLabel().mb_str());
    string old_device_id = string(ProfileTriggerDeviceId->GetLabel().mb_str());
    string old_event_id = string(ProfileTriggerButtonId->GetLabel().mb_str());
    Device* dev;
    Event* ev;

    auto_detect(ProfileTriggerDeviceType, ProfileTriggerDeviceName, ProfileTriggerDeviceId, _("button"), ProfileTriggerButtonId);

    if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
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
          dev->SetType(string(ProfileTriggerDeviceType->GetLabel().mb_str()));
          dev->SetName(string(ProfileTriggerDeviceName->GetLabel().mb_str()));
          dev->SetId(string(ProfileTriggerDeviceId->GetLabel().mb_str()));
          ev->SetId(string(ProfileTriggerButtonId->GetLabel().mb_str()));
        }
      }
    }

    ButtonAutoDetect->Enable(true);

    refresh_gui();
}

void configFrame::OnButtonTabAutoDetectClick(wxCommandEvent& event)
{
    ButtonTabAutoDetect->Enable(false);

    auto_detect(ButtonTabDeviceType, ButtonTabDeviceName, ButtonTabDeviceId, ButtonTabEventType->GetStringSelection(), ButtonTabEventId);

    if(evcatch.GetEventType() == "button")
    {
        ButtonTabThreshold->Disable();
        ButtonTabThreshold->SetValue(wxEmptyString);
    }
    else
    {
        ButtonTabThreshold->Enable();
        if(ButtonTabEventType->GetStringSelection() == _("axis down"))
        {
          ButtonTabThreshold->SetValue(_("-10"));
        }
        else if(ButtonTabEventType->GetStringSelection() == _("axis up"))
        {
          ButtonTabThreshold->SetValue(_("10"));
        }
    }

    fillButtonChoice(ButtonTabButtonId);

    refresh_gui();

    ButtonTabAutoDetect->Enable(true);
}

void configFrame::OnAxisTabAutoDetectClick(wxCommandEvent& event)
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
        if(evcatch.GetEventType() == "button")
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
          if(evcatch.GetDeviceType() == "mouse")
          {
              AxisTabDeadZone->SetValue(_("20"));
              AxisTabSensitivity->SetValue(_("1.00"));
              AxisTabBufferSize->Enable();
              AxisTabBufferSize->SetValue(_("1"));
              AxisTabFilter->Enable();
              AxisTabFilter->SetValue(_("0.00"));
          }
          else if(evcatch.GetDeviceType() == "joystick")
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

void configFrame::save_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Save Trigger
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetType(string(ProfileTriggerDeviceType->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetName(string(ProfileTriggerDeviceName->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetId(string(ProfileTriggerDeviceId->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->SetId(string(ProfileTriggerButtonId->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetSwitchBack(string(CheckBoxSwitchBack->GetValue()?"yes":"no"));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetDelay(ProfileTriggerDelay->GetValue());
    //Save Intensity
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    intensityList->erase(intensityList->begin(), intensityList->end());
    //Save left stick Intensity
    if(!LSIncDeviceType->GetLabel().IsEmpty())
    {
      intensityList->push_back(Intensity("left_stick",
          string(LSIncDeviceType->GetLabel().mb_str()), string(LSIncDeviceId->GetLabel().mb_str()), string(LSIncDeviceName->GetLabel().mb_str()), string(LSIncButtonId->GetLabel().mb_str()),
          string(LSDecDeviceType->GetLabel().mb_str()), string(LSDecDeviceId->GetLabel().mb_str()), string(LSDecDeviceName->GetLabel().mb_str()), string(LSDecButtonId->GetLabel().mb_str()),
          LSDeadzone->GetValue(), LSSteps->GetValue(), string(LSShape->GetStringSelection().mb_str())));
    }
    //Save right stick intensity
    if(!RSIncDeviceType->GetLabel().IsEmpty())
    {
      intensityList->push_back(Intensity("right_stick",
          string(RSIncDeviceType->GetLabel().mb_str()), string(RSIncDeviceId->GetLabel().mb_str()), string(RSIncDeviceName->GetLabel().mb_str()), string(RSIncButtonId->GetLabel().mb_str()),
          string(RSDecDeviceType->GetLabel().mb_str()), string(RSDecDeviceId->GetLabel().mb_str()), string(RSDecDeviceName->GetLabel().mb_str()), string(RSDecButtonId->GetLabel().mb_str()),
          RSDeadZone->GetValue(), RSSteps->GetValue(), string(RSShape->GetStringSelection().mb_str())));
    }
    //Save ButtonMappers
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    buttonMappers->erase(buttonMappers->begin(), buttonMappers->end());
    for(int i=0; i<GridPanelButton->GetNumberRows(); i++)
    {
        buttonMappers->push_front(ButtonMapper(string(GridPanelButton->GetCellValue(i, 0).mb_str()), string(GridPanelButton->GetCellValue(i, 2).mb_str()), string(GridPanelButton->GetCellValue(i, 1).mb_str()),
            string(GridPanelButton->GetCellValue(i, 3).mb_str()), string(GridPanelButton->GetCellValue(i, 4).mb_str()), string(GridPanelButton->GetCellValue(i, 5).mb_str()), string(GridPanelButton->GetCellValue(i, 6).mb_str()),
            string(GridPanelButton->GetCellValue(i, 7).mb_str())));
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    axisMappers->erase(axisMappers->begin(), axisMappers->end());
    for(int i=0; i<GridPanelAxis->GetNumberRows(); i++)
    {
        axisMappers->push_front(AxisMapper(string(GridPanelAxis->GetCellValue(i, 0).mb_str()), string(GridPanelAxis->GetCellValue(i, 2).mb_str()), string(GridPanelAxis->GetCellValue(i, 1).mb_str()),
            string(GridPanelAxis->GetCellValue(i, 3).mb_str()), string(GridPanelAxis->GetCellValue(i, 4).mb_str()), string(GridPanelAxis->GetCellValue(i, 5).mb_str()), string(GridPanelAxis->GetCellValue(i, 6).mb_str()),
            string(GridPanelAxis->GetCellValue(i, 7).mb_str()), string(GridPanelAxis->GetCellValue(i, 8).mb_str()), string(GridPanelAxis->GetCellValue(i, 9).mb_str()), string(GridPanelAxis->GetCellValue(i, 10).mb_str()),
            string(GridPanelAxis->GetCellValue(i, 11).mb_str()), string(GridPanelAxis->GetCellValue(i, 12).mb_str())));
    }

}

void configFrame::load_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Load Trigger
    ProfileTriggerDeviceType->SetLabel(wxString(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetType().c_str(),wxConvUTF8));
    ProfileTriggerDeviceName->SetLabel(wxString(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetName().c_str(),wxConvUTF8));
    ProfileTriggerDeviceId->SetLabel(wxString(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetId().c_str(),wxConvUTF8));
    ProfileTriggerButtonId->SetLabel(wxString(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->GetId().c_str(),wxConvUTF8));
    if(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetSwitchBack() == "yes")
    {
        CheckBoxSwitchBack->SetValue(true);
    }
    else
    {
        CheckBoxSwitchBack->SetValue(false);
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
    for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
    {
      if(it->GetControl() == "left_stick")
      {
        LSIncDeviceType->SetLabel(wxString(it->GetDeviceUp()->GetType().c_str(),wxConvUTF8));
        LSIncDeviceName->SetLabel(wxString(it->GetDeviceUp()->GetName().c_str(),wxConvUTF8));
        LSIncDeviceId->SetLabel(wxString(it->GetDeviceUp()->GetId().c_str(),wxConvUTF8));
        LSIncButtonId->SetLabel(wxString(it->GetEventUp()->GetId().c_str(),wxConvUTF8));
        LSDecDeviceType->SetLabel(wxString(it->GetDeviceDown()->GetType().c_str(),wxConvUTF8));
        LSDecDeviceName->SetLabel(wxString(it->GetDeviceDown()->GetName().c_str(),wxConvUTF8));
        LSDecDeviceId->SetLabel(wxString(it->GetDeviceDown()->GetId().c_str(),wxConvUTF8));
        LSDecButtonId->SetLabel(wxString(it->GetEventDown()->GetId().c_str(),wxConvUTF8));
        LSDeadzone->SetValue(it->GetDeadZone());
        LSSteps->SetValue(it->GetSteps());
        LSShape->SetSelection(LSShape->FindString(wxString(it->GetShape().c_str(),wxConvUTF8)));
      }
      else if(it->GetControl() == "right_stick")
      {
        RSIncDeviceType->SetLabel(wxString(it->GetDeviceUp()->GetType().c_str(),wxConvUTF8));
        RSIncDeviceName->SetLabel(wxString(it->GetDeviceUp()->GetName().c_str(),wxConvUTF8));
        RSIncDeviceId->SetLabel(wxString(wxString(it->GetDeviceUp()->GetId().c_str(),wxConvUTF8)));
        RSIncButtonId->SetLabel(wxString(it->GetEventUp()->GetId().c_str(),wxConvUTF8));
        RSDecDeviceType->SetLabel(wxString(it->GetDeviceDown()->GetType().c_str(),wxConvUTF8));
        RSDecDeviceName->SetLabel(wxString(it->GetDeviceDown()->GetName().c_str(),wxConvUTF8));
        RSDecDeviceId->SetLabel(wxString(it->GetDeviceDown()->GetId().c_str(),wxConvUTF8));
        RSDecButtonId->SetLabel(wxString(it->GetEventDown()->GetId().c_str(),wxConvUTF8));
        RSDeadZone->SetValue(it->GetDeadZone());
        RSSteps->SetValue(it->GetSteps());
        RSShape->SetSelection(RSShape->FindString(wxString(it->GetShape().c_str(),wxConvUTF8)));
      }
    }
    //Load ButtonMappers
    GridPanelButton->DeleteRows(0, GridPanelButton->GetNumberRows());
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); ++it)
    {
        GridPanelButton->InsertRows();
        GridPanelButton->SetCellValue(0, 0, wxString(it->GetDevice()->GetType().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 1, wxString(it->GetDevice()->GetName().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 2, wxString(it->GetDevice()->GetId().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 3, wxString(it->GetEvent()->GetType().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 4, wxString(it->GetEvent()->GetId().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 5, wxString(it->GetEvent()->GetThreshold().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 6, wxString(it->GetButton().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 7, wxString(it->GetLabel().c_str(),wxConvUTF8));
        if(it->GetLabel().find(", not found") != string::npos || it->GetLabel().find(", duplicate") != string::npos )
        {
          for(int i=0; i<GridPanelButton->GetNumberCols(); ++i)
          {
            GridPanelButton->SetCellBackgroundColour(0, i, wxColour(255, 0, 0));
          }
        }
        else
        {
          for(int i=0; i<GridPanelButton->GetNumberCols(); ++i)
          {
            GridPanelButton->SetCellBackgroundColour(0, i, GridPanelButton->GetDefaultCellBackgroundColour());
          }
        }
    }
    GridPanelButton->AutoSizeColumns();
    //Load AxisMappers
    GridPanelAxis->DeleteRows(0, GridPanelAxis->GetNumberRows());
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); ++it)
    {
        GridPanelAxis->InsertRows();
        GridPanelAxis->SetCellValue(0, 0, wxString(it->GetDevice()->GetType().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 1, wxString(it->GetDevice()->GetName().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 2, wxString(it->GetDevice()->GetId().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 3, wxString(it->GetEvent()->GetType().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 4, wxString(it->GetEvent()->GetId().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 5, wxString(it->GetAxis().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 6, wxString(it->GetEvent()->GetDeadZone().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 7, wxString(it->GetEvent()->GetMultiplier().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 8, wxString(it->GetEvent()->GetExponent().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 9, wxString(it->GetEvent()->GetShape().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 10, wxString(it->GetEvent()->GetBufferSize().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 11, wxString(it->GetEvent()->GetFilter().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 12, wxString(it->GetLabel().c_str(),wxConvUTF8));
        if(it->GetLabel().find(", not found") != string::npos || it->GetLabel().find(", duplicate") != string::npos )
        {
          for(int i=0; i<GridPanelAxis->GetNumberCols(); ++i)
          {
            GridPanelAxis->SetCellBackgroundColour(0, i, wxColour(255, 0, 0));
          }
        }
        else
        {
          for(int i=0; i<GridPanelAxis->GetNumberCols(); ++i)
          {
            GridPanelAxis->SetCellBackgroundColour(0, i, GridPanelAxis->GetDefaultCellBackgroundColour());
          }
        }
    }
    GridPanelAxis->AutoSizeColumns();
}

void configFrame::refresh_gui()
{
    PanelOverall->Layout();
    PanelButton->Layout();
    PanelAxis->Layout();
    Refresh();
}

void configFrame::OnMenuOpen(wxCommandEvent& event)
{
    int ret;
    FileDialog1->SetDirectory(default_directory);

    if ( FileDialog1->ShowModal() != wxID_OK ) return;

    wxString FileName = FileDialog1->GetPath();
    if ( FileName.IsEmpty() ) return;

    ret = configFile.ReadConfigFile(string(FileName.mb_str()));

    if(ret < 0)
    {
      wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), wxT("Error"), wxICON_ERROR);
    }
    else if(ret > 0)
    {
      wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), wxT("Info"), wxICON_INFORMATION);
    }

    MenuItemMultipleMiceAndKeyboards->Check(configFile.MultipleMK());
    if(MenuItemMultipleMiceAndKeyboards->IsChecked())
    {
        MenuItemReplaceMouse->Enable(true);
        MenuItemReplaceKeyboard->Enable(true);
    }
    else
    {
        MenuItemReplaceMouse->Enable(false);
        MenuItemReplaceKeyboard->Enable(false);
    }

    currentController = 0;
    currentConfiguration = 0;
    MenuController->Check(ID_MENUITEM1, true);
    MenuConfiguration->Check(ID_MENUITEM8, true);
    load_current();
    refresh_gui();
    MenuFile->Enable(idMenuSave, true);
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void configFrame::OnMenuItemController(wxCommandEvent& event)
{
    save_current();
    if(MenuController1->IsChecked())
    {
      currentController = 0;
    }
    else if(MenuController2->IsChecked())
    {
      currentController = 1;
    }
    else if(MenuController3->IsChecked())
    {
      currentController = 2;
    }
    else if(MenuController4->IsChecked())
    {
      currentController = 3;
    }
    else if(MenuController5->IsChecked())
    {
      currentController = 4;
    }
    else if(MenuController6->IsChecked())
    {
      currentController = 5;
    }
    else if(MenuController7->IsChecked())
    {
      currentController = 6;
    }
    currentConfiguration = 0;
    MenuConfiguration->Check(ID_MENUITEM8, true);
    load_current();
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    refresh_gui();
}


void configFrame::OnMenuItemConfiguration(wxCommandEvent& event)
{
  save_current();
  if(MenuConfiguration1->IsChecked())
  {
    currentConfiguration = 0;
  }
  else if(MenuConfiguration2->IsChecked())
  {
    currentConfiguration = 1;
  }
  else if(MenuConfiguration3->IsChecked())
  {
    currentConfiguration = 2;
  }
  else if(MenuConfiguration4->IsChecked())
  {
    currentConfiguration = 3;
  }
  else if(MenuConfiguration5->IsChecked())
  {
    currentConfiguration = 4;
  }
  else if(MenuConfiguration6->IsChecked())
  {
    currentConfiguration = 5;
  }
  else if(MenuConfiguration7->IsChecked())
  {
    currentConfiguration = 6;
  }
  else if(MenuConfiguration8->IsChecked())
  {
    currentConfiguration = 7;
  }
  load_current();
  ButtonTabModify->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  Button3->Enable();
  ButtonTabAdd->Enable();
  ButtonTabRemove->Enable();
  Button7->Enable();
  refresh_gui();
}

void configFrame::OnMenuSave(wxCommandEvent& event)
{
    wxString end;
    save_current();
    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(wxT("Can't save ") + wxString(configFile.GetFilePath().c_str(), wxConvUTF8), wxT("Error"), wxICON_ERROR);
    }
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
}

void configFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _T("Save Config file"), _T(""), _T(""), _T("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);
    saveFileDialog.SetFilename(FileDialog1->GetFilename());

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(string(FileName.mb_str()));

    OnMenuSave(event);

    MenuFile->Enable(idMenuSave, true);
}

void configFrame::OnButtonModifyButton(wxCommandEvent& event)
{
    wxArrayInt array = GetGridSelectedRows(GridPanelButton);
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

        ButtonTabDeviceType->SetLabel(GridPanelButton->GetCellValue(grid1mod, 0));
        ButtonTabDeviceName->SetLabel(GridPanelButton->GetCellValue(grid1mod, 1));
        ButtonTabDeviceId->SetLabel(GridPanelButton->GetCellValue(grid1mod, 2));
        ButtonTabEventType->SetSelection(ButtonTabEventType->FindString(GridPanelButton->GetCellValue(grid1mod, 3)));
        ButtonTabEventId->SetLabel(GridPanelButton->GetCellValue(grid1mod, 4));
        ButtonTabThreshold->SetValue(GridPanelButton->GetCellValue(grid1mod, 5));
        if(ButtonTabEventType->GetStringSelection() == _("button"))
        {
            ButtonTabThreshold->Disable();
            ButtonTabThreshold->SetValue(wxEmptyString);
        }
        else
        {
            ButtonTabThreshold->Enable();
        }
        fillButtonChoice(ButtonTabButtonId);
        ButtonTabButtonId->SetSelection(ButtonTabButtonId->FindString(GridPanelButton->GetCellValue(grid1mod, 6)));
        ButtonTabLabel->SetValue(GridPanelButton->GetCellValue(grid1mod, 7));
        ButtonTabAdd->Disable();
        ButtonTabRemove->Disable();
        ButtonTabModify->SetLabel(_("Apply"));
    }
    else
    {
        if (ButtonTabEventId->GetLabel().IsEmpty())
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(ButtonTabButtonId->GetStringSelection().IsEmpty())
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

        GridPanelButton->SetCellValue(grid1mod, 0, ButtonTabDeviceType->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 1, ButtonTabDeviceName->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 2, ButtonTabDeviceId->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 3, ButtonTabEventType->GetStringSelection());
        GridPanelButton->SetCellValue(grid1mod, 4, ButtonTabEventId->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 5, ButtonTabThreshold->GetValue());
        GridPanelButton->SetCellValue(grid1mod, 6, ButtonTabButtonId->GetStringSelection());
        string l = string(ButtonTabLabel->GetValue().mb_str());
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
          ButtonTabLabel->SetValue(wxString(l.c_str(), wxConvUTF8));
          for(int i=0; i<GridPanelButton->GetNumberCols(); ++i)
          {
            GridPanelButton->SetCellBackgroundColour(grid1mod, i, GridPanelButton->GetDefaultCellBackgroundColour());
          }
        }
        GridPanelButton->SetCellValue(grid1mod, 7, ButtonTabLabel->GetValue());
        if(MenuItemLinkControls->IsChecked())
        {
          updateButtonConfigurations();
        }
        ButtonTabAdd->Enable();
        ButtonTabRemove->Enable();
        ButtonTabModify->SetLabel(_("Modify"));
    }
    GridPanelButton->AutoSizeColumns();
    refresh_gui();
}

void configFrame::updateButtonConfigurations()
{
    int k;

    std::list<ButtonMapper>* buttonMappers;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      buttonMappers = config->GetButtonMapperList();
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); ++it)
      {
          if(it->GetDevice()->GetType() == string(GridPanelButton->GetCellValue(grid1mod, 0).mb_str())
              && it->GetDevice()->GetName() == string(GridPanelButton->GetCellValue(grid1mod, 1).mb_str())
              && it->GetDevice()->GetId() == string(GridPanelButton->GetCellValue(grid1mod, 2).mb_str())
              && it->GetEvent()->GetType() == string(GridPanelButton->GetCellValue(grid1mod, 3).mb_str())
              && it->GetEvent()->GetId() == string(GridPanelButton->GetCellValue(grid1mod, 4).mb_str())
              && it->GetButton() == string(GridPanelButton->GetCellValue(grid1mod, 6).mb_str()))
          {
              it->GetDevice()->SetType(string(ButtonTabDeviceType->GetLabel().mb_str()));
              it->GetDevice()->SetId(string(ButtonTabDeviceId->GetLabel().mb_str()));
              it->GetDevice()->SetName(string(ButtonTabDeviceName->GetLabel().mb_str()));
              it->GetEvent()->SetType(string(ButtonTabEventType->GetStringSelection().mb_str()));
              it->GetEvent()->SetId(string(ButtonTabEventId->GetLabel().mb_str()));
              it->GetEvent()->SetThreshold(string(ButtonTabThreshold->GetValue().mb_str()));
              it->SetButton(string(ButtonTabButtonId->GetStringSelection().mb_str()));
              it->SetLabel(string(ButtonTabLabel->GetValue().mb_str()));
          }
      }
    }
}

void configFrame::OnButtonModifyAxis(wxCommandEvent& event)
{
    wxArrayInt array = GetGridSelectedRows(GridPanelAxis);
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

        AxisTabDeviceType->SetLabel(GridPanelAxis->GetCellValue(grid2mod, 0));
        AxisTabDeviceName->SetLabel(GridPanelAxis->GetCellValue(grid2mod, 1));
        AxisTabDeviceId->SetLabel(GridPanelAxis->GetCellValue(grid2mod, 2));
        AxisTabEventType->SetSelection(AxisTabEventType->FindString(GridPanelAxis->GetCellValue(grid2mod, 3)));
        AxisTabEventId->SetLabel(GridPanelAxis->GetCellValue(grid2mod, 4));
        AxisTabDeadZone->SetValue(GridPanelAxis->GetCellValue(grid2mod, 6));
        AxisTabSensitivity->SetValue(GridPanelAxis->GetCellValue(grid2mod, 7));
        AxisTabAcceleration->SetValue(GridPanelAxis->GetCellValue(grid2mod, 8));
        AxisTabBufferSize->SetValue(GridPanelAxis->GetCellValue(grid2mod, 10));
        AxisTabFilter->SetValue(GridPanelAxis->GetCellValue(grid2mod, 11));
        AxisTabLabel->SetValue(GridPanelAxis->GetCellValue(grid2mod, 12));
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
            AxisTabShape->SetSelection(AxisTabShape->FindString(GridPanelAxis->GetCellValue(grid2mod, 9)));
            AxisTabShape->Enable();
			      if(AxisTabDeviceType->GetLabel() == _("mouse"))
			      {
                AxisTabBufferSize->Enable();
                AxisTabFilter->Enable();
      			}
            fillAxisAxisChoice(AxisTabAxisId);
        }
        AxisTabAxisId->SetSelection(AxisTabAxisId->FindString(GridPanelAxis->GetCellValue(grid2mod, 5)));
        Button3->Disable();
        Button7->Disable();
        Button5->SetLabel(_("Apply"));
    }
    else
    {
        if (AxisTabEventId->GetLabel().IsEmpty())
        {
          wxMessageBox(wxT("Please detect an Event!"), wxT("Error"), wxICON_ERROR);
          return;
        }
        if(AxisTabAxisId->GetStringSelection().IsEmpty())
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

        GridPanelAxis->SetCellValue(grid2mod, 0, AxisTabDeviceType->GetLabel());
        GridPanelAxis->SetCellValue(grid2mod, 1, AxisTabDeviceName->GetLabel());
        GridPanelAxis->SetCellValue(grid2mod, 2, AxisTabDeviceId->GetLabel());
        GridPanelAxis->SetCellValue(grid2mod, 3, AxisTabEventType->GetStringSelection());
        GridPanelAxis->SetCellValue(grid2mod, 4, AxisTabEventId->GetLabel());
        GridPanelAxis->SetCellValue(grid2mod, 5, AxisTabAxisId->GetStringSelection());
        GridPanelAxis->SetCellValue(grid2mod, 6, AxisTabDeadZone->GetValue());
        GridPanelAxis->SetCellValue(grid2mod, 7, AxisTabSensitivity->GetValue());
        GridPanelAxis->SetCellValue(grid2mod, 8, AxisTabAcceleration->GetValue());
        GridPanelAxis->SetCellValue(grid2mod, 9, AxisTabShape->GetStringSelection());
        GridPanelAxis->SetCellValue(grid2mod, 10, AxisTabBufferSize->GetValue());
        GridPanelAxis->SetCellValue(grid2mod, 11, AxisTabFilter->GetValue());
        string l = string(AxisTabLabel->GetValue().mb_str());
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
          AxisTabLabel->SetValue(wxString(l.c_str(), wxConvUTF8));
          for(int i=0; i<GridPanelAxis->GetNumberCols(); ++i)
          {
            GridPanelAxis->SetCellBackgroundColour(grid2mod, i, GridPanelAxis->GetDefaultCellBackgroundColour());
          }
        }
        GridPanelAxis->SetCellValue(grid2mod, 12, AxisTabLabel->GetValue());
        if(MenuItemLinkControls->IsChecked())
        {
          updateAxisConfigurations();
        }
        Button3->Enable();
        Button7->Enable();
        Button5->SetLabel(_("Modify"));
    }
    GridPanelAxis->AutoSizeColumns();
    refresh_gui();
}

void configFrame::updateAxisConfigurations()
{
    int k;

    std::list<AxisMapper>* axisMappers;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); ++it)
      {
          if(it->GetDevice()->GetType() == string(GridPanelAxis->GetCellValue(grid2mod, 0).mb_str())
              && it->GetDevice()->GetName() == string(GridPanelAxis->GetCellValue(grid2mod, 1).mb_str())
              && it->GetDevice()->GetId() == string(GridPanelAxis->GetCellValue(grid2mod, 2).mb_str())
              && it->GetEvent()->GetType() == string(GridPanelAxis->GetCellValue(grid2mod, 3).mb_str())
              && it->GetEvent()->GetId() == string(GridPanelAxis->GetCellValue(grid2mod, 4).mb_str())
              && it->GetAxis() == string(GridPanelAxis->GetCellValue(grid2mod, 5).mb_str()))
          {
              it->GetDevice()->SetType(string(AxisTabDeviceType->GetLabel().mb_str()));
              it->GetDevice()->SetId(string(AxisTabDeviceId->GetLabel().mb_str()));
              it->GetDevice()->SetName(string(AxisTabDeviceName->GetLabel().mb_str()));
              it->GetEvent()->SetType(string(AxisTabEventType->GetStringSelection().mb_str()));
              it->GetEvent()->SetId(string(AxisTabEventId->GetLabel().mb_str()));
              it->SetAxis(string(AxisTabAxisId->GetStringSelection().mb_str()));
              it->SetLabel(string(AxisTabLabel->GetValue().mb_str()));
          }
      }
    }
}

void configFrame::OnAxisTabShapeSelect(wxCommandEvent& event)
{
    if(AxisTabEventType->GetStringSelection() == _("axis"))
    {
        if(AxisTabShape->GetStringSelection().IsEmpty())
        {
            AxisTabShape->SetSelection(1);
        }
    }
}

void configFrame::OnMenuItemCopyConfiguration(wxCommandEvent& event)
{
  save_current();
  tempConfiguration = *configFile.GetController(currentController)->GetConfiguration(currentConfiguration);
}

void configFrame::OnMenuItemPasteConfiguration(wxCommandEvent& event)
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

void configFrame::OnMenuItemCopyController(wxCommandEvent& event)
{
  save_current();
  tempController = *configFile.GetController(currentController);
}

void configFrame::OnMenuItemPasteController(wxCommandEvent& event)
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

void configFrame::OnButtonDeleteTrigger(wxCommandEvent& event)
{
    ProfileTriggerDeviceType->SetLabel(wxEmptyString);
    ProfileTriggerDeviceName->SetLabel(wxEmptyString);
    ProfileTriggerDeviceId->SetLabel(wxEmptyString);
    ProfileTriggerButtonId->SetLabel(wxEmptyString);
    CheckBoxSwitchBack->SetValue(false);

    refresh_gui();
}

void configFrame::replaceDevice(wxString wx_device_type)
{
    int k;
    string device_name = "";
    string device_id = "0";
    string device_type = string(wx_device_type.mb_str());

    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;

    if(MenuItemMultipleMiceAndKeyboards->IsChecked())
    {
        wxString msg = _("Press a ") + wx_device_type + _(" button.");
        StatusBar1->SetStatusText(msg);
	      evcatch.run(device_type, "button");
        StatusBar1->SetStatusText(wxEmptyString);
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
      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
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
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); ++it)
      {
          if(it->GetDevice()->GetType() == device_type)
          {
              it->GetDevice()->SetId(device_id);
              it->GetDevice()->SetName(device_name);
          }
      }
      axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); ++it)
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

void configFrame::OnMenuReplaceMouse(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the mouse in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(_("mouse"));
}

void configFrame::OnMenuReplaceKeyboard(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the keyboard in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(_("keyboard"));
}

void configFrame::OnMenuReplaceMouseDPI(wxCommandEvent& event)
{
    int k;
    string device_name = "";
    string device_id = "0";
    string device_type = "mouse";

    std::list<AxisMapper>* axisMappers;

    int old_value, new_value;

    old_value = configFile.GetController(currentController)->GetMouseDPI();

    if(!old_value)
    {
      wxNumberEntryDialog dialog1(this, wxT(""), wxT("Enter a number:"), wxT("Source mouse DPI"), 2000, 100, MAX_DPI);

      if(dialog1.ShowModal() == wxID_OK)
      {
          old_value = dialog1.GetValue();
      }
    }


    if (old_value)
    {
        wxNumberEntryDialog dialog2(this, wxT(""), wxT("Enter a number:"), wxT("Destination mouse DPI"), old_value, 100, MAX_DPI);

        if (dialog2.ShowModal() == wxID_OK)
        {
            new_value = dialog2.GetValue();

            if(old_value == new_value)
            {
              return;
            }

            if(MenuItemMultipleMiceAndKeyboards->IsChecked())
            {
                StatusBar1->SetStatusText(_("Press a mouse button."));
                evcatch.run(device_type, "button");
                StatusBar1->SetStatusText(wxEmptyString);
                device_name = evcatch.GetDeviceName();
                device_id = evcatch.GetDeviceId();
            }

            save_current();

            Controller* controller = configFile.GetController(currentController);

            for(k=0; k<MAX_CONFIGURATIONS; ++k)
            {
              Configuration* config = controller->GetConfiguration(k);

              axisMappers = config->GetAxisMapperList();
              for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); ++it)
              {
                if(it->GetDevice()->GetType() == "mouse" && it->GetEvent()->GetType() == "axis")
                {
                  if(MenuItemMultipleMiceAndKeyboards->IsChecked() && (it->GetDevice()->GetName() != device_name || it->GetDevice()->GetId() != device_id))
                  {
                    continue;
                  }
                  double val = atof(it->GetEvent()->GetMultiplier().c_str());
                  double exp = atof(it->GetEvent()->GetExponent().c_str());
                  val = val * pow((double)old_value / new_value, exp);
                  ostringstream ios;
                  ios << setprecision(2) << val;
                  it->GetEvent()->SetMultiplier(ios.str());
                }
              }
            }

            configFile.GetController(currentController)->SetMouseDPI(new_value);

            load_current();
            refresh_gui();
        }
    }
}

void configFrame::OnLSIncAutoDetectClick(wxCommandEvent& event)
{
  LSIncAutoDetect->Enable(false);

  string old_device_type = string(LSIncDeviceType->GetLabel().mb_str());
  string old_device_name = string(LSIncDeviceName->GetLabel().mb_str());
  string old_device_id = string(LSIncDeviceId->GetLabel().mb_str());
  string old_event_id = string(LSIncButtonId->GetLabel().mb_str());

  auto_detect(LSIncDeviceType, LSIncDeviceName, LSIncDeviceId, _("button"), LSIncButtonId);

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "left_stick")
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(string(LSIncDeviceType->GetLabel().mb_str()));
            dev->SetName(string(LSIncDeviceName->GetLabel().mb_str()));
            dev->SetId(string(LSIncDeviceId->GetLabel().mb_str()));
            ev->SetId(string(LSIncButtonId->GetLabel().mb_str()));
          }
        }
      }
    }
  }

  LSIncAutoDetect->Enable(true);

  refresh_gui();
}

void configFrame::OnLSIncDeleteClick(wxCommandEvent& event)
{
  string old_device_type = string(LSIncDeviceType->GetLabel().mb_str());
  string old_device_name = string(LSIncDeviceName->GetLabel().mb_str());
  string old_device_id = string(LSIncDeviceId->GetLabel().mb_str());
  string old_event_id = string(LSIncButtonId->GetLabel().mb_str());

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "left_stick")
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType("");
            dev->SetName("");
            dev->SetId("");
            ev->SetId("");
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

void configFrame::OnRSIncAutoDetectClick1(wxCommandEvent& event)
{
  RSIncAutoDetect->Enable(false);

  string old_device_type = string(RSIncDeviceType->GetLabel().mb_str());
  string old_device_name = string(RSIncDeviceName->GetLabel().mb_str());
  string old_device_id = string(RSIncDeviceId->GetLabel().mb_str());
  string old_event_id = string(RSIncButtonId->GetLabel().mb_str());

  auto_detect(RSIncDeviceType, RSIncDeviceName, RSIncDeviceId, _("button"), RSIncButtonId);

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "right_stick")
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(string(RSIncDeviceType->GetLabel().mb_str()));
            dev->SetName(string(RSIncDeviceName->GetLabel().mb_str()));
            dev->SetId(string(RSIncDeviceId->GetLabel().mb_str()));
            ev->SetId(string(RSIncButtonId->GetLabel().mb_str()));
          }
        }
      }
    }
  }

  RSIncAutoDetect->Enable(true);

  refresh_gui();
}

void configFrame::OnRSIncDeleteClick(wxCommandEvent& event)
{
  string old_device_type = string(RSIncDeviceType->GetLabel().mb_str());
  string old_device_name = string(RSIncDeviceName->GetLabel().mb_str());
  string old_device_id = string(RSIncDeviceId->GetLabel().mb_str());
  string old_event_id = string(RSIncButtonId->GetLabel().mb_str());

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "right_stick")
        {
          Device* dev = it->GetDeviceUp();
          Event* ev = it->GetEventUp();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType("");
            dev->SetName("");
            dev->SetId("");
            ev->SetId("");
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

void configFrame::OnLSDeadzoneChange(wxSpinEvent& event)
{
    int max = 127-LSDeadzone->GetValue();
    if(max)
    {
        LSSteps->SetValue(LSSteps->GetValue()*LSSteps->GetMax()/max);
        LSSteps->SetRange(1, max);
    }
    refresh_gui();
}

void configFrame::OnRSDeadZoneChange(wxSpinEvent& event)
{
    int max = 127-RSDeadZone->GetValue();
    if(max)
    {
        RSSteps->SetValue(RSSteps->GetValue()*RSSteps->GetMax()/max);
        RSSteps->SetRange(1, max);
    }
    refresh_gui();
}

void configFrame::OnLSDecAutoDetectClick(wxCommandEvent& event)
{
  LSDecAutoDetect->Enable(false);

  string old_device_type = string(LSDecDeviceType->GetLabel().mb_str());
  string old_device_name = string(LSDecDeviceName->GetLabel().mb_str());
  string old_device_id = string(LSDecDeviceId->GetLabel().mb_str());
  string old_event_id = string(LSDecButtonId->GetLabel().mb_str());

  auto_detect(LSDecDeviceType, LSDecDeviceName, LSDecDeviceId, _("button"), LSDecButtonId);

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "left_stick")
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(string(LSDecDeviceType->GetLabel().mb_str()));
            dev->SetName(string(LSDecDeviceName->GetLabel().mb_str()));
            dev->SetId(string(LSDecDeviceId->GetLabel().mb_str()));
            ev->SetId(string(LSDecButtonId->GetLabel().mb_str()));
          }
        }
      }
    }
  }

  LSDecAutoDetect->Enable(true);

  refresh_gui();
}

void configFrame::OnLSDecDeleteClick(wxCommandEvent& event)
{
  string old_device_type = string(LSDecDeviceType->GetLabel().mb_str());
  string old_device_name = string(LSDecDeviceName->GetLabel().mb_str());
  string old_device_id = string(LSDecDeviceId->GetLabel().mb_str());
  string old_event_id = string(LSDecButtonId->GetLabel().mb_str());

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "left_stick")
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType("");
            dev->SetName("");
            dev->SetId("");
            ev->SetId("");
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

void configFrame::OnRSDecAutoDetectClick(wxCommandEvent& event)
{
  RSDecAutoDetect->Enable(false);

  string old_device_type = string(RSDecDeviceType->GetLabel().mb_str());
  string old_device_name = string(RSDecDeviceName->GetLabel().mb_str());
  string old_device_id = string(RSDecDeviceId->GetLabel().mb_str());
  string old_event_id = string(RSDecButtonId->GetLabel().mb_str());

  auto_detect(RSDecDeviceType, RSDecDeviceName, RSDecDeviceId, _("button"), RSDecButtonId);

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "right_stick")
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType(string(RSDecDeviceType->GetLabel().mb_str()));
            dev->SetName(string(RSDecDeviceName->GetLabel().mb_str()));
            dev->SetId(string(RSDecDeviceId->GetLabel().mb_str()));
            ev->SetId(string(RSDecButtonId->GetLabel().mb_str()));
          }
        }
      }
    }
  }

  RSDecAutoDetect->Enable(true);

  refresh_gui();
}

void configFrame::OnRSDecDeleteClick(wxCommandEvent& event)
{
  string old_device_type = string(RSDecDeviceType->GetLabel().mb_str());
  string old_device_name = string(RSDecDeviceName->GetLabel().mb_str());
  string old_device_id = string(RSDecDeviceId->GetLabel().mb_str());
  string old_event_id = string(RSDecButtonId->GetLabel().mb_str());

  if(!old_device_type.empty() && MenuItemLinkControls->IsChecked())
  {
    int k;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensityList = config->GetIntensityList();

      for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
      {
        if(it->GetControl() == "right_stick")
        {
          Device* dev = it->GetDeviceDown();
          Event* ev = it->GetEventDown();
          if(dev->GetType() == old_device_type && dev->GetName() == old_device_name && dev->GetType() == old_device_type && ev->GetId() == old_event_id)
          {
            dev->SetType("");
            dev->SetName("");
            dev->SetId("");
            ev->SetId("");
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

void configFrame::OnMenuSetMouseDPI(wxCommandEvent& event)
{
    wxNumberEntryDialog dialog1(this, wxT(""), wxT("Enter a number:"), wxT("Mouse DPI value"), configFile.GetController(currentController)->GetMouseDPI(), 0, MAX_DPI);
    if (dialog1.ShowModal() == wxID_OK)
    {
       configFile.GetController(currentController)->SetMouseDPI(dialog1.GetValue());
    }
}

void configFrame::OnTextCtrl(wxCommandEvent& event)
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

wxString configFrame::isAlreadyUsed(wxString device_type, wxString device_name, wxString device_id, wxString event_type, wxString event_id, int gridIndex1, int gridIndex2)
{
  for(int i=0; i<GridPanelButton->GetNumberRows(); i++)
  {
    if(gridIndex1 >= 0 && i == gridIndex1)
    {
      continue;
    }

    if(GridPanelButton->GetCellValue(i, 0) == device_type
       && (GridPanelButton->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || GridPanelButton->GetCellValue(i, 1) == device_name)
       && GridPanelButton->GetCellValue(i, 2) == device_id
       && GridPanelButton->GetCellValue(i, 3) == event_type
       && GridPanelButton->GetCellValue(i, 4) == event_id)
    {
      return _("This control is already used in the current profile (Button tab).\nContinue?");
    }
  }

  for(int i=0; i<GridPanelAxis->GetNumberRows(); i++)
  {
    if(gridIndex2 >= 0 && i == gridIndex2)
    {
      continue;
    }

    if(GridPanelAxis->GetCellValue(i, 0) == device_type
       && (GridPanelAxis->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || GridPanelAxis->GetCellValue(i, 1) == device_name)
       && GridPanelAxis->GetCellValue(i, 2) == device_id
       && GridPanelAxis->GetCellValue(i, 3) == event_type
       && GridPanelAxis->GetCellValue(i, 4) == event_id)
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

void configFrame::OnMenuMultipleMK(wxCommandEvent& event)
{
    if(MenuItemMultipleMiceAndKeyboards->IsChecked())
    {
        MenuItemReplaceMouse->Enable(true);
        MenuItemReplaceKeyboard->Enable(true);
    }
    else
    {
        MenuItemReplaceMouse->Enable(false);
        MenuItemReplaceKeyboard->Enable(false);
    }
    replaceDevice(_("mouse"));
    replaceDevice(_("keyboard"));
}

void configFrame::OnMenuUpdate(wxCommandEvent& event)
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

void configFrame::OnMenuAutoBindControls(wxCommandEvent& event)
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
    load_current();
    wxMessageBox(wxT("Auto-bind done!"), wxT("Info"), wxICON_INFORMATION);
  }
}

