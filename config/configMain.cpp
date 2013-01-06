/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "configMain.h"
#include <wx/msgdlg.h>
#include <stdio.h>
#include <sys/types.h>
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

#include <libintl.h>

/*
 * This macro works around the gettext("") non-empty string result.
 */
#define _CN(STRING) (strlen(STRING) ? gettext(STRING) : STRING)

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
const long configFrame::ID_STATICTEXT45 = wxNewId();
const long configFrame::ID_STATICTEXT46 = wxNewId();
const long configFrame::ID_STATICTEXT47 = wxNewId();
const long configFrame::ID_STATICTEXT52 = wxNewId();
const long configFrame::ID_STATICTEXT73 = wxNewId();
const long configFrame::ID_BUTTON19 = wxNewId();
const long configFrame::ID_STATICTEXT74 = wxNewId();
const long configFrame::ID_STATICTEXT53 = wxNewId();
const long configFrame::ID_STATICTEXT1 = wxNewId();
const long configFrame::ID_STATICTEXT54 = wxNewId();
const long configFrame::ID_STATICTEXT55 = wxNewId();
const long configFrame::ID_CHOICE9 = wxNewId();
const long configFrame::ID_SPINCTRL6 = wxNewId();
const long configFrame::ID_CHOICE2 = wxNewId();
const long configFrame::ID_SPINCTRL7 = wxNewId();
const long configFrame::ID_CHOICE6 = wxNewId();
const long configFrame::ID_GRID3 = wxNewId();
const long configFrame::ID_BUTTON21 = wxNewId();
const long configFrame::ID_BUTTON22 = wxNewId();
const long configFrame::ID_BUTTON23 = wxNewId();
const long configFrame::ID_PANEL1 = wxNewId();
const long configFrame::ID_STATICTEXT38 = wxNewId();
const long configFrame::ID_STATICTEXT30 = wxNewId();
const long configFrame::ID_STATICTEXT39 = wxNewId();
const long configFrame::ID_CHOICE4 = wxNewId();
const long configFrame::ID_STATICTEXT40 = wxNewId();
const long configFrame::ID_STATICTEXT2 = wxNewId();
const long configFrame::ID_BUTTON8 = wxNewId();
const long configFrame::ID_STATICTEXT8 = wxNewId();
const long configFrame::ID_STATICTEXT7 = wxNewId();
const long configFrame::ID_STATICTEXT34 = wxNewId();
const long configFrame::ID_TEXTCTRL3 = wxNewId();
const long configFrame::ID_CHOICE5 = wxNewId();
const long configFrame::ID_COMBOBOX1 = wxNewId();
const long configFrame::ID_GRID1 = wxNewId();
const long configFrame::ID_BUTTON4 = wxNewId();
const long configFrame::ID_BUTTON6 = wxNewId();
const long configFrame::ID_BUTTON2 = wxNewId();
const long configFrame::ID_PANEL2 = wxNewId();
const long configFrame::ID_STATICTEXT41 = wxNewId();
const long configFrame::ID_STATICTEXT32 = wxNewId();
const long configFrame::ID_STATICTEXT42 = wxNewId();
const long configFrame::ID_CHOICE7 = wxNewId();
const long configFrame::ID_STATICTEXT43 = wxNewId();
const long configFrame::ID_STATICTEXT21 = wxNewId();
const long configFrame::ID_BUTTON9 = wxNewId();
const long configFrame::ID_STATICTEXT16 = wxNewId();
const long configFrame::ID_STATICTEXT15 = wxNewId();
const long configFrame::ID_STATICTEXT17 = wxNewId();
const long configFrame::ID_STATICTEXT18 = wxNewId();
const long configFrame::ID_STATICTEXT22 = wxNewId();
const long configFrame::ID_STATICTEXT33 = wxNewId();
const long configFrame::ID_STATICTEXT44 = wxNewId();
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

/*
 * \brief Give the original english string for a locale string. \
 *        This function is useful for a few strings only.
 *
 * \param str the locale string
 *
 * \return the original string, or the string itself (if no original string found).
 */
static string reverse_gettext(string str)
{
  if(str == gettext("Increase"))
  {
    return "Increase";
  }
  if(str == gettext("Decrease"))
  {
    return "Decrease";
  }
  if(str == gettext("Circle"))
  {
    return "Circle";
  }
  if(str == gettext("Rectangle"))
  {
    return "Rectangle";
  }
  if(str == gettext("mouse"))
  {
    return "mouse";
  }
  if(str == gettext("keyboard"))
  {
    return "keyboard";
  }
  if(str == gettext("axis"))
  {
    return "axis";
  }
  if(str == gettext("axis up"))
  {
    return "axis up";
  }
  if(str == gettext("axis down"))
  {
    return "axis down";
  }
  if(str == gettext("button"))
  {
    return "button";
  }

  return str;
}

/*
 * \brief This function fills a wxChoice for button to axis bindings.
 *
 * \param choice the wxChoice to fill
 */
static void fillButtonAxisChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(wxT("rstick left"));
    choice->Append(wxT("rstick right"));
    choice->Append(wxT("rstick up"));
    choice->Append(wxT("rstick down"));
    choice->Append(wxT("lstick left"));
    choice->Append(wxT("lstick right"));
    choice->Append(wxT("lstick up"));
    choice->Append(wxT("lstick down"));
    choice->Append(wxT("acc x +"));
    choice->Append(wxT("acc x -"));
    choice->Append(wxT("acc y +"));
    choice->Append(wxT("acc y -"));
    choice->Append(wxT("acc z +"));
    choice->Append(wxT("acc z -"));
    choice->Append(wxT("gyro +"));
    choice->Append(wxT("gyro -"));
    choice->Append(wxT("up"));
    choice->Append(wxT("down"));
    choice->Append(wxT("right"));
    choice->Append(wxT("left"));
    choice->Append(wxT("r1"));
    choice->Append(wxT("r2"));
    choice->Append(wxT("l1"));
    choice->Append(wxT("l2"));
    choice->Append(wxT("circle"));
    choice->Append(wxT("square"));
    choice->Append(wxT("cross"));
    choice->Append(wxT("triangle"));
    choice->SetSelection(choice->FindString(previous));
}

/*
 * \brief This function fills a wxChoice for axis to axis bindings.
 *
 * \param choice the wxChoice to fill
 */
static void fillAxisAxisChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(wxT("rstick x"));
    choice->Append(wxT("rstick y"));
    choice->Append(wxT("lstick x"));
    choice->Append(wxT("lstick y"));
    choice->Append(wxT("acc x"));
    choice->Append(wxT("acc y"));
    choice->Append(wxT("acc z"));
    choice->Append(wxT("gyro"));
    choice->Append(wxT("up"));
    choice->Append(wxT("down"));
    choice->Append(wxT("right"));
    choice->Append(wxT("left"));
    choice->Append(wxT("r1"));
    choice->Append(wxT("r2"));
    choice->Append(wxT("l1"));
    choice->Append(wxT("l2"));
    choice->Append(wxT("circle"));
    choice->Append(wxT("square"));
    choice->Append(wxT("cross"));
    choice->Append(wxT("triangle"));
    choice->SetSelection(choice->FindString(previous));
}

/*
 * \brief This function fills a wxChoice for axis/button to button bindings.
 *
 * \param choice the wxChoice to fill
 */
static void fillButtonChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(wxT("up"));
    choice->Append(wxT("down"));
    choice->Append(wxT("right"));
    choice->Append(wxT("left"));
    choice->Append(wxT("r1"));
    choice->Append(wxT("r2"));
    choice->Append(wxT("r3"));
    choice->Append(wxT("l1"));
    choice->Append(wxT("l2"));
    choice->Append(wxT("l3"));
    choice->Append(wxT("circle"));
    choice->Append(wxT("square"));
    choice->Append(wxT("cross"));
    choice->Append(wxT("triangle"));
    choice->Append(wxT("start"));
    choice->Append(wxT("select"));
    choice->Append(wxT("PS"));
    choice->SetSelection(choice->FindString(previous));
}

/*
 * \brief This function fills a wxChoice for axis intensities.
 *
 * \param choice the wxChoice to fill
 */
static void fillIntensityAxisChoice(wxChoice* choice)
{
    wxString previous = choice->GetStringSelection();
    choice->Clear();
    choice->SetSelection(choice->Append(wxEmptyString));
    choice->Append(wxT("lstick"));
    choice->Append(wxT("rstick"));
    choice->Append(wxT("acc x"));
    choice->Append(wxT("acc y"));
    choice->Append(wxT("acc z"));
    choice->Append(wxT("gyro"));
    choice->Append(wxT("up"));
    choice->Append(wxT("down"));
    choice->Append(wxT("right"));
    choice->Append(wxT("left"));
    choice->Append(wxT("r1"));
    choice->Append(wxT("r2"));
    choice->Append(wxT("l1"));
    choice->Append(wxT("l2"));
    choice->Append(wxT("circle"));
    choice->Append(wxT("square"));
    choice->Append(wxT("cross"));
    choice->Append(wxT("triangle"));
    choice->SetSelection(choice->FindString(previous));
}

/*
 * \brief Compare to strings (not case sensitive).
 *
 * \param first   the first string
 * \param second  the second string
 *
 * \return false if equal, true otherwise
 */
static bool compare_nocase (string first, string second)
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

/*
 * \brief Read labels from the config files, sort them, \
 *        and append them into ButtonTabLabel and AxisTabLabel.
 */
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

/*
 * \brief Constructor.
 *
 * \param file    the config file to open (empty means no file to open)
 * \param parent  the parent window
 * \param id      the parent window id
 */
configFrame::configFrame(wxString file,wxWindow* parent,wxWindowID id)
{
    /*
     * Use the system locale.
     * Messages will be translated if a translation file is available.
     */
    setlocale( LC_ALL, "" );
#ifndef WIN32
    bindtextdomain( "gimx", "/usr/share/locale" );
#else
    bindtextdomain( "gimx", "share/locale" );
#endif
    textdomain( "gimx" );
    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

    //(*Initialize(configFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxMenu* MenuHelp;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItemAbout;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizer24;
    wxFlexGridSizer* FlexGridSizer19;
    wxFlexGridSizer* FlexGridSizer23;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer27;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer25;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer9;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer7;
    wxMenuItem* MenuItemOpen;
    wxStaticBoxSizer* StaticBoxSizer10;
    wxFlexGridSizer* FlexGridSizer29;
    wxStaticBoxSizer* StaticBoxSizer8;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxGridSizer* GridSizer1;
    wxStaticBoxSizer* StaticBoxSizer6;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer21;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer20;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer12;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer17;
    wxStaticBoxSizer* StaticBoxSizer5;
    wxFlexGridSizer* FlexGridSizer28;
    wxFlexGridSizer* FlexGridSizer30;
    
    Create(parent, wxID_ANY, _("Gimx-config"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    GridSizer1 = new wxGridSizer(1, 1, 0, 0);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxSize(-1,570), 0, _T("ID_NOTEBOOK1"));
    PanelOverall = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer10 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer10->AddGrowableRow(1);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Profile trigger"));
    FlexGridSizer13 = new wxFlexGridSizer(1, 9, 0, 0);
    StaticBoxSizer9 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Device"));
    FlexGridSizer17 = new wxFlexGridSizer(1, 3, 0, 0);
    ProfileTriggerDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT35, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer17->Add(ProfileTriggerDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT27, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer17->Add(ProfileTriggerDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProfileTriggerDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT36, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer17->Add(ProfileTriggerDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer9->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13->Add(StaticBoxSizer9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer10 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Button"));
    FlexGridSizer23 = new wxFlexGridSizer(1, 1, 0, 0);
    ProfileTriggerButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT37, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer23->Add(ProfileTriggerButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer10->Add(FlexGridSizer23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13->Add(StaticBoxSizer10, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonAutoDetect = new wxButton(PanelOverall, ID_BUTTON1, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer13->Add(ButtonAutoDetect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Axis intensity"));
    FlexGridSizer19 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer19->AddGrowableRow(1);
    FlexGridSizer20 = new wxFlexGridSizer(1, 4, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Device"));
    FlexGridSizer18 = new wxFlexGridSizer(1, 3, 0, 0);
    IntensityDeviceType = new wxStaticText(PanelOverall, ID_STATICTEXT45, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
    FlexGridSizer18->Add(IntensityDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityDeviceName = new wxStaticText(PanelOverall, ID_STATICTEXT46, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    FlexGridSizer18->Add(IntensityDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityDeviceId = new wxStaticText(PanelOverall, ID_STATICTEXT47, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT47"));
    FlexGridSizer18->Add(IntensityDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer20->Add(StaticBoxSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, PanelOverall, _("Button"));
    FlexGridSizer15 = new wxFlexGridSizer(1, 1, 0, 0);
    IntensityButtonId = new wxStaticText(PanelOverall, ID_STATICTEXT52, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT52"));
    FlexGridSizer15->Add(IntensityButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer20->Add(StaticBoxSizer6, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer24 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticText17 = new wxStaticText(PanelOverall, ID_STATICTEXT73, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT73"));
    FlexGridSizer24->Add(StaticText17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityAutoDetect = new wxButton(PanelOverall, ID_BUTTON19, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON19"));
    FlexGridSizer24->Add(IntensityAutoDetect, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer20->Add(FlexGridSizer24, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer25 = new wxFlexGridSizer(2, 5, 0, 0);
    StaticText18 = new wxStaticText(PanelOverall, ID_STATICTEXT74, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT74"));
    FlexGridSizer25->Add(StaticText18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(PanelOverall, ID_STATICTEXT53, _("DZ"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT53"));
    StaticText6->SetToolTip(_("Dead zone"));
    FlexGridSizer25->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(PanelOverall, ID_STATICTEXT1, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer25->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(PanelOverall, ID_STATICTEXT54, _("Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT54"));
    FlexGridSizer25->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(PanelOverall, ID_STATICTEXT55, _("Axis"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT55"));
    FlexGridSizer25->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityDirection = new wxChoice(PanelOverall, ID_CHOICE9, wxDefaultPosition, wxSize(100,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE9"));
    IntensityDirection->Append(_("Increase"));
    IntensityDirection->SetSelection( IntensityDirection->Append(_("Decrease")) );
    FlexGridSizer25->Add(IntensityDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityDeadZone = new wxSpinCtrl(PanelOverall, ID_SPINCTRL6, _T("32"), wxDefaultPosition, wxSize(50,-1), 0, 0, 127, 32, _T("ID_SPINCTRL6"));
    IntensityDeadZone->SetValue(_T("32"));
    FlexGridSizer25->Add(IntensityDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityShape = new wxChoice(PanelOverall, ID_CHOICE2, wxDefaultPosition, wxSize(75,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer25->Add(IntensityShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensitySteps = new wxSpinCtrl(PanelOverall, ID_SPINCTRL7, _T("3"), wxDefaultPosition, wxSize(50,-1), 0, 1, 127, 3, _T("ID_SPINCTRL7"));
    IntensitySteps->SetValue(_T("3"));
    FlexGridSizer25->Add(IntensitySteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityAxis = new wxChoice(PanelOverall, ID_CHOICE6, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE6"));
    FlexGridSizer25->Add(IntensityAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer20->Add(FlexGridSizer25, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer19->Add(FlexGridSizer20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer21 = new wxFlexGridSizer(1, 2, 0, 0);
    GridIntensity = new wxGrid(PanelOverall, ID_GRID3, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID3"));
    GridIntensity->CreateGrid(0,9);
    GridIntensity->EnableEditing(false);
    GridIntensity->EnableGridLines(true);
    GridIntensity->SetRowLabelSize(25);
    GridIntensity->SetColLabelValue(0, _("Device"));
    GridIntensity->SetColLabelValue(1, wxEmptyString);
    GridIntensity->SetColLabelValue(2, wxEmptyString);
    GridIntensity->SetColLabelValue(3, _("Button"));
    GridIntensity->SetColLabelValue(4, _("Direction"));
    GridIntensity->SetColLabelValue(5, _("Dead zone"));
    GridIntensity->SetColLabelValue(6, _("Shape"));
    GridIntensity->SetColLabelValue(7, _("Steps"));
    GridIntensity->SetColLabelValue(8, _("Axis"));
    GridIntensity->SetDefaultCellFont( GridIntensity->GetFont() );
    GridIntensity->SetDefaultCellTextColour( GridIntensity->GetForegroundColour() );
    FlexGridSizer21->Add(GridIntensity, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer22 = new wxFlexGridSizer(2, 1, 0, 0);
    IntensityAdd = new wxButton(PanelOverall, ID_BUTTON21, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON21"));
    FlexGridSizer22->Add(IntensityAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityRemove = new wxButton(PanelOverall, ID_BUTTON22, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON22"));
    FlexGridSizer22->Add(IntensityRemove, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IntensityModify = new wxButton(PanelOverall, ID_BUTTON23, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON23"));
    FlexGridSizer22->Add(IntensityModify, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer21->Add(FlexGridSizer22, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer19->Add(FlexGridSizer21, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer19, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelOverall->SetSizer(FlexGridSizer10);
    FlexGridSizer10->Fit(PanelOverall);
    FlexGridSizer10->SetSizeHints(PanelOverall);
    PanelButton = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer9->AddGrowableRow(1);
    FlexGridSizer1 = new wxFlexGridSizer(1, 7, 0, 0);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, PanelButton, _("Device"));
    FlexGridSizer12 = new wxFlexGridSizer(1, 3, 0, 0);
    ButtonTabDeviceType = new wxStaticText(PanelButton, ID_STATICTEXT38, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer12->Add(ButtonTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceName = new wxStaticText(PanelButton, ID_STATICTEXT30, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer12->Add(ButtonTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabDeviceId = new wxStaticText(PanelButton, ID_STATICTEXT39, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer12->Add(ButtonTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, PanelButton, _("Event"));
    FlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    ButtonTabEventType = new wxChoice(PanelButton, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    ButtonTabEventType->SetSelection( ButtonTabEventType->Append(_("button")) );
    ButtonTabEventType->Append(_("axis up"));
    ButtonTabEventType->Append(_("axis down"));
    FlexGridSizer2->Add(ButtonTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabEventId = new wxStaticText(PanelButton, ID_STATICTEXT40, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer2->Add(ButtonTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticText3 = new wxStaticText(PanelButton, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer16->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabAutoDetect = new wxButton(PanelButton, ID_BUTTON8, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer16->Add(ButtonTabAutoDetect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer16, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14 = new wxFlexGridSizer(2, 3, 0, 0);
    StaticTextThresholdPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT8, _("Threshold"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer14->Add(StaticTextThresholdPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextButtonPanelButton = new wxStaticText(PanelButton, ID_STATICTEXT7, _("Button"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer14->Add(StaticTextButtonPanelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextLabel = new wxStaticText(PanelButton, ID_STATICTEXT34, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer14->Add(StaticTextLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabThreshold = new wxTextCtrl(PanelButton, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    ButtonTabThreshold->Disable();
    FlexGridSizer14->Add(ButtonTabThreshold, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabButtonId = new wxChoice(PanelButton, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    FlexGridSizer14->Add(ButtonTabButtonId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTabLabel = new wxComboBox(PanelButton, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
    FlexGridSizer14->Add(ButtonTabLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer14, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    GridPanelButton = new wxGrid(PanelButton, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
    GridPanelButton->CreateGrid(0,8);
    GridPanelButton->EnableEditing(false);
    GridPanelButton->EnableGridLines(true);
    GridPanelButton->SetRowLabelSize(25);
    GridPanelButton->SetColLabelValue(0, _("Device"));
    GridPanelButton->SetColLabelValue(1, wxEmptyString);
    GridPanelButton->SetColLabelValue(2, wxEmptyString);
    GridPanelButton->SetColLabelValue(3, _("Event"));
    GridPanelButton->SetColLabelValue(4, wxEmptyString);
    GridPanelButton->SetColLabelValue(5, _("Threshold"));
    GridPanelButton->SetColLabelValue(6, _("Button"));
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
    StaticBoxSizer7 = new wxStaticBoxSizer(wxHORIZONTAL, PanelAxis, _("Device"));
    FlexGridSizer27 = new wxFlexGridSizer(1, 3, 0, 0);
    AxisTabDeviceType = new wxStaticText(PanelAxis, ID_STATICTEXT41, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer27->Add(AxisTabDeviceType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceName = new wxStaticText(PanelAxis, ID_STATICTEXT32, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer27->Add(AxisTabDeviceName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeviceId = new wxStaticText(PanelAxis, ID_STATICTEXT42, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer27->Add(AxisTabDeviceId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer7->Add(FlexGridSizer27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(StaticBoxSizer7, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, PanelAxis, _("Event"));
    FlexGridSizer28 = new wxFlexGridSizer(1, 2, 0, 0);
    AxisTabEventType = new wxChoice(PanelAxis, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    AxisTabEventType->SetSelection( AxisTabEventType->Append(_("axis")) );
    AxisTabEventType->Append(_("button"));
    FlexGridSizer28->Add(AxisTabEventType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabEventId = new wxStaticText(PanelAxis, ID_STATICTEXT43, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer28->Add(AxisTabEventId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(StaticBoxSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer29 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticTextEmptyPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT21, wxEmptyString, wxDefaultPosition, wxSize(30,17), 0, _T("ID_STATICTEXT21"));
    FlexGridSizer29->Add(StaticTextEmptyPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAutoDetect = new wxButton(PanelAxis, ID_BUTTON9, _("Auto detect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer29->Add(AxisTabAutoDetect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer29, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30 = new wxFlexGridSizer(2, 7, 0, 0);
    StaticTextAxisPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT16, _("Axis"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer30->Add(StaticTextAxisPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDZPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT15, _("DZ"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    StaticTextDZPanelAxis->SetToolTip(_("Dead zone"));
    FlexGridSizer30->Add(StaticTextDZPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextSensPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT17, _("Sens."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    StaticTextSensPanelAxis->SetToolTip(_("Sensitivity"));
    FlexGridSizer30->Add(StaticTextSensPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextAccelPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT18, _("Accel."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    StaticTextAccelPanelAxis->SetToolTip(_("Acceleration"));
    FlexGridSizer30->Add(StaticTextAccelPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShapePanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT22, _("Shape"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer30->Add(StaticTextShapePanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextSmoothingPanelAxis = new wxStaticText(PanelAxis, ID_STATICTEXT33, _("Smoothing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer30->Add(StaticTextSmoothingPanelAxis, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(PanelAxis, ID_STATICTEXT44, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer30->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAxisId = new wxChoice(PanelAxis, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE8"));
    FlexGridSizer30->Add(AxisTabAxisId, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabDeadZone = new wxTextCtrl(PanelAxis, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    AxisTabDeadZone->Disable();
    AxisTabDeadZone->SetToolTip(_("Dead zone [0..64]"));
    FlexGridSizer30->Add(AxisTabDeadZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabSensitivity = new wxTextCtrl(PanelAxis, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(59,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    AxisTabSensitivity->Disable();
    AxisTabSensitivity->SetToolTip(_("Sensitivity [-100.00..100.00]"));
    FlexGridSizer30->Add(AxisTabSensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabAcceleration = new wxTextCtrl(PanelAxis, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(55,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
    AxisTabAcceleration->Disable();
    AxisTabAcceleration->SetToolTip(_("Acceleration [0.00..2.00]"));
    FlexGridSizer30->Add(AxisTabAcceleration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabShape = new wxChoice(PanelAxis, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    AxisTabShape->SetSelection( AxisTabShape->Append(wxEmptyString) );
    AxisTabShape->Append(_("Circle"));
    AxisTabShape->Append(_("Rectangle"));
    AxisTabShape->Disable();
    FlexGridSizer30->Add(AxisTabShape, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(1, 2, 0, 0);
    AxisTabBufferSize = new wxTextCtrl(PanelAxis, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(27,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    AxisTabBufferSize->Disable();
    AxisTabBufferSize->SetToolTip(_("Buffer size [1..30]"));
    FlexGridSizer11->Add(AxisTabBufferSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabFilter = new wxTextCtrl(PanelAxis, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(41,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    AxisTabFilter->Disable();
    AxisTabFilter->SetToolTip(_("Filter [0.00..1.00]"));
    FlexGridSizer11->Add(AxisTabFilter, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    AxisTabLabel = new wxComboBox(PanelAxis, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(150,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX2"));
    FlexGridSizer30->Add(AxisTabLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer30, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(1, 2, 0, 0);
    GridPanelAxis = new wxGrid(PanelAxis, ID_GRID2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID2"));
    GridPanelAxis->CreateGrid(0,13);
    GridPanelAxis->EnableEditing(false);
    GridPanelAxis->EnableGridLines(true);
    GridPanelAxis->SetRowLabelSize(25);
    GridPanelAxis->SetDefaultColSize(100, true);
    GridPanelAxis->SetColLabelValue(0, _("Device"));
    GridPanelAxis->SetColLabelValue(1, wxEmptyString);
    GridPanelAxis->SetColLabelValue(2, wxEmptyString);
    GridPanelAxis->SetColLabelValue(3, _("Event"));
    GridPanelAxis->SetColLabelValue(4, wxEmptyString);
    GridPanelAxis->SetColLabelValue(5, _("Axis"));
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
    Connect(ID_BUTTON19,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnIntensityAutoDetectClick);
    Connect(ID_CHOICE6,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&configFrame::OnIntensityAxisSelect);
    Connect(ID_BUTTON21,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnIntensityAddClick);
    Connect(ID_BUTTON22,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnIntensityRemoveClick);
    Connect(ID_BUTTON23,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&configFrame::OnIntensityModifyClick);
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
    fillIntensityAxisChoice(IntensityAxis);

    currentController = 0;
    currentConfiguration = 0;

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

    /* Retrieve config/ directory location */
    default_directory.Append(wxFileName::GetHomeDir());
    default_directory.Append(wxT(APP_DIR));

	  /* Init user's config directory */
    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( _("Can't init ~/.emuclient/config!"), _("Error"), wxICON_ERROR);
    }
#endif

    default_directory.Append(wxT(CONFIG_DIR));

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
    GridIntensity->AutoSizeColumns();

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
          wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
        }
        else if(ret > 0)
        {
          wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), _("Info"), wxICON_INFORMATION);
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
        MenuFile->Enable(idMenuSave, true);
        FileDialog1->SetFilename(file);
      }
      else
      {
        wxMessageBox( _("Cannot open config file: ") + file, _("Error"), wxICON_ERROR);
      }
    }

    readLabels();
    
    int cW, cH;
    GetTextExtent(wxT("_"), &cW, &cH);
    StaticBoxSizer3->SetMinSize(StaticBoxSizer3->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer4->SetMinSize(StaticBoxSizer4->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer5->SetMinSize(StaticBoxSizer5->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer6->SetMinSize(StaticBoxSizer6->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer7->SetMinSize(StaticBoxSizer7->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer8->SetMinSize(StaticBoxSizer8->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer9->SetMinSize(StaticBoxSizer9->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    StaticBoxSizer10->SetMinSize(StaticBoxSizer10->GetStaticBox()->GetLabel().size() * cW + 20, -1);
    refresh_gui();
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

/*
 * \brief Display the about menu.
 */
void configFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(wxT(INFO_DESCR)) + wxString(wxT("\n")) + wxString(wxT(INFO_YEAR)) + wxString(wxT(" ")) + wxString(wxT(INFO_DEV)) + wxString(wxT(" ")) + wxString(wxT(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

/*
 * \brief Method called on File>New menu click.
 */
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
    IntensityModify->SetLabel(_("Modify"));
    IntensityAdd->Enable();
    IntensityRemove->Enable();
}

/*
 * \brief Method called on Panel_Button>Add button click.
 */
void configFrame::OnButtonAddPanelButton(wxCommandEvent& event)
{
    if(ButtonTabEventId->GetLabel().IsEmpty())
    {
        wxMessageBox( _("Please detect an Event!"), _("Error"), wxICON_ERROR);
        return;
    }
    if(ButtonTabButtonId->GetStringSelection().IsEmpty())
    {
        wxMessageBox( _("Please select a Button!"), _("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(ButtonTabDeviceType->GetLabel(), ButtonTabDeviceName->GetLabel(), ButtonTabDeviceId->GetLabel(), ButtonTabEventType->GetStringSelection(), ButtonTabEventId->GetLabel(), -1, -1, -1);

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

/*
 * \brief Method called on Panel_Axis>Add button click.
 */
void configFrame::OnButtonAddPanelAxis(wxCommandEvent& event)
{
    if(AxisTabEventId->GetLabel().IsEmpty())
    {
        wxMessageBox( _("Please detect an Event!"), _("Error"), wxICON_ERROR);
        return;
    }
    if(AxisTabAxisId->GetStringSelection().IsEmpty())
    {
        wxMessageBox( _("Please select an Axis!"), _("Error"), wxICON_ERROR);
        return;
    }

    wxString check = isAlreadyUsed(AxisTabDeviceType->GetLabel(), AxisTabDeviceName->GetLabel(), AxisTabDeviceId->GetLabel(), AxisTabEventType->GetStringSelection(), AxisTabEventId->GetLabel(), -1, -1, -1);

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

/*
 * \brief Delete rows in other profiles (same binding + same panel). \
 *        Only called if MenuItemLinkControls is checked.
 *
 * \param grid  the wxGrid (GridPanelButton or GridPanelAxis).
 * \param row   the binding
 */
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
        if (it->GetDevice()->GetType() == reverse_gettext(old_device_type)
            && it->GetDevice()->GetName() == old_device_name
            && it->GetDevice()->GetId() == old_device_id
            && it->GetEvent()->GetType() == reverse_gettext(old_event_type)
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
        if (it->GetDevice()->GetType() == reverse_gettext(old_device_type)
            && it->GetDevice()->GetName() == old_device_name
            && it->GetDevice()->GetId() == old_device_id
            && it->GetEvent()->GetType() == reverse_gettext(old_event_type)
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
  else if(grid == GridIntensity)
  {
    string old_device_type = string(GridIntensity->GetCellValue(row, 0).mb_str());
    string old_device_name = string(GridIntensity->GetCellValue(row, 1).mb_str());
    string old_device_id = string(GridIntensity->GetCellValue(row, 2).mb_str());
    string old_event_id = string(GridIntensity->GetCellValue(row, 3).mb_str());
    string old_axis_id = string(GridIntensity->GetCellValue(row, 8).mb_str());
  
    Controller* controller = configFile.GetController(currentController);

    for(unsigned int k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      if(k == currentConfiguration)
      {
        continue;
      }
      
      Configuration* config = controller->GetConfiguration(k);

      std::list<Intensity>* intensities = config->GetIntensityList();
      for(std::list<Intensity>::iterator it = intensities->begin(); it!=intensities->end(); )
      {
          if(it->GetDevice()->GetType() == reverse_gettext(old_device_type)
              && it->GetDevice()->GetName() == old_device_name
              && it->GetDevice()->GetId() == old_device_id
              && it->GetEvent()->GetId() == old_event_id
              && it->GetControl() == old_axis_id)
        {
          it = intensities ->erase(it);
        }
        else
        {
          ++it;
        }
      }
    }
  }
}

/*
 * \brief Get the selected rows of a wxGrid.
 *
 * \param grid  the wxGrid
 *
 * \return an array containing the selected rows
 */
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

/*
 * \brief Delete the selected rows of a wxGrid.
 *
 * \param grid  the wxGrid
 */
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

/*
 * \brief Method called on Panel_Button>Remove click.
 */
void configFrame::OnButtonRemovePanelButton(wxCommandEvent& event)
{
    configFrame::DeleteSelectedRows(GridPanelButton);
    refresh_gui();
}

/*
 * \brief Method called on Panel_Axis>Remove click.
 */
void configFrame::OnButtonRemovePanelAxis(wxCommandEvent& event)
{
    configFrame::DeleteSelectedRows(GridPanelAxis);
    refresh_gui();
}

/*
 * \brief Method called on Panel_Axis>Event_type selection.
 */
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

/*
 * \brief Method called on Panel_Button>Event_type selection.
 */
void configFrame::OnButtonTabEventTypeSelect(wxCommandEvent& event)
{
    ButtonTabDeviceType->SetLabel(wxEmptyString);
    ButtonTabDeviceName->SetLabel(wxEmptyString);
    ButtonTabDeviceId->SetLabel(wxEmptyString);
    ButtonTabEventId->SetLabel(wxEmptyString);

    if(ButtonTabEventType->GetStringSelection() = _("button"))
    {
        ButtonTabThreshold->Disable();
        ButtonTabThreshold->SetValue(wxEmptyString);
    }
    else
    {
        ButtonTabThreshold->Enable();
        if(ButtonTabEventType->GetStringSelection() == _("axis down"))
        {
          ButtonTabThreshold->SetValue(wxT("-10"));
        }
        else if(ButtonTabEventType->GetStringSelection() == _("axis up"))
        {
          ButtonTabThreshold->SetValue(wxT("10"));
        }
    }
    refresh_gui();
}

/*
 * \brief Auto detect a device and an event.
 *
 * \param event_type   the event type (translated) [IN]
 *
 * \param device_type  the device type (translated) [OUT]
 * \param device_name  the device name [OUT]
 * \param device_id    the device id [OUT]
 * \param event_id     the event id [OUT]
 */
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
    evcatch.run("", reverse_gettext(string(event_type.mb_str())));
    StatusBar1->SetStatusText(wxEmptyString);

    device_type->SetLabel(wxString(_CN(evcatch.GetDeviceType().c_str()), wxConvUTF8));

    if(MenuItemMultipleMiceAndKeyboards->IsChecked() || evcatch.GetDeviceType() == "joystick")
    {
      device_name->SetLabel(wxString(evcatch.GetDeviceName().c_str(), wxConvUTF8));
      device_id->SetLabel( wxString(evcatch.GetDeviceId().c_str(), wxConvUTF8));
    }
    else
    {
      device_name->SetLabel(wxEmptyString);
      device_id->SetLabel(wxT("0"));
    }

    event_id->SetLabel(wxString(evcatch.GetEventId().c_str(), wxConvUTF8));
}

/*
 * \brief Method called on Overall_Panel>Auto_detect_Trigger click. \
 *        It also updates profiles that had the same trigger.
 */
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

        if(dev->GetType() == reverse_gettext(old_device_type) && dev->GetName() == old_device_name && dev->GetId() == old_device_id && ev->GetId().c_str() == old_event_id)
        {
          dev->SetType(reverse_gettext(string(ProfileTriggerDeviceType->GetLabel().mb_str())));
          dev->SetName(string(ProfileTriggerDeviceName->GetLabel().mb_str()));
          dev->SetId(string(ProfileTriggerDeviceId->GetLabel().mb_str()));
          ev->SetId(string(ProfileTriggerButtonId->GetLabel().mb_str()));
        }
      }
    }

    ButtonAutoDetect->Enable(true);

    refresh_gui();
}

/*
 * \brief Method called on Button_Panel>Auto_detect click.
 */
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
          ButtonTabThreshold->SetValue(wxT("-10"));
        }
        else if(ButtonTabEventType->GetStringSelection() == _("axis up"))
        {
          ButtonTabThreshold->SetValue(wxT("10"));
        }
    }

    fillButtonChoice(ButtonTabButtonId);

    refresh_gui();

    ButtonTabAutoDetect->Enable(true);
}

/*
 * \brief Method called on Axis_Panel>Auto_detect click.
 */
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
          AxisTabAcceleration->SetValue(wxT("1.00"));
          AxisTabShape->Enable();
          AxisTabShape->SetSelection(1);
          if(evcatch.GetDeviceType() == "mouse")
          {
              AxisTabDeadZone->SetValue(wxT("20"));
              AxisTabSensitivity->SetValue(wxT("1.00"));
              AxisTabBufferSize->Enable();
              AxisTabBufferSize->SetValue(wxT("1"));
              AxisTabFilter->Enable();
              AxisTabFilter->SetValue(wxT("0.00"));
          }
          else if(evcatch.GetDeviceType() == "joystick")
          {
              AxisTabDeadZone->SetValue(wxT("0"));
              if(!AxisTabAxisId->GetStringSelection().Contains(wxT("stick")))
              {
                  AxisTabSensitivity->SetValue(wxT("0.008"));
              }
              else
              {
                  AxisTabSensitivity->SetValue(wxT("0.004"));
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

/*
 * \brief Save the current profile.
 */
void configFrame::save_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Save Trigger
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetType(reverse_gettext(string(ProfileTriggerDeviceType->GetLabel().mb_str())));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetName(string(ProfileTriggerDeviceName->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->SetId(string(ProfileTriggerDeviceId->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetEvent()->SetId(string(ProfileTriggerButtonId->GetLabel().mb_str()));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetSwitchBack(string(CheckBoxSwitchBack->GetValue()?"yes":"no"));
    configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->SetDelay(ProfileTriggerDelay->GetValue());
    //Save axis Intensity
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    intensityList->erase(intensityList->begin(), intensityList->end());
    for(int i=0; i<GridIntensity->GetNumberRows(); i++)
    {
      intensityList->push_front(Intensity(string(GridIntensity->GetCellValue(i, 8).mb_str()), reverse_gettext(string(GridIntensity->GetCellValue(i, 0).mb_str())),
          string(GridIntensity->GetCellValue(i, 2).mb_str()), string(GridIntensity->GetCellValue(i, 1).mb_str()), string(GridIntensity->GetCellValue(i, 3).mb_str()), reverse_gettext(string(GridIntensity->GetCellValue(i, 4).mb_str())),
          wxAtoi(GridIntensity->GetCellValue(i, 5)), wxAtoi(GridIntensity->GetCellValue(i, 7)), reverse_gettext(string(GridIntensity->GetCellValue(i, 6).mb_str()))));
    }
    //Save ButtonMappers
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    buttonMappers->erase(buttonMappers->begin(), buttonMappers->end());
    for(int i=0; i<GridPanelButton->GetNumberRows(); i++)
    {
        buttonMappers->push_front(ButtonMapper(reverse_gettext(string(GridPanelButton->GetCellValue(i, 0).mb_str())), string(GridPanelButton->GetCellValue(i, 2).mb_str()), string(GridPanelButton->GetCellValue(i, 1).mb_str()),
            reverse_gettext(string(GridPanelButton->GetCellValue(i, 3).mb_str())), string(GridPanelButton->GetCellValue(i, 4).mb_str()), string(GridPanelButton->GetCellValue(i, 5).mb_str()), string(GridPanelButton->GetCellValue(i, 6).mb_str()),
            string(GridPanelButton->GetCellValue(i, 7).mb_str())));
    }
    //Save AxisMappers
    axisMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetAxisMapperList();
    axisMappers->erase(axisMappers->begin(), axisMappers->end());
    for(int i=0; i<GridPanelAxis->GetNumberRows(); i++)
    {
        axisMappers->push_front(AxisMapper(reverse_gettext(string(GridPanelAxis->GetCellValue(i, 0).mb_str())), string(GridPanelAxis->GetCellValue(i, 2).mb_str()), string(GridPanelAxis->GetCellValue(i, 1).mb_str()),
            reverse_gettext(string(GridPanelAxis->GetCellValue(i, 3).mb_str())), string(GridPanelAxis->GetCellValue(i, 4).mb_str()), string(GridPanelAxis->GetCellValue(i, 5).mb_str()), string(GridPanelAxis->GetCellValue(i, 6).mb_str()),
            string(GridPanelAxis->GetCellValue(i, 7).mb_str()), string(GridPanelAxis->GetCellValue(i, 8).mb_str()), reverse_gettext(string(GridPanelAxis->GetCellValue(i, 9).mb_str())), string(GridPanelAxis->GetCellValue(i, 10).mb_str()),
            string(GridPanelAxis->GetCellValue(i, 11).mb_str()), string(GridPanelAxis->GetCellValue(i, 12).mb_str())));
    }

}

/*
 * \brief Load the current profile.
 */
void configFrame::load_current()
{
    std::list<ButtonMapper>* buttonMappers;
    std::list<AxisMapper>* axisMappers;
    std::list<Intensity>* intensityList;
    //Load Trigger
    ProfileTriggerDeviceType->SetLabel(wxString(_CN(configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetTrigger()->GetDevice()->GetType().c_str()),wxConvUTF8));
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
    //Load axis intensities
    GridIntensity->DeleteRows(0, GridIntensity->GetNumberRows());
    intensityList = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetIntensityList();
    for(std::list<Intensity>::iterator it = intensityList->begin(); it!=intensityList->end(); ++it)
    {
      GridIntensity->InsertRows();
      GridIntensity->SetCellValue(0, 0, wxString(_CN(it->GetDevice()->GetType().c_str()),wxConvUTF8));
      GridIntensity->SetCellValue(0, 1, wxString(it->GetDevice()->GetName().c_str(),wxConvUTF8));
      GridIntensity->SetCellValue(0, 2, wxString(it->GetDevice()->GetId().c_str(),wxConvUTF8));
      GridIntensity->SetCellValue(0, 3, wxString(it->GetEvent()->GetId().c_str(),wxConvUTF8));
      GridIntensity->SetCellValue(0, 4, wxString(_CN(it->GetDirection().c_str()),wxConvUTF8));
      wxString dz;
      dz << it->GetDeadZone();
      GridIntensity->SetCellValue(0, 5, dz);
      GridIntensity->SetCellValue(0, 6, wxString(_CN(it->GetShape().c_str()),wxConvUTF8));
      wxString steps;
      steps << it->GetSteps();
      GridIntensity->SetCellValue(0, 7, steps);
      GridIntensity->SetCellValue(0, 8, wxString(it->GetControl().c_str(),wxConvUTF8));
    }
    GridIntensity->AutoSizeColumns();
    //Load ButtonMappers
    GridPanelButton->DeleteRows(0, GridPanelButton->GetNumberRows());
    buttonMappers = configFile.GetController(currentController)->GetConfiguration(currentConfiguration)->GetButtonMapperList();
    for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); ++it)
    {
        GridPanelButton->InsertRows();
        GridPanelButton->SetCellValue(0, 0, wxString(_CN(it->GetDevice()->GetType().c_str()),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 1, wxString(it->GetDevice()->GetName().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 2, wxString(it->GetDevice()->GetId().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 3, wxString(_CN(it->GetEvent()->GetType().c_str()),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 4, wxString(it->GetEvent()->GetId().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 5, wxString(it->GetEvent()->GetThreshold().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 6, wxString(it->GetButton().c_str(),wxConvUTF8));
        GridPanelButton->SetCellValue(0, 7, wxString(it->GetLabel().c_str(),wxConvUTF8));
        if(it->GetLabel().find(_CN(", not found")) != string::npos || it->GetLabel().find(_CN(", duplicate")) != string::npos )
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
        GridPanelAxis->SetCellValue(0, 0, wxString(_CN(it->GetDevice()->GetType().c_str()),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 1, wxString(it->GetDevice()->GetName().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 2, wxString(it->GetDevice()->GetId().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 3, wxString(_CN(it->GetEvent()->GetType().c_str()),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 4, wxString(it->GetEvent()->GetId().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 5, wxString(it->GetAxis().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 6, wxString(it->GetEvent()->GetDeadZone().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 7, wxString(it->GetEvent()->GetMultiplier().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 8, wxString(it->GetEvent()->GetExponent().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 9, wxString(_CN(it->GetEvent()->GetShape().c_str()),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 10, wxString(it->GetEvent()->GetBufferSize().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 11, wxString(it->GetEvent()->GetFilter().c_str(),wxConvUTF8));
        GridPanelAxis->SetCellValue(0, 12, wxString(it->GetLabel().c_str(),wxConvUTF8));
        if(it->GetLabel().find(_CN(", not found")) != string::npos || it->GetLabel().find(_CN(", duplicate")) != string::npos )
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

/*
 * \brief Refresh all panels.
 */
void configFrame::refresh_gui()
{
    PanelOverall->Layout();
    PanelButton->Layout();
    PanelAxis->Layout();
    Refresh();
}

/*
 * \brief Method called on File>Open click.
 */
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
      wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
    else if(ret > 0)
    {
      wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), _("Info"), wxICON_INFORMATION);
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
    IntensityModify->SetLabel(_("Modify"));
    IntensityAdd->Enable();
    IntensityRemove->Enable();
}

/*
 * \brief Method called on controller selection. \
 *        It loads the first profile and refreshes the GUI.
 */
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
    IntensityModify->SetLabel(_("Modify"));
    IntensityAdd->Enable();
    IntensityRemove->Enable();
    refresh_gui();
}

/*
 * \brief Method called on profile selection. \
 *        It save the previous profile, loads the new profile and refreshes the GUI.
 */
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
  IntensityModify->SetLabel(_("Modify"));
  IntensityAdd->Enable();
  IntensityRemove->Enable();
  refresh_gui();
}

/*
 * \bried Method called on File>Save click. \
 *        It saves the current profile and writes the config file on the disk.
 */
void configFrame::OnMenuSave(wxCommandEvent& event)
{
    wxString end;
    save_current();
    if(configFile.WriteConfigFile() < 0)
    {
      wxMessageBox(_("Can't save ") + wxString(configFile.GetFilePath().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
    ButtonTabModify->SetLabel(_("Modify"));
    Button5->SetLabel(_("Modify"));
    Button3->Enable();
    ButtonTabAdd->Enable();
    ButtonTabRemove->Enable();
    Button7->Enable();
    IntensityModify->SetLabel(_("Modify"));
    IntensityAdd->Enable();
    IntensityRemove->Enable();
}

/*
 * \bried Method called on File>Save_As click. \
 *        It asks the user for a config location & name, and calls OnMenuSave.
 */
void configFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Save config file"), wxT(""), wxT(""), _("XML files (*.xml)|*.xml"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    saveFileDialog.SetDirectory(default_directory);
    saveFileDialog.SetFilename(FileDialog1->GetFilename());

    if ( saveFileDialog.ShowModal() == wxID_CANCEL ) return;

    wxString FileName = saveFileDialog.GetPath();

    if ( FileName.IsEmpty() ) return;

    configFile.SetFilePath(string(FileName.mb_str()));

    OnMenuSave(event);

    MenuFile->Enable(idMenuSave, true);
}

/*
 * \bried Method called on Button_Panel>Modify/Apply click. \
 *        It loads the selected line into the edition fields (Modify). \
 *        It saves the edited binding, and eventually updates other profiles (Apply).
 */
void configFrame::OnButtonModifyButton(wxCommandEvent& event)
{
    wxArrayInt array = GetGridSelectedRows(GridPanelButton);
    int count = array.GetCount();

    if(ButtonTabModify->GetLabel() == _("Modify"))
    {
        if(count == 0)
        {
            wxMessageBox( _("Please select a line of the table."), _("Info"), wxICON_INFORMATION);
            return;
        }
        else if(count > 1)
        {
            wxMessageBox( _("Please select a SINGLE line of the table."), _("Info"), wxICON_INFORMATION);
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
          wxMessageBox(_("Please detect an Event!"), _("Error"), wxICON_ERROR);
          return;
        }
        if(ButtonTabButtonId->GetStringSelection().IsEmpty())
        {
            wxMessageBox( _("Please select a Button!"), _("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(ButtonTabDeviceType->GetLabel(), ButtonTabDeviceName->GetLabel(), ButtonTabDeviceId->GetLabel(), ButtonTabEventType->GetStringSelection(), ButtonTabEventId->GetLabel(), grid1mod, -1, -1);

        if(!check.IsEmpty())
        {
          int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            return;
          }
        }

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

        if(MenuItemLinkControls->IsChecked())
        {
          updateButtonConfigurations();
        }

        GridPanelButton->SetCellValue(grid1mod, 0, ButtonTabDeviceType->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 1, ButtonTabDeviceName->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 2, ButtonTabDeviceId->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 3, ButtonTabEventType->GetStringSelection());
        GridPanelButton->SetCellValue(grid1mod, 4, ButtonTabEventId->GetLabel());
        GridPanelButton->SetCellValue(grid1mod, 5, ButtonTabThreshold->GetValue());
        GridPanelButton->SetCellValue(grid1mod, 6, ButtonTabButtonId->GetStringSelection());
        GridPanelButton->SetCellValue(grid1mod, 7, ButtonTabLabel->GetValue());

        ButtonTabAdd->Enable();
        ButtonTabRemove->Enable();
        ButtonTabModify->SetLabel(_("Modify"));
    }
    GridPanelButton->AutoSizeColumns();
    refresh_gui();
}

/*
 * \brief Update profiles (panel Button) according to the edited binding.
 */
void configFrame::updateButtonConfigurations()
{
    int k;

    std::list<ButtonMapper>* buttonMappers;

    string old_device_type = reverse_gettext(string(GridPanelButton->GetCellValue(grid1mod, 0).mb_str()));
    string new_device_type = reverse_gettext(string(ButtonTabDeviceType->GetLabel().mb_str()));

    string old_event_type = reverse_gettext(string(GridPanelButton->GetCellValue(grid1mod, 3).mb_str()));
    string new_event_type = reverse_gettext(string(ButtonTabEventType->GetStringSelection().mb_str()));

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      buttonMappers = config->GetButtonMapperList();
      for(std::list<ButtonMapper>::iterator it = buttonMappers->begin(); it!=buttonMappers->end(); ++it)
      {
          if(it->GetDevice()->GetType() == old_device_type
              && it->GetDevice()->GetName() == string(GridPanelButton->GetCellValue(grid1mod, 1).mb_str())
              && it->GetDevice()->GetId() == string(GridPanelButton->GetCellValue(grid1mod, 2).mb_str())
              && it->GetEvent()->GetType() == old_event_type
              && it->GetEvent()->GetId() == string(GridPanelButton->GetCellValue(grid1mod, 4).mb_str())
              && it->GetButton() == string(GridPanelButton->GetCellValue(grid1mod, 6).mb_str()))
          {
              it->GetDevice()->SetType(new_device_type);
              it->GetDevice()->SetId(string(ButtonTabDeviceId->GetLabel().mb_str()));
              it->GetDevice()->SetName(string(ButtonTabDeviceName->GetLabel().mb_str()));
              it->GetEvent()->SetType(new_event_type);
              it->GetEvent()->SetId(string(ButtonTabEventId->GetLabel().mb_str()));
              it->GetEvent()->SetThreshold(string(ButtonTabThreshold->GetValue().mb_str()));
              it->SetButton(string(ButtonTabButtonId->GetStringSelection().mb_str()));
              it->SetLabel(string(ButtonTabLabel->GetValue().mb_str()));
          }
      }
    }
}

/*
 * \bried Method called on Axis_Panel>Modify/Apply click. \
 *        It loads the selected line into the edition fields (Modify). \
 *        It saves the edited binding, and eventually updates other profiles (Apply).
 */
void configFrame::OnButtonModifyAxis(wxCommandEvent& event)
{
    wxArrayInt array = GetGridSelectedRows(GridPanelAxis);
    int count = array.GetCount();

    if(Button5->GetLabel() == _("Modify"))
    {
        if(count == 0)
        {
            wxMessageBox( _("Please select a line of the table."), _("Info"), wxICON_INFORMATION);
            return;
        }
        else if(count > 1)
        {
            wxMessageBox( _("Please select a SINGLE line of the table."), _("Info"), wxICON_INFORMATION);
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
          wxMessageBox(_("Please detect an Event!"), _("Error"), wxICON_ERROR);
          return;
        }
        if(AxisTabAxisId->GetStringSelection().IsEmpty())
        {
            wxMessageBox( _("Please select an Axis!"), _("Error"), wxICON_ERROR);
            return;
        }

        wxString check = isAlreadyUsed(AxisTabDeviceType->GetLabel(), AxisTabDeviceName->GetLabel(), AxisTabDeviceId->GetLabel(), AxisTabEventType->GetStringSelection(), AxisTabEventId->GetLabel(), -1, grid2mod, -1);

        if(!check.IsEmpty())
        {
          int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            return;
          }
        }

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

        if(MenuItemLinkControls->IsChecked())
        {
          updateAxisConfigurations();
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
        GridPanelAxis->SetCellValue(grid2mod, 12, AxisTabLabel->GetValue());

        Button3->Enable();
        Button7->Enable();
        Button5->SetLabel(_("Modify"));
    }
    GridPanelAxis->AutoSizeColumns();
    refresh_gui();
}

/*
 * \brief Update profiles (panel Axis) according to the edited binding.
 */
void configFrame::updateAxisConfigurations()
{
    int k;

    std::list<AxisMapper>* axisMappers;

    string old_device_type = reverse_gettext(string(GridPanelAxis->GetCellValue(grid2mod, 0).mb_str()));
    string new_device_type = reverse_gettext(string(AxisTabDeviceType->GetLabel().mb_str()));

    string old_event_type = reverse_gettext(string(GridPanelAxis->GetCellValue(grid2mod, 3).mb_str()));
    string new_event_type = reverse_gettext(string(AxisTabEventType->GetStringSelection().mb_str()));

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      axisMappers = config->GetAxisMapperList();
      for(std::list<AxisMapper>::iterator it = axisMappers->begin(); it!=axisMappers->end(); ++it)
      {
          if(it->GetDevice()->GetType() == old_device_type
              && it->GetDevice()->GetName() == string(GridPanelAxis->GetCellValue(grid2mod, 1).mb_str())
              && it->GetDevice()->GetId() == string(GridPanelAxis->GetCellValue(grid2mod, 2).mb_str())
              && it->GetEvent()->GetType() == old_event_type
              && it->GetEvent()->GetId() == string(GridPanelAxis->GetCellValue(grid2mod, 4).mb_str())
              && it->GetAxis() == string(GridPanelAxis->GetCellValue(grid2mod, 5).mb_str()))
          {
              it->GetDevice()->SetType(new_device_type);
              it->GetDevice()->SetId(string(AxisTabDeviceId->GetLabel().mb_str()));
              it->GetDevice()->SetName(string(AxisTabDeviceName->GetLabel().mb_str()));
              it->GetEvent()->SetType(new_event_type);
              it->GetEvent()->SetId(string(AxisTabEventId->GetLabel().mb_str()));
              it->SetAxis(string(AxisTabAxisId->GetStringSelection().mb_str()));
              it->SetLabel(string(AxisTabLabel->GetValue().mb_str()));
          }
      }
    }
}

/*
 * \brief Method called on Panel_Axis>Shape selection. \
 *        It selects shape "Circle" in case event type is axis and selection is empty.
 */
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

/*
 * \brief Method called on Edit>Copy_Profile click. \
 *        It copies the current profile into a temporary profile.
 */
void configFrame::OnMenuItemCopyConfiguration(wxCommandEvent& event)
{
  save_current();
  tempConfiguration = *configFile.GetController(currentController)->GetConfiguration(currentConfiguration);
}

/*
 * \brief Method called on Edit>Paste_Profile click. \
 *        It copies the temporary profile into the current profile.
 */
void configFrame::OnMenuItemPasteConfiguration(wxCommandEvent& event)
{
  configFile.GetController(currentController)->SetConfiguration(tempConfiguration, currentConfiguration);
  load_current();
  refresh_gui();
  ButtonTabModify->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  IntensityModify->SetLabel(_("Modify"));
  Button3->Enable();
  ButtonTabAdd->Enable();
  ButtonTabRemove->Enable();
  Button7->Enable();
}

/*
 * \brief Method called on Edit>Copy_Controller click. \
 *        It copies the current controller into a temporary controller.
 */
void configFrame::OnMenuItemCopyController(wxCommandEvent& event)
{
  save_current();
  tempController = *configFile.GetController(currentController);
}

/*
 * \brief Method called on Edit>Paste_Controller click. \
 *        It copies the temporary controller into the current controller.
 */
void configFrame::OnMenuItemPasteController(wxCommandEvent& event)
{
  configFile.SetController(tempController, currentController);
  load_current();
  refresh_gui();
  ButtonTabModify->SetLabel(_("Modify"));
  Button5->SetLabel(_("Modify"));
  IntensityModify->SetLabel(_("Modify"));
  Button3->Enable();
  ButtonTabAdd->Enable();
  ButtonTabRemove->Enable();
  Button7->Enable();
}

/*
 * \brief Method called on Overall_Panel>Delete_trigger click.
 */
void configFrame::OnButtonDeleteTrigger(wxCommandEvent& event)
{
    ProfileTriggerDeviceType->SetLabel(wxEmptyString);
    ProfileTriggerDeviceName->SetLabel(wxEmptyString);
    ProfileTriggerDeviceId->SetLabel(wxEmptyString);
    ProfileTriggerButtonId->SetLabel(wxEmptyString);
    CheckBoxSwitchBack->SetValue(false);

    refresh_gui();
}

/*
 * \brief Replace all device names for a given device type, in the current controller.
 *
 * \param wx_device_type  the device type (untranslated)
 */
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
        wxString msg;
        if(wx_device_type == wxT("mouse"))
        {
          msg = _("Press a mouse button.");
        }
        else
        {
          msg = _("Press a keyboard key.");
        }
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
          if(it->GetDevice()->GetType() == device_type)
          {
              it->GetDevice()->SetId(device_id);
              it->GetDevice()->SetName(device_name);
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

/*
 * \brief Method called on Edit>Replace_Mouse click. \
 *        It replaces all mouse names in the current controller. \
 *        It asks the user for a mouse click so as to get the new mouse name. \
 *        Only available if MenuItemMultipleMiceAndKeyboards is checked.
 */
void configFrame::OnMenuReplaceMouse(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the mouse in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(wxT("mouse"));
}

/*
 * \brief Method called on Edit>Replace_Mouse click. \
 *        It replaces all keyboard names in the current controller. \
 *        It asks the user for a key press so as to get the new keyboard name. \
 *        Only available if MenuItemMultipleMiceAndKeyboards is checked.
 */
void configFrame::OnMenuReplaceKeyboard(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("This will replace the keyboard in the current controller.\nContinue?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    replaceDevice(wxT("keyboard"));
}

/*
 * \brief Method called on Edit>Replace_Mouse_DPI click. \
 *        It converts the sensitivity in the current controller. \
 *        It asks the user for a source and a destination DPI.
 */
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
      wxNumberEntryDialog dialog1(this, wxT(""), _("Enter a number:"), _("Source mouse DPI"), 2000, 100, MAX_DPI);

      if(dialog1.ShowModal() == wxID_OK)
      {
          old_value = dialog1.GetValue();
      }
    }


    if (old_value)
    {
        wxNumberEntryDialog dialog2(this, wxT(""), _("Enter a number:"), _("Destination mouse DPI"), old_value, 100, MAX_DPI);

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

/*
 * \brief Method called on Overall_Panel>Auto_detect_Intensity click.
 */
void configFrame::OnIntensityAutoDetectClick(wxCommandEvent& event)
{
  IntensityAutoDetect->Enable(false);

  auto_detect(IntensityDeviceType, IntensityDeviceName, IntensityDeviceId, _("button"), IntensityButtonId);

  IntensityAutoDetect->Enable(true);

  refresh_gui();
}

/*
 * \brief Method called on Overall_Panel>IntensityDeadZone selection.
 */
void configFrame::OnIntensityDeadZoneChange(wxSpinEvent& event)
{
    int max = 127-IntensityDeadZone->GetValue();
    if(max)
    {
        IntensitySteps->SetValue(IntensitySteps->GetValue()*IntensitySteps->GetMax()/max);
        IntensitySteps->SetRange(1, max);
    }
    refresh_gui();
}

/*
 * \brief Method called on Edit>Set_Mouse_DPI. \
 *        It sets the mouse DPI for the current controller.
 */
void configFrame::OnMenuSetMouseDPI(wxCommandEvent& event)
{
    wxNumberEntryDialog dialog1(this, wxT(""), _("Enter a number:"), _("Mouse DPI value"), configFile.GetController(currentController)->GetMouseDPI(), 0, MAX_DPI);
    if (dialog1.ShowModal() == wxID_OK)
    {
       configFile.GetController(currentController)->SetMouseDPI(dialog1.GetValue());
    }
}

/*
 * \brief Method called on a wxTextCtrl edition. \
 *        Only applicable for AxisTabDeadZone, AxisTabSensitivity, \
 *        AxisTabAcceleration, AxisTabBufferSize or AxisTabFilter. \
 *        It sets the mouse DPI for the current controller.
 */
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

    if(str.Replace(wxT(","), wxT(".")))
    {
        text->SetValue(str);
    }

    if(text == AxisTabDeadZone)
    {
        if(!str.ToLong(&lvalue))
        {
            text->SetValue(wxT("20"));
        }
        else if(lvalue < 0)
        {
            text->SetValue(wxT("0"));
        }
        else if(lvalue > 64)
        {
            text->SetValue(wxT("64"));
        }
    }
    else if(text == AxisTabSensitivity)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(wxT("1.00"));
        }
    }
    else if(text == AxisTabAcceleration)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(wxT("1.00"));
        }
        else if(value < 0)
        {
            text->SetValue(wxT("0.00"));
        }
        else if(value > 2)
        {
            text->SetValue(wxT("2.00"));
        }
    }
    else if(text == AxisTabBufferSize)
    {
        if(!str.ToLong(&lvalue))
        {
            text->SetValue(wxT("1"));
        }
        else if(lvalue < 1)
        {
            text->SetValue(wxT("1"));
        }
        else if(lvalue > 30)
        {
            text->SetValue(wxT("30"));
        }
    }
    else if(text == AxisTabFilter)
    {
        if(!str.ToDouble(&value))
        {
            text->SetValue(wxT("0.00"));
        }
        else if(value < 0)
        {
            text->SetValue(wxT("0.00"));
        }
        else if(value > 1)
        {
            text->SetValue(wxT("1.00"));
        }
    }
}

/*
 * \brief Check if a control (device+event) is already used.
 *
 * \param device_type  the device type (translated)
 * \param device_name  the device name
 * \param device_id    the device id
 * \param event_type   the event type (translated)
 * \param event_id     the event id
 *
 * \param gridIndex1  an index in GridPanelButton, or -1
 * \param gridIndex2  an index in GridPanelAxis, or -1
 * \param gridIndex3  an index in GridIntensity, or -1
 *
 * \return if the control is already used, a message for the user, an empty string otherwise.
 */
wxString configFrame::isAlreadyUsed(wxString device_type, wxString device_name, wxString device_id, wxString event_type, wxString event_id, int gridIndex1, int gridIndex2, int gridIndex3)
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
    for(int i=0; i<GridIntensity->GetNumberRows(); i++)
    {
      if(gridIndex3 >= 0 && i == gridIndex3)
      {
        continue;
      }

      if(GridIntensity->GetCellValue(i, 0) == device_type
         && (GridIntensity->GetCellValue(i, 1).IsEmpty() || device_name.IsEmpty() || GridIntensity->GetCellValue(i, 1) == device_name)
         && GridIntensity->GetCellValue(i, 2) == device_id
         && GridIntensity->GetCellValue(i, 3) == event_id)
      {
        return _("This control is already used in the current profile (Overall tab).\nContinue?");
      }
    }
  }

  return wxEmptyString;
}

/*
 * \brief Called on Advanced>Multiple_mice_and_keyboards edition. \
 *        If MenuItemMultipleMiceAndKeyboards is checked, \
 *        the user is asked to auto detect a mouse and a keyboard for the current controller. \
 *        If it is unchecked, all mouse and keyboard names are erased in the current controller.
 */
void configFrame::OnMenuMultipleMK(wxCommandEvent& event)
{
    IntensityDeviceType->SetLabel(wxEmptyString);
    IntensityDeviceName->SetLabel(wxEmptyString);
    IntensityDeviceId->SetLabel(wxEmptyString);
    IntensityButtonId->SetLabel(wxEmptyString);
    
    ButtonTabDeviceType->SetLabel(wxEmptyString);
    ButtonTabDeviceName->SetLabel(wxEmptyString);
    ButtonTabDeviceId->SetLabel(wxEmptyString);
    ButtonTabEventId->SetLabel(wxEmptyString);
    
    AxisTabDeviceType->SetLabel(wxEmptyString);
    AxisTabDeviceName->SetLabel(wxEmptyString);
    AxisTabDeviceId->SetLabel(wxEmptyString);
    AxisTabEventId->SetLabel(wxEmptyString);
    
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
    replaceDevice(wxT("mouse"));
    replaceDevice(wxT("keyboard"));
}

/*
 * \brief Method called on Help>Update click. \
 *        It checks if a software update is available. \
 *        If an update is available, it asks if it has to be downloaded and installed.
 */
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
      wxMessageBox(_("Can't retrieve update file!"), _("Error"), wxICON_ERROR);
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

/*
 * \brief Method called on Advanced>Auto_bind_controls. \
 *        It asks the user for a reference config, and auto binds the controls according to the control labels.
 */
void configFrame::OnMenuAutoBindControls(wxCommandEvent& event)
{
  if(configFile.GetFilePath().empty())
  {
    wxMessageBox( _("No config opened!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxFileDialog FileDialog(this, _("Select the reference config."), default_directory, wxEmptyString, _T("XML files (*.xml)|*.xml"), wxFD_DEFAULT_STYLE);

  if ( FileDialog.ShowModal() != wxID_OK ) return;

  wxString FileName = FileDialog.GetPath();
  if ( FileName.IsEmpty() ) return;

  if(configFile.AutoBind(string(FileName.mb_str())) < 0)
  {
    wxMessageBox(_("Can't auto-bind controls!"), _("Error"), wxICON_ERROR);
  }
  else
  {
    load_current();
    wxMessageBox(_("Done!"), _("Info"), wxICON_INFORMATION);
  }
}

/*
 * \brief Method called on Overall_Panel>Add_Intensity click. \
 *        It inserts an entry into GridIntensity.
 */
void configFrame::OnIntensityAddClick(wxCommandEvent& event)
{
  if(IntensityButtonId->GetLabel().IsEmpty())
  {
      wxMessageBox( _("Please detect a button!"), _("Error"), wxICON_ERROR);
      return;
  }
  if(IntensityAxis->GetStringSelection().IsEmpty())
  {
      wxMessageBox( _("Please select an axis!"), _("Error"), wxICON_ERROR);
      return;
  }

  wxString check = isAlreadyUsed(IntensityDeviceType->GetLabel(), IntensityDeviceName->GetLabel(), IntensityDeviceId->GetLabel(), _("button"), IntensityButtonId->GetLabel(), -1, -1, -1);

  if(!check.IsEmpty())
  {
    int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
      return;
    }
  }

  GridIntensity->InsertRows();
  GridIntensity->SetCellValue(0, 0, IntensityDeviceType->GetLabel());
  GridIntensity->SetCellValue(0, 1, IntensityDeviceName->GetLabel());
  GridIntensity->SetCellValue(0, 2, IntensityDeviceId->GetLabel());
  GridIntensity->SetCellValue(0, 3, IntensityButtonId->GetLabel());
  GridIntensity->SetCellValue(0, 4, IntensityDirection->GetStringSelection());
  wxString dz;
  dz << IntensityDeadZone->GetValue();
  GridIntensity->SetCellValue(0, 5, dz);
  GridIntensity->SetCellValue(0, 6, IntensityShape->GetStringSelection());
  wxString steps;
  steps << IntensitySteps->GetValue();
  GridIntensity->SetCellValue(0, 7, steps);
  GridIntensity->SetCellValue(0, 8, IntensityAxis->GetStringSelection());

  GridIntensity->AutoSizeColumns();
  refresh_gui();
}

/*
 * \brief Method called on Overall_Panel>Remove_Intensity click. \
 *        It removes an entry from GridIntensity.
 */
void configFrame::OnIntensityRemoveClick(wxCommandEvent& event)
{
  configFrame::DeleteSelectedRows(GridIntensity);
  refresh_gui();
}

/*
 * \brief Update all profiles of the current controller. \
 *        It updates intensity controls.
 *
 * \param oldI  the previous intensity control (device type untranslated)
 * \param newI  the new intensity control (device type untranslated)
 */
void configFrame::updateIntensityConfigurations(Intensity* oldI, Intensity* newI)
{
    int k;

    std::list<Intensity>* intensities;

    Controller* controller = configFile.GetController(currentController);

    for(k=0; k<MAX_CONFIGURATIONS; ++k)
    {
      Configuration* config = controller->GetConfiguration(k);

      intensities = config->GetIntensityList();
      for(std::list<Intensity>::iterator it = intensities->begin(); it!=intensities->end(); ++it)
      {
          if(it->GetDevice()->GetType() == oldI->GetDevice()->GetType()
              && it->GetDevice()->GetName() == oldI->GetDevice()->GetName()
              && it->GetDevice()->GetId() == oldI->GetDevice()->GetId()
              && it->GetEvent()->GetId() == oldI->GetEvent()->GetId()
              && it->GetControl() == oldI->GetControl())
          {
              *it = *newI;
          }
      }
    }
}

/*
 * \brief Method called on Overall_Panel>Modify click. \
 *        It modifies the edited intensity control. \
 *        It updates the other profiles if MenuItemLinkControls is checked.
 */
void configFrame::OnIntensityModifyClick(wxCommandEvent& event)
{
  wxArrayInt array = GetGridSelectedRows(GridIntensity);
  int count = array.GetCount();

  if(IntensityModify->GetLabel() == _("Modify"))
  {
      if(count == 0)
      {
          wxMessageBox( _("Please select a line of the table."), _("Info"), wxICON_INFORMATION);
          return;
      }
      else if(count > 1)
      {
          wxMessageBox( _("Please select a SINGLE line of the table."), _("Info"), wxICON_INFORMATION);
          return;
      }
      
      grid3mod = array.Item(0);
      
      IntensityAxis->SetSelection(IntensityAxis->FindString(GridIntensity->GetCellValue(grid3mod, 8)));
      OnIntensityAxisSelect(event);
      
      IntensityDeviceType->SetLabel(GridIntensity->GetCellValue(grid3mod, 0));
      IntensityDeviceName->SetLabel(GridIntensity->GetCellValue(grid3mod, 1));
      IntensityDeviceId->SetLabel(GridIntensity->GetCellValue(grid3mod, 2));
      IntensityButtonId->SetLabel(GridIntensity->GetCellValue(grid3mod, 3));
      IntensityDirection->SetSelection(IntensityDirection->FindString(GridIntensity->GetCellValue(grid3mod, 4)));
      IntensityDeadZone->SetValue(wxAtoi(GridIntensity->GetCellValue(grid3mod, 5)));
      IntensityShape->SetSelection(IntensityShape->FindString(GridIntensity->GetCellValue(grid3mod, 6)));
      IntensitySteps->SetValue(wxAtoi(GridIntensity->GetCellValue(grid3mod, 7)));
      IntensityAdd->Disable();
      IntensityRemove->Disable();
      IntensityModify->SetLabel(_("Apply"));
  }
  else
  {
      if (IntensityButtonId->GetLabel().IsEmpty())
      {
        wxMessageBox(_("Please detect an Event!"), _("Error"), wxICON_ERROR);
        return;
      }
      if(IntensityAxis->GetStringSelection().IsEmpty())
      {
          wxMessageBox( _("Please select a Button!"), _("Error"), wxICON_ERROR);
          return;
      }

      wxString check = isAlreadyUsed(IntensityDeviceType->GetLabel(), IntensityDeviceName->GetLabel(), IntensityDeviceId->GetLabel(), _("button"), IntensityButtonId->GetLabel(), -1, -1, grid3mod);

      if(!check.IsEmpty())
      {
        int answer = wxMessageBox(check, _("Confirm"), wxYES_NO);
        if (answer == wxNO)
        {
          return;
        }
      }

      Intensity oldI(string(GridIntensity->GetCellValue(grid3mod, 8).mb_str()), reverse_gettext(string(GridIntensity->GetCellValue(grid3mod, 0).mb_str())),
          string(GridIntensity->GetCellValue(grid3mod, 2).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 1).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 3).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 4).mb_str()),
          wxAtoi(GridIntensity->GetCellValue(grid3mod, 5)), wxAtoi(GridIntensity->GetCellValue(grid3mod, 7)), reverse_gettext(string(GridIntensity->GetCellValue(grid3mod, 6).mb_str())));

      GridIntensity->SetCellValue(grid3mod, 0, IntensityDeviceType->GetLabel());
      GridIntensity->SetCellValue(grid3mod, 1, IntensityDeviceName->GetLabel());
      GridIntensity->SetCellValue(grid3mod, 2, IntensityDeviceId->GetLabel());
      GridIntensity->SetCellValue(grid3mod, 3, IntensityButtonId->GetLabel());
      GridIntensity->SetCellValue(grid3mod, 4, IntensityDirection->GetStringSelection());
      wxString dz;
      dz << IntensityDeadZone->GetValue();
      GridIntensity->SetCellValue(grid3mod, 5, dz);
      GridIntensity->SetCellValue(grid3mod, 6, IntensityShape->GetStringSelection());
      wxString steps;
      steps << IntensitySteps->GetValue();
      GridIntensity->SetCellValue(grid3mod, 7, steps);
      GridIntensity->SetCellValue(grid3mod, 8, IntensityAxis->GetStringSelection());

      Intensity newI(string(GridIntensity->GetCellValue(grid3mod, 8).mb_str()), reverse_gettext(string(GridIntensity->GetCellValue(grid3mod, 0).mb_str())),
          string(GridIntensity->GetCellValue(grid3mod, 2).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 1).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 3).mb_str()), string(GridIntensity->GetCellValue(grid3mod, 4).mb_str()),
          wxAtoi(GridIntensity->GetCellValue(grid3mod, 5)), wxAtoi(GridIntensity->GetCellValue(grid3mod, 7)), reverse_gettext(string(GridIntensity->GetCellValue(grid3mod, 6).mb_str())));

      if(MenuItemLinkControls->IsChecked())
      {
        updateIntensityConfigurations(&oldI, &newI);
      }

      IntensityAdd->Enable();
      IntensityRemove->Enable();
      IntensityModify->SetLabel(_("Modify"));
  }
  GridIntensity->AutoSizeColumns();
  refresh_gui();
}

/*
 * \brief Method called on Overall_Panel>Axis selection. \
 *        It selects the intensity shape according to the selection.
 */
void configFrame::OnIntensityAxisSelect(wxCommandEvent& event)
{
  if(IntensityAxis->GetStringSelection() == wxT("lstick") || IntensityAxis->GetStringSelection() == wxT("rstick"))
  {
    if(IntensityShape->IsEmpty())
    {
      IntensityShape->Clear();
      IntensityShape->SetSelection( IntensityShape->Append(_("Circle")) );
      IntensityShape->Append(_("Rectangle"));
    }
  }
  else
  {
    IntensityShape->Clear();
  }
}
