/***************************************************************
 * Name:      sixemuguiMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixemuguiMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(sixemuguiFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <pthread.h>
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
#include "serial.h"

using namespace std;

#ifdef WIN32
#define CONFIG_DIR "config/"
#define MAX_PORT_ID 32
#else
#define CONFIG_DIR ".emuclient/config/"
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

//(*IdInit(sixemuguiFrame)
const long sixemuguiFrame::ID_STATICTEXT3 = wxNewId();
const long sixemuguiFrame::ID_COMBOBOX1 = wxNewId();
const long sixemuguiFrame::ID_STATICTEXT1 = wxNewId();
const long sixemuguiFrame::ID_COMBOBOX2 = wxNewId();
const long sixemuguiFrame::ID_STATICTEXT2 = wxNewId();
const long sixemuguiFrame::ID_CHECKBOX5 = wxNewId();
const long sixemuguiFrame::ID_CHECKBOX1 = wxNewId();
const long sixemuguiFrame::ID_CHECKBOX4 = wxNewId();
const long sixemuguiFrame::ID_CHECKBOX2 = wxNewId();
const long sixemuguiFrame::ID_CHECKBOX3 = wxNewId();
const long sixemuguiFrame::ID_CHOICE4 = wxNewId();
const long sixemuguiFrame::ID_BUTTON1 = wxNewId();
const long sixemuguiFrame::ID_BUTTON3 = wxNewId();
const long sixemuguiFrame::ID_PANEL1 = wxNewId();
const long sixemuguiFrame::ID_MENUITEM1 = wxNewId();
const long sixemuguiFrame::ID_MENUITEM2 = wxNewId();
const long sixemuguiFrame::ID_MENUITEM3 = wxNewId();
const long sixemuguiFrame::idMenuQuit = wxNewId();
const long sixemuguiFrame::idMenuAbout = wxNewId();
const long sixemuguiFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(sixemuguiFrame,wxFrame)
    //(*EventTable(sixemuguiFrame)
    //*)
END_EVENT_TABLE()

#ifdef WIN32
static void read_filenames(wxChoice* choice)
{
    DIR *dirp;
    struct dirent *d;
    string filename = "";
    string line = "";
    struct stat buf;
    char path[PATH_MAX];

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

    dirp = opendir(CONFIG_DIR);
    if (dirp == NULL)
    {
      printf("Warning: can't open config directory\n");
      return;
    }

    while ((d = readdir(dirp)))
    {
      snprintf(path, sizeof(path), "%s/%s", CONFIG_DIR, d->d_name);
      if(stat(path, &buf) == 0)
      {
        if(S_ISREG(buf.st_mode))
        {
          if(!line.empty() && line == d->d_name)
          {
            choice->SetSelection(choice->Append(wxString(d->d_name, wxConvFile)));
          }
          else
          {
            choice->Append(wxString(d->d_name, wxConvFile));
          }
        }
      }
    }

    closedir(dirp);
}

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

static void read_frequency(wxComboBox* choice)
{
  string filename = "";
  string line = "";

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
#else
static void read_devices(wxComboBox* choice)
{
  DIR *dirp;
  char dir_path[PATH_MAX];
  struct dirent *d;
  string filename = "";
  string line = "";

  filename.append(homedir);
  filename.append("/.sixemugui-serial/config");
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

  dirp = opendir("/dev");
  if (dirp == NULL)
  {
    printf("Warning: can't open /dev directory %s\n", dir_path);
    return;
  }

  while ((d = readdir(dirp)))
  {
    if (d->d_type == DT_CHR && !strncmp(d->d_name, "ttyUSB", 6))
    {
      if(!line.empty() && line == d->d_name)
      {
        choice->SetSelection(choice->Append(wxString(d->d_name, wxConvUTF8)));
      }
      else
      {
        choice->Append(wxString(d->d_name, wxConvUTF8));
      }
    }
  }

  closedir(dirp);
}

static void read_filenames(wxChoice* choice)
{
    DIR *dirp;
    char dir_path[PATH_MAX];
    struct dirent *d;
    string filename = "";
    string line = "";

    filename.append(homedir);
    filename.append("/.sixemugui-serial/default");
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

    snprintf(dir_path, sizeof(dir_path), "%s/%s", homedir, CONFIG_DIR);
    dirp = opendir(dir_path);
    if (dirp == NULL)
    {
      printf("Warning: can't open config directory %s\n", dir_path);
      return;
    }

    while ((d = readdir(dirp)))
    {
      if (d->d_type == DT_REG)
      {
        if(!line.empty() && line == d->d_name)
        {
          choice->SetSelection(choice->Append(wxString(d->d_name, wxConvUTF8)));
        }
        else
        {
          choice->Append(wxString(d->d_name, wxConvUTF8));
        }
      }
    }

    closedir(dirp);
}

static void read_frequency(wxComboBox* choice)
{
  string filename = "";
  string line = "";

  filename.append(homedir);
  filename.append("/.sixemugui-serial/frequency");
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
#endif

sixemuguiFrame::sixemuguiFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(sixemuguiFrame)
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
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer11;
    wxMenu* Menu2;
    wxStaticBoxSizer* StaticBoxSizer5;

    Create(parent, wxID_ANY, _("Gimx-serial"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(412,420));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxSize(0,0), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("USB to serial"));
    FlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("Device"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ComboBox1 = new wxComboBox(Panel1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
    FlexGridSizer3->Add(ComboBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("emuclient"));
    FlexGridSizer5 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Update frequency "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ComboBox2 = new wxComboBox(Panel1, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(75,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX2"));
    ComboBox2->SetSelection( ComboBox2->Append(_("100")) );
    ComboBox2->Append(_("125"));
    ComboBox2->Append(_("250"));
    FlexGridSizer6->Add(ComboBox2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Hz"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox5 = new wxCheckBox(Panel1, ID_CHECKBOX5, _("Force updates"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox5->SetValue(true);
    FlexGridSizer2->Add(CheckBox5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("Mouse"));
    FlexGridSizer10 = new wxFlexGridSizer(1, 2, 0, 0);
    CheckBox1 = new wxCheckBox(Panel1, ID_CHECKBOX1, _("grab"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox1->SetValue(true);
    FlexGridSizer10->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox4 = new wxCheckBox(Panel1, ID_CHECKBOX4, _("calibrate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox4->SetValue(false);
    FlexGridSizer10->Add(CheckBox4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Output"));
    FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox2 = new wxCheckBox(Panel1, ID_CHECKBOX2, _("gui"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox2->SetValue(false);
    FlexGridSizer11->Add(CheckBox2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox3 = new wxCheckBox(Panel1, ID_CHECKBOX3, _("terminal"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBox3->SetValue(false);
    FlexGridSizer11->Add(CheckBox3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Config"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    Choice4 = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxSize(225,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    FlexGridSizer4->Add(Choice4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(StaticBoxSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(2, 1, 0, 0);
    Button1 = new wxButton(Panel1, ID_BUTTON1, _("Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer7->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button3 = new wxButton(Panel1, ID_BUTTON3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu1, ID_MENUITEM1, _("Edit config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem4 = new wxMenuItem(Menu1, ID_MENUITEM2, _("Edit fps config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem4);
    MenuItem5 = new wxMenuItem(Menu1, ID_MENUITEM3, _("Refresh"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem5);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
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
    SingleInstanceChecker1.Create(_T("Sixemugui-serial_") + wxGetUserId() + _T("_Guard"));

    Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&sixemuguiFrame::OnCheckBoxCalibrate);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&sixemuguiFrame::OnCheckBox2Click);
    Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&sixemuguiFrame::OnCheckBox3Click);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixemuguiFrame::OnButton1Click1);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&sixemuguiFrame::OnButton3Click);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixemuguiFrame::OnMenuEditConfig);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixemuguiFrame::OnMenuEditFpsConfig);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixemuguiFrame::OnMenuRefresh);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixemuguiFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&sixemuguiFrame::OnAbout);
    //*)

    if(SingleInstanceChecker1.IsAnotherRunning())
    {
        wxMessageBox( wxT("Sixemugui is already running!"), wxT("Error"), wxICON_ERROR);
        exit(-1);
    }

#ifndef WIN32
    homedir = getpwuid(getuid())->pw_dir;
#endif

    read_filenames(Choice4);
    read_frequency(ComboBox2);
    read_devices(ComboBox1);

    Refresh();
}

sixemuguiFrame::~sixemuguiFrame()
{
    //(*Destroy(sixemuguiFrame)
    //*)
}

void sixemuguiFrame::OnQuit(wxCommandEvent& event)
{
  Close();
}

void sixemuguiFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(_(INFO_DESCR)) + wxString(_("\n")) + wxString(_(INFO_YEAR)) + wxString(_(" ")) + wxString(_(INFO_DEV)) + wxString(_(" ")) + wxString(_(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void sixemuguiFrame::OnButton3Click(wxCommandEvent& event)
{
    string command = "";
    string filename = "";
    wxString dpi;
    int refresh;
    char crefresh[3];
    int frequency;
    wxString configname;

#ifdef WIN32
    command.append("emuclient.exe");
#else
    if(CheckBox3->IsChecked())
    {
        command.append("gnome-terminal -e \"");
    }
    else
    {
        command.append("/bin/bash -c \"");
    }
    command.append(" emuclient");
#endif
    command.append(" --precision 16 --serial");
    if(!CheckBox1->IsChecked())
    {
        command.append(" --nograb");
    }
    command.append(" --config ");
    configname = Choice4->GetStringSelection();
    configname.Replace(_(" "), _("\\ "));
    command.append(configname.mb_str());
    command.append(" --refresh ");
    frequency = wxAtoi(ComboBox2->GetValue());
    if(frequency)
    {
      refresh = 1000 / frequency;
    }
    else
    {
      refresh = 10;
    }
    snprintf(crefresh, sizeof(crefresh), "%d", refresh);
    command.append(crefresh);
    if(CheckBox5->IsChecked())
    {
        command.append(" --force-updates");
    }
    command.append(" --port ");
#ifndef WIN32
    command.append("/dev/");
#endif
    command.append(ComboBox1->GetStringSelection().mb_str());
    if(CheckBox2->IsChecked())
    {
        command.append(" --status | gimx-status");
    }
    else if(CheckBox3->IsChecked())
    {
        command.append(" --status");
    }
#ifndef WIN32
    command.append("\"");
#endif

    //cout << command << endl;

    Button3->Disable();
#ifdef WIN32
    filename.append("default");
#else
    filename.append(homedir);
    filename.append("/.sixemugui-serial/default");
#endif
    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        outfile << Choice4->GetStringSelection().mb_str() << endl;
        outfile.close();
    }
    filename.erase();
#ifdef WIN32
    filename.append("frequency");
#else
    filename.append(homedir);
    filename.append("/.sixemugui-serial/frequency");
#endif
    ofstream outfile2 (filename.c_str(), ios_base::trunc);
    if(outfile2.is_open())
    {
        outfile2 << ComboBox2->GetStringSelection().mb_str() << endl;
        outfile2.close();
    }
    filename.erase();
#ifdef WIN32
    filename.append("config");
#else
    filename.append(homedir);
    filename.append("/.sixemugui-serial/config");
#endif
    ofstream outfile3 (filename.c_str(), ios_base::trunc);
    if(outfile3.is_open())
    {
        outfile3 << ComboBox1->GetStringSelection().mb_str() << endl;
        outfile3.close();
    }

    if(system(command.c_str()) != 0)
    {
        wxMessageBox( wxT("Connection error!\nPlease check the USB to serial device!"), wxT("Error"), wxICON_ERROR);
    }

    Button3->Enable();
}

void sixemuguiFrame::OnCheckBox2Click(wxCommandEvent& event)
{
    CheckBox3->SetValue(false);
}

void sixemuguiFrame::OnCheckBox3Click(wxCommandEvent& event)
{
    CheckBox2->SetValue(false);
}

void sixemuguiFrame::OnCheckBoxCalibrate(wxCommandEvent& event)
{
    if(CheckBox4->IsChecked())
    {
        CheckBox2->SetValue(true);
        CheckBox3->SetValue(false);
    }
    else
    {
        CheckBox2->SetValue(false);
    }
}

#ifdef WIN32
#define CHECK_FILE "check_result"
#else
#define CHECK_FILE "/tmp/check_result"
#endif

void sixemuguiFrame::OnButton1Click1(wxCommandEvent& event)
{
    string command = "";
    string filename = "";
    string result = "";
    string line = "";
    wxString configname;

#ifdef WIN32
    command.append("emuclient.exe");
#else
    command.append("emuclient");
#endif
    command.append(" --config ");
    configname = Choice4->GetStringSelection();
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

void sixemuguiFrame::OnMenuEditConfig(wxCommandEvent& event)
{
  string command = "";
#ifdef WIN32
  command.append("gimx-config.exe -f ");
  command.append(Choice4->GetStringSelection().mb_str());

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
  command.append(Choice4->GetStringSelection().mb_str());
  command.append(" &");

  if (system(command.c_str()) != 0)
  {
    wxMessageBox(wxT("Error editing the config file!"), wxT("Error"),
        wxICON_ERROR);
  }
#endif
}

void sixemuguiFrame::OnMenuEditFpsConfig(wxCommandEvent& event)
{
  string command = "";
#ifdef WIN32
  command.append("gimx-fpsconfig.exe -f ");
  command.append(Choice4->GetStringSelection().mb_str());

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
  command.append(Choice4->GetStringSelection().mb_str());
  command.append(" &");

  if (system(command.c_str()) != 0)
  {
    wxMessageBox(wxT("Error editing the config file!"), wxT("Error"),
        wxICON_ERROR);
  }
#endif
}

void sixemuguiFrame::OnMenuRefresh(wxCommandEvent& event)
{
    wxString previous = Choice4->GetStringSelection();
    Choice4->Clear();
    read_filenames(Choice4);
    Choice4->SetSelection(Choice4->FindString(previous));
    previous = ComboBox1->GetStringSelection();
    ComboBox1->Clear();
    read_devices(ComboBox1);
    ComboBox1->SetSelection(ComboBox1->FindString(previous));
}
