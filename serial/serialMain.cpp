/***************************************************************
 * Name:      serialMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "serialMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(serialFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <pwd.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include "serial.h"

#include "../directories.h"
#include "../shared/updater/updater.h"
#include "../shared/configupdater/configupdater.h"

using namespace std;

#ifdef WIN32
#define MAX_PORT_ID 32
#else
#define OPT_DIR "/.sixemugui-serial/"
char* homedir;
#endif

//helper functions
enum wxbuildinfoformat
{
    short_f, long_f
};

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

//(*IdInit(serialFrame)
const long serialFrame::ID_STATICTEXT3 = wxNewId();
const long serialFrame::ID_COMBOBOX1 = wxNewId();
const long serialFrame::ID_STATICTEXT4 = wxNewId();
const long serialFrame::ID_CHOICE1 = wxNewId();
const long serialFrame::ID_BUTTON2 = wxNewId();
const long serialFrame::ID_STATICTEXT1 = wxNewId();
const long serialFrame::ID_COMBOBOX2 = wxNewId();
const long serialFrame::ID_STATICTEXT2 = wxNewId();
const long serialFrame::ID_CHECKBOX5 = wxNewId();
const long serialFrame::ID_CHECKBOX6 = wxNewId();
const long serialFrame::ID_CHECKBOX1 = wxNewId();
const long serialFrame::ID_CHECKBOX4 = wxNewId();
const long serialFrame::ID_CHECKBOX2 = wxNewId();
const long serialFrame::ID_CHECKBOX3 = wxNewId();
const long serialFrame::ID_CHOICE4 = wxNewId();
const long serialFrame::ID_BUTTON1 = wxNewId();
const long serialFrame::ID_BUTTON3 = wxNewId();
const long serialFrame::ID_PANEL1 = wxNewId();
const long serialFrame::ID_MENUITEM1 = wxNewId();
const long serialFrame::ID_MENUITEM2 = wxNewId();
const long serialFrame::ID_MENUITEM3 = wxNewId();
const long serialFrame::idMenuQuit = wxNewId();
const long serialFrame::ID_MENUITEM6 = wxNewId();
const long serialFrame::ID_MENUITEM4 = wxNewId();
const long serialFrame::ID_MENUITEM5 = wxNewId();
const long serialFrame::idMenuAbout = wxNewId();
const long serialFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(serialFrame,wxFrame)
    //(*EventTable(serialFrame)
    //*)
END_EVENT_TABLE()

#ifdef WIN32
static void read_devices(wxComboBox* choice)
{
  HANDLE hSerial;
  DWORD accessdirection = /*GENERIC_READ |*/GENERIC_WRITE;
  char portname[16];
  wchar_t szCOM[16];
  int i;

  choice->Clear();

  for(i=0; i<MAX_PORT_ID; ++i)
  {
    wsprintf(szCOM, L"\\\\.\\COM%d", i);
    hSerial = CreateFile(szCOM, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
    if (hSerial != INVALID_HANDLE_VALUE)
    {
      snprintf(portname, sizeof(portname), "COM%d", i);
      choice->SetSelection(choice->Append(wxString(portname, wxConvUTF8)));
    }
    CloseHandle(hSerial);
  }
}
#else
static void read_devices(wxComboBox* choice)
{
  string filename = "";
  string line = "";

  filename.append(homedir);
  filename.append(OPT_DIR);
  filename.append("config");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
      if( infile.good() )
      {
          getline (infile,line);
      }
      infile.close();
  }

  choice->Clear();

  string ds = "/dev";

  wxDir dir(wxString(ds.c_str(), wxConvUTF8));

  if(!dir.IsOpened())
  {
    cout << "Warning: can't open " << ds << endl;
    return;
  }

  wxString file;
  wxString filespec = wxT("*");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    if(file.StartsWith(_("ttyUSB")) || file.StartsWith(_("ttyACM")))
    {
      if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
      {
        choice->SetSelection(choice->Append(file));
      }
      else
      {
        choice->Append(file);
      }
    }
  }
}
#endif

static void read_filenames(wxChoice* choice)
{
  string filename = "";
  string line = "";

  /* Read the last config used so as to auto-select it. */
#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("default");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
    }
    infile.close();
  }

  choice->Clear();

  /* Read all config file names. */
  string ds;
#ifndef WIN32
  ds.append(homedir);
  ds.append(APP_DIR);
#endif
  ds.append(CONFIG_DIR);

  wxDir dir(wxString(ds.c_str(), wxConvUTF8));

  if(!dir.IsOpened())
  {
    cout << "Warning: can't open " << ds << endl;
    return;
  }

  wxString file;
  wxString filespec = wxT("*.xml");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
    {
      choice->SetSelection(choice->Append(file));
    }
    else
    {
      choice->Append(file);
    }
  }
}

static void read_frequency(wxComboBox* choice)
{
  string filename = "";
  string line = "";

#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("frequency");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
      choice->SetSelection(choice->FindString(wxString(line.c_str(), wxConvUTF8)));
    }
    infile.close();
  }
}

static void read_controller_type(wxChoice* choice)
{
  string filename = "";
  string line = "";

#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("controller");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
      choice->SetSelection(choice->FindString(wxString(line.c_str(), wxConvUTF8)));
    }
    infile.close();
  }
}

static void readStartUpdates(wxMenuItem* menuItem)
{
  string filename = "";
  string line = "";

#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("startUpdates");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
      if(line == "yes")
      {
        menuItem->Check(true);
      }
    }
    infile.close();
  }
}

serialFrame::serialFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(serialFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItem2;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer8;
    wxStaticBoxSizer* StaticBoxSizer6;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer12;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxMenu* Menu2;
    wxStaticBoxSizer* StaticBoxSizer5;
    
    Create(parent, wxID_ANY, _("Gimx-serial"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(412,470));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxSize(0,0), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("USB to serial"));
    FlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("Device"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ComboBoxDevice = new wxComboBox(Panel1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
    FlexGridSizer3->Add(ComboBoxDevice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("emuclient"));
    FlexGridSizer5 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer12 = new wxFlexGridSizer(1, 3, 0, 0);
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Controller"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer12->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ControllerType = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxSize(100,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    ControllerType->SetSelection( ControllerType->Append(_("Joystick")) );
    ControllerType->Append(_("GPP"));
    ControllerType->Append(_("PS2 pad"));
    FlexGridSizer12->Add(ControllerType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSpoof = new wxButton(Panel1, ID_BUTTON2, _("Spoof"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    ButtonSpoof->Disable();
    FlexGridSizer12->Add(ButtonSpoof, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(1, 3, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Update frequency "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ComboBoxFrequency = new wxComboBox(Panel1, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(75,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX2"));
    ComboBoxFrequency->Append(_("62.5"));
    ComboBoxFrequency->Append(_("100"));
    ComboBoxFrequency->Append(_("125"));
    ComboBoxFrequency->SetSelection( ComboBoxFrequency->Append(_("250")) );
    FlexGridSizer6->Add(ComboBoxFrequency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Hz"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13 = new wxFlexGridSizer(1, 2, 0, 0);
    CheckBoxForceUpdates = new wxCheckBox(Panel1, ID_CHECKBOX5, _("Force updates"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBoxForceUpdates->SetValue(true);
    FlexGridSizer13->Add(CheckBoxForceUpdates, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxSubpositions = new wxCheckBox(Panel1, ID_CHECKBOX6, _("Subposition"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    CheckBoxSubpositions->SetValue(true);
    FlexGridSizer13->Add(CheckBoxSubpositions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("Mouse"));
    FlexGridSizer10 = new wxFlexGridSizer(1, 2, 0, 0);
    CheckBoxGrab = new wxCheckBox(Panel1, ID_CHECKBOX1, _("grab"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxGrab->SetValue(true);
    FlexGridSizer10->Add(CheckBoxGrab, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxCalibrate = new wxCheckBox(Panel1, ID_CHECKBOX4, _("calibrate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBoxCalibrate->SetValue(false);
    FlexGridSizer10->Add(CheckBoxCalibrate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Output"));
    FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBoxGui = new wxCheckBox(Panel1, ID_CHECKBOX2, _("gui"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBoxGui->SetValue(false);
    FlexGridSizer11->Add(CheckBoxGui, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxTerminal = new wxCheckBox(Panel1, ID_CHECKBOX3, _("terminal"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBoxTerminal->SetValue(false);
    FlexGridSizer11->Add(CheckBoxTerminal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Config"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    ChoiceConfig = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxSize(225,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    FlexGridSizer4->Add(ChoiceConfig, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(StaticBoxSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(2, 1, 0, 0);
    ButtonCheck = new wxButton(Panel1, ID_BUTTON1, _("Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer7->Add(ButtonCheck, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStart = new wxButton(Panel1, ID_BUTTON3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(ButtonStart, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuEditConfig = new wxMenuItem(Menu1, ID_MENUITEM1, _("Edit config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuEditConfig);
    MenuEditFpsConfig = new wxMenuItem(Menu1, ID_MENUITEM2, _("Edit fps config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuEditFpsConfig);
    MenuRefresh = new wxMenuItem(Menu1, ID_MENUITEM3, _("Refresh\tF5"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuRefresh);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuGetConfigs = new wxMenuItem(Menu2, ID_MENUITEM6, _("Get configs"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuGetConfigs);
    MenuUpdate = new wxMenuItem(Menu2, ID_MENUITEM4, _("Update"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuUpdate);
    MenuStartupUpdates = new wxMenuItem(Menu2, ID_MENUITEM5, _("Check updates at startup"), wxEmptyString, wxITEM_CHECK);
    Menu2->Append(MenuStartupUpdates);
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -1, 20 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    SingleInstanceChecker1.Create(_T("gimx-serial_") + wxGetUserId() + _T("_Guard"));
    
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&serialFrame::OnControllerTypeSelect);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&serialFrame::OnButtonSpoofClick);
    Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&serialFrame::OnCheckBoxCalibrate);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&serialFrame::OnCheckBoxGuiClick);
    Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&serialFrame::OnCheckBoxTerminalClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&serialFrame::OnButtonCheckClick1);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&serialFrame::OnButtonStartClick);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuEditConfig);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuEditFpsConfig);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuRefresh);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnQuit);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuGetConfigs);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuUpdate);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnMenuStartupUpdates);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&serialFrame::OnAbout);
    //*)

#ifndef WIN32
    ControllerType->Append(_("360 pad"));
    /*ControllerType->Append(_("Sixaxis"));*/
#endif

    spoofed = false;

    if(SingleInstanceChecker1.IsAnotherRunning())
    {
        wxMessageBox( wxT("gimx-serial is already running!"), wxT("Error"), wxICON_ERROR);
        exit(-1);
    }

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

#ifndef WIN32
    homedir = getpwuid(getuid())->pw_dir;

	  /* Init user's config directory. */
    if(system("mkdir -p ~/.sixemugui-serial"))
    {
        wxMessageBox( wxT("Can't init ~/.sixemugui-serial directory!"), wxT("Error"), wxICON_ERROR);
    }
    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( wxT("Can't init ~/.emuclient/config!"), wxT("Error"), wxICON_ERROR);
    }
    if(system("mkdir -p ~/.emuclient/macros"))
    {
        wxMessageBox( wxT("Can't init ~/.emuclient/macros!"), wxT("Error"), wxICON_ERROR);
    }
#endif

    started = false;

    read_filenames(ChoiceConfig);
    read_frequency(ComboBoxFrequency);
    read_devices(ComboBoxDevice);
    read_controller_type(ControllerType);
    wxCommandEvent event;
    OnControllerTypeSelect(event);

    readStartUpdates(MenuStartupUpdates);
    if(MenuStartupUpdates->IsChecked())
    {
      OnMenuUpdate(event);
    }

    started = true;

    Refresh();
	
    if(ChoiceConfig->IsEmpty())
    {
      int answer = wxMessageBox(_("No config found! Download configs?"), _("Confirm"), wxYES_NO);
      if (answer == wxYES)
      {
        wxCommandEvent event;
        OnMenuGetConfigs(event);
      }
    }
}

serialFrame::~serialFrame()
{
    //(*Destroy(serialFrame)
    //*)
}

void serialFrame::OnQuit(wxCommandEvent& event)
{
  Close();
}

void serialFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(_(INFO_DESCR)) + wxString(_("\n")) + wxString(_(INFO_YEAR)) + wxString(_(" ")) + wxString(_(INFO_DEV)) + wxString(_(" ")) + wxString(_(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void serialFrame::OnButtonStartClick(wxCommandEvent& event)
{
    string command = "";
    string filename = "";
    wxString dpi;
    int refresh;
    ostringstream ios;
    double frequency;
    wxString wxfrequency;
    wxString configname;

    if(ChoiceConfig->GetStringSelection().IsEmpty())
    {
      wxMessageBox( wxT("No config selected!"), wxT("Error"), wxICON_ERROR);
      return;
    }

    if(ControllerType->GetStringSelection() != _("GPP"))
    {
      if(ComboBoxDevice->GetValue().IsEmpty())
      {
        wxMessageBox( wxT("No USB to serial device selected!"), wxT("Error"), wxICON_ERROR);
        return;
      }
    }

    if(ControllerType->GetStringSelection() == _("360 pad") && !spoofed)
    {
      wxMessageBox( wxT("Spoof the 360 controller first!"), wxT("Error"), wxICON_ERROR);
      return;
    }

#ifdef WIN32
    command.append("emuclient.exe");
#else
    if(CheckBoxTerminal->IsChecked())
    {
        command.append("gnome-terminal -e \"");
    }
    else
    {
        command.append("/bin/bash -c \"");
    }
    command.append(" emuclient");
#endif
    if(ControllerType->GetStringSelection() == _("Joystick"))
    {
      command.append(" --joystick");
    }
    else if(ControllerType->GetStringSelection() == _("GPP"))
    {
      command.append(" --GPP");
    }
    else if(ControllerType->GetStringSelection() == _("360 pad"))
    {
      command.append(" --360pad");
    }
    else if(ControllerType->GetStringSelection() == _("Sixaxis"))
    {
      command.append(" --Sixaxis");
    }
    else if(ControllerType->GetStringSelection() == _("PS2 pad"))
    {
      command.append(" --PS2pad");
    }
    if(ControllerType->GetStringSelection() == _("Joystick"))
    {
      command.append(" --precision 16");
    }
    else
    {
      command.append(" --precision 8");
    }
    if(!CheckBoxGrab->IsChecked())
    {
        command.append(" --nograb");
    }
    command.append(" --config ");
    configname = ChoiceConfig->GetStringSelection();
    configname.Replace(_(" "), _("\\ "));
    command.append(configname.mb_str());
    command.append(" --refresh ");
    wxfrequency = ComboBoxFrequency->GetValue();
    if(wxfrequency.ToDouble(&frequency))
    {
      refresh = 1000 / frequency;
    }
    else
    {
      refresh = 10;
    }
    ios << refresh;
    command.append(ios.str());
    if(CheckBoxForceUpdates->IsChecked())
    {
        command.append(" --force-updates");
    }
    if(CheckBoxSubpositions->IsChecked())
    {
        command.append(" --subpos");
    }
    if(ControllerType->GetStringSelection() != _("GPP"))
    {
      command.append(" --serial");
      command.append(" --port ");
#ifndef WIN32
      command.append("/dev/");
#endif
      command.append(ComboBoxDevice->GetValue().mb_str());
    }
    if(CheckBoxGui->IsChecked())
    {
        command.append(" --status | gimx-status");
    }
    else if(CheckBoxTerminal->IsChecked())
    {
        command.append(" --status");
    }
#ifndef WIN32
    command.append("\"");
#endif

    //cout << command << endl;

    ButtonStart->Disable();
#ifndef WIN32
    filename.append(homedir);
    filename.append(OPT_DIR);
#endif
    filename.append("default");
    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        outfile << ChoiceConfig->GetStringSelection().mb_str() << endl;
        outfile.close();
    }
    filename.erase();
#ifndef WIN32
    filename.append(homedir);
    filename.append(OPT_DIR);
#endif
    filename.append("frequency");
    ofstream outfile2 (filename.c_str(), ios_base::trunc);
    if(outfile2.is_open())
    {
        outfile2 << ComboBoxFrequency->GetValue().mb_str() << endl;
        outfile2.close();
    }
    filename.erase();
#ifndef WIN32
    filename.append(homedir);
    filename.append(OPT_DIR);
#endif
    filename.append("config");
    ofstream outfile3 (filename.c_str(), ios_base::trunc);
    if(outfile3.is_open())
    {
        outfile3 << ComboBoxDevice->GetValue().mb_str() << endl;
        outfile3.close();
    }
    filename.erase();
#ifndef WIN32
    filename.append(homedir);
    filename.append(OPT_DIR);
#endif
    filename.append("controller");
    ofstream outfile4 (filename.c_str(), ios_base::trunc);
    if(outfile4.is_open())
    {
        outfile4 << ControllerType->GetStringSelection().mb_str() << endl;
        outfile4.close();
    }

    if(system(command.c_str()) != 0)
    {
        wxMessageBox( wxT("Connection error!\nPlease check the USB to serial device!"), wxT("Error"), wxICON_ERROR);
    }

    ButtonStart->Enable();
}

void serialFrame::OnCheckBoxGuiClick(wxCommandEvent& event)
{
    CheckBoxTerminal->SetValue(false);
}

void serialFrame::OnCheckBoxTerminalClick(wxCommandEvent& event)
{
    CheckBoxGui->SetValue(false);
}

void serialFrame::OnCheckBoxCalibrate(wxCommandEvent& event)
{
    if(CheckBoxCalibrate->IsChecked())
    {
        CheckBoxGui->SetValue(true);
        CheckBoxTerminal->SetValue(false);
    }
    else
    {
        CheckBoxGui->SetValue(false);
    }
}

#ifdef WIN32
#define CHECK_FILE "check_result"
#else
#define CHECK_FILE "/tmp/check_result"
#endif

void serialFrame::OnButtonCheckClick1(wxCommandEvent& event)
{
    string command = "";
    string filename = "";
    string result = "";
    string line = "";
    wxString configname;

    if(ChoiceConfig->GetStringSelection().IsEmpty())
    {
      wxMessageBox( wxT("No config selected!"), wxT("Error"), wxICON_ERROR);
      return;
    }

#ifdef WIN32
    command.append("emuclient.exe");
#else
    command.append("emuclient");
#endif
    command.append(" --config ");
    configname = ChoiceConfig->GetStringSelection();
    configname.Replace(_(" "), _("\\ "));
    command.append(configname.mb_str());
    command.append(" --check --nograb > ");
    command.append(CHECK_FILE);

    if(system(command.c_str()) != 0)
    {
        wxMessageBox( wxT("Can't check the config file!"), wxT("Error"), wxICON_ERROR);
    }
    else
    {
        ifstream infile (CHECK_FILE);
        if ( infile.is_open() )
        {
            if( infile.good() )
            {
                while(getline (infile,line))
                {
                    if(result.find(line) == string::npos)
                    {
                        result.append(line);
                        result.append("\n");
                    }
                }
                if(result.empty())
                {
                    result.append("This config seems OK!\n");
                }
                wxMessageBox( wxString(result.c_str(), wxConvUTF8), wxT("Info"), wxICON_INFORMATION);
            }
            infile.close();
        }
    }
    remove(CHECK_FILE);
}

void serialFrame::OnMenuEditConfig(wxCommandEvent& event)
{
  string command = "";
#ifdef WIN32
  command.append("gimx-config.exe -f ");
  command.append(ChoiceConfig->GetStringSelection().mb_str());

  STARTUPINFOA startupInfo =
  { 0};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInformation;

  char* cmd = strdup(command.c_str());

  BOOL result = CreateProcessA(
      "gimx-config.exe",
      cmd,
      NULL,
      NULL,
      FALSE,
      NORMAL_PRIORITY_CLASS,
      NULL,
      NULL,
      &startupInfo,
      &processInformation
  );

  free(cmd);

  if(result == 0)
  {
    wxMessageBox( wxT("Error editing the config file!"), wxT("Error"), wxICON_ERROR);
  }
#else
  command.append("gimx-config");
  command.append(" -f ");
  command.append(ChoiceConfig->GetStringSelection().mb_str());
  command.append(" &");

  if (system(command.c_str()) != 0)
  {
    wxMessageBox(wxT("Error editing the config file!"), wxT("Error"),
        wxICON_ERROR);
  }
#endif
}

void serialFrame::OnMenuEditFpsConfig(wxCommandEvent& event)
{
  string command = "";
#ifdef WIN32
  command.append("gimx-fpsconfig.exe -f ");
  command.append(ChoiceConfig->GetStringSelection().mb_str());

  STARTUPINFOA startupInfo =
  { 0};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInformation;

  char* cmd = strdup(command.c_str());

  BOOL result = CreateProcessA(
      "gimx-fpsconfig.exe",
      cmd,
      NULL,
      NULL,
      FALSE,
      NORMAL_PRIORITY_CLASS,
      NULL,
      NULL,
      &startupInfo,
      &processInformation
  );

  free(cmd);

  if(result == 0)
  {
    wxMessageBox( wxT("Error editing the config file!"), wxT("Error"), wxICON_ERROR);
  }
#else
  command.append("gimx-fpsconfig");
  command.append(" -f ");
  command.append(ChoiceConfig->GetStringSelection().mb_str());
  command.append(" &");

  if (system(command.c_str()) != 0)
  {
    wxMessageBox(wxT("Error editing the config file!"), wxT("Error"),
        wxICON_ERROR);
  }
#endif
}

void serialFrame::OnMenuRefresh(wxCommandEvent& event)
{
    wxString previous = ChoiceConfig->GetStringSelection();
    ChoiceConfig->Clear();
    read_filenames(ChoiceConfig);
    ChoiceConfig->SetSelection(ChoiceConfig->FindString(previous));
    previous = ComboBoxDevice->GetStringSelection();
    ComboBoxDevice->Clear();
    read_devices(ComboBoxDevice);
    ComboBoxDevice->SetSelection(ComboBoxDevice->FindString(previous));
}

void serialFrame::OnControllerTypeSelect(wxCommandEvent& event)
{
    if(!spoofed && (ControllerType->GetStringSelection() == _("360 pad")))
    {
      ButtonSpoof->Enable(true);
    }
    else
    {
      ButtonSpoof->Enable(false);
    }
    if(ControllerType->GetStringSelection() == _("GPP"))
    {
      ComboBoxDevice->Enable(false);
    }
    else
    {
      ComboBoxDevice->Enable(true);
    }
    if(ControllerType->GetStringSelection() == _("PS2 pad"))
    {
      ComboBoxFrequency->SetSelection(0);
      ComboBoxFrequency->Enable(false);
    }
    else
    {
      if(started)
      {
        ComboBoxFrequency->SetSelection(3);
      }
      ComboBoxFrequency->Enable(true);
    }
}

void serialFrame::OnButtonSpoofClick(wxCommandEvent& event)
{
    string command = "";

    if(ComboBoxDevice->GetValue().IsEmpty())
    {
      wxMessageBox( wxT("No USB to serial device selected!"), wxT("Error"), wxICON_ERROR);
      return;
    }

    if(ControllerType->GetStringSelection() == _("360 pad") && !spoofed)
    {
      {
        wxMessageBox( _("1. Make sure a single genuine 360 controller is connected to the PC.\n"
            "2. Make sure the adapter is connected to the PC.\n"
            "3. Make sure the adapter is NOT connected to the 360.\n"
            "4. Press OK, and connect the adapter to the 360 after 2-3 seconds."), wxT("Info"), wxICON_INFORMATION);
      }
#ifdef WIN32
      command.append("usbspoof.exe -p ");
#else
      command.append("usbspoof -p /dev/");
#endif
      command.append(ComboBoxDevice->GetValue().mb_str());

      if(system(command.c_str()) != 0)
      {
        spoofed = false;
        wxMessageBox( wxT("Spoof error!\nPlease try again!"), wxT("Error"), wxICON_ERROR);
        return;
      }
      else
      {
        wxMessageBox( _("Spoof successful!"), wxT("Info"), wxICON_INFORMATION);
        spoofed = true;
        ButtonSpoof->Enable(false);
      }
    }
}

void serialFrame::OnMenuUpdate(wxCommandEvent& event)
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
    else
    {
      exit(0);
    }
  }
  else if (ret < 0)
  {
    wxMessageBox(wxT("Can't check version!"), wxT("Error"), wxICON_ERROR);
  }
  else if(started)
  {
    wxMessageBox(wxT("GIMX is up-to-date!"), wxT("Info"), wxICON_INFORMATION);
  }
}

void serialFrame::OnMenuStartupUpdates(wxCommandEvent& event)
{
  string filename;
#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("startUpdates");
  ofstream outfile (filename.c_str(), ios_base::trunc);
  if(outfile.is_open())
  {
    if(MenuStartupUpdates->IsChecked())
    {
      outfile << "yes" << endl;
    }
    else
    {
      outfile << "no" << endl;
    }
    outfile.close();
  }
}

void serialFrame::OnMenuGetConfigs(wxCommandEvent& event)
{
  configupdater u(CONFIGS_URL, CONFIGS_FILE, CONFIG_DIR);

  list<string>* cl = u.getconfiglist();
  list<string> cl_sel;

  if(cl && !cl->empty())
  {
    wxArrayString choices;

    for(list<string>::iterator it = cl->begin(); it != cl->end(); it++)
    {
      choices.Add(wxString(it->c_str(), wxConvUTF8));
    }

    wxMultiChoiceDialog dialog(this, wxT("Select the files to download."), wxT("Config download"), choices);

    if (dialog.ShowModal() == wxID_OK)
    {
      wxArrayInt selections = dialog.GetSelections();

      for ( size_t n = 0; n < selections.GetCount(); n++ )
      {
        string sel = string(choices[selections[n]].mb_str());
        wxString wxfile = _(CONFIG_DIR) + choices[selections[n]];
        if (::wxFileExists(wxfile))
        {
          int answer = wxMessageBox(_("Overwrite local file: ") + choices[selections[n]] + _("?"), _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            continue;
          }
        }
        cl_sel.push_back(sel);
      }

      if(u.getconfigs(&cl_sel) < 0)
      {
        wxMessageBox(wxT("Can't retrieve configs!"), wxT("Error"), wxICON_ERROR);
        return;
      }
      read_filenames(ChoiceConfig);
      if(!cl_sel.empty())
	    {
	      ChoiceConfig->SetSelection(ChoiceConfig->FindString(wxString(cl_sel.front().c_str(), wxConvUTF8)));
	      wxMessageBox(wxT("Download is complete!"), wxT("Info"), wxICON_INFORMATION);
      }
    }
  }
  else
  {
    wxMessageBox(wxT("Can't retrieve config list!"), wxT("Error"), wxICON_ERROR);
    return;
  }
}
