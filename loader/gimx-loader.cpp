/*
 Copyright (c) 2016 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "gimx-loader.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(loaderFrame)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <string.h>

#include <wx/aboutdlg.h>
#include <wx/busyinfo.h>
#include <wx/dir.h>
#include <wx/file.h>
#include "loader.h"

#include "../directories.h"

#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <stdlib.h>
#endif

using namespace std;

#define MAX_PORT_NB 257 // 0 to 256

#ifdef WIN32
static void check_port(int i) {

    char path[sizeof("\\\\.\\COM256")];
    snprintf(path, sizeof(path), "\\\\.\\COM%d", i);
    HANDLE handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        return 0;
    }
    return -1;
}
#else
static int check_port(int i) {

    char path[sizeof("/dev/ttyACM256")];
    snprintf(path, sizeof(path), "/dev/ttyACM%d", i);
    int fd = open(path, O_RDWR);
    if (fd != -1) {
        close(fd);
        return 0;
    }
    return -1;
}
#endif

static void list_ports(int ports[MAX_PORT_NB]) {

    int i;
    for (i = 0; i < MAX_PORT_NB; ++i) {
        if (check_port(i) != -1) {
            ports[i] = 1;
        }
    }
}

//(*IdInit(loaderFrame)
const long loaderFrame::ID_CHOICE1 = wxNewId();
const long loaderFrame::ID_BUTTON1 = wxNewId();
const long loaderFrame::ID_PANEL1 = wxNewId();
const long loaderFrame::ID_MENUITEM1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(loaderFrame,wxFrame)
//(*EventTable(loaderFrame)
//*)
END_EVENT_TABLE()

loaderFrame::loaderFrame(wxWindow* parent,wxWindowID id __attribute__((unused)))
{
    locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));

    //(*Initialize(loaderFrame)
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, wxID_ANY, _("Gimx-loader"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    ChoiceFirmware = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ChoiceFirmware, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonLoad = new wxButton(Panel1, ID_BUTTON1, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(ButtonLoad, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(Panel1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, ID_MENUITEM1, _("About"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("Help"));
    SetMenuBar(MenuBar1);
    Center();

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&loaderFrame::OnButtonLoadClick);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&loaderFrame::OnAbout);
    //*)

    wxString fw = wxT(FIRMWARE_DIR);

    wxDir dir(fw);

    if(!dir.IsOpened())
    {
        wxMessageBox( _("Cannot open directory: ") + fw, _("Error"), wxICON_ERROR);
        exit(-1);
    }

    wxString file;
    wxString filespec = wxT("*.hex");

    for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
    {
      ChoiceFirmware->Append(file);
    }

    if(ChoiceFirmware->IsEmpty())
    {
        wxMessageBox( _("No firmware found in ") + fw, _("Error"), wxICON_ERROR);
        exit(-1);
    }

    ChoiceFirmware->SetSelection(0);

    FlexGridSizer1->Layout();
    Panel1->Fit();
    Fit();
    Refresh();
}

loaderFrame::~loaderFrame() {
    //(*Destroy(loaderFrame)
    //*)
}

void loaderFrame::OnQuit(wxCommandEvent& event __attribute__((unused))) {
    Close();
}

void loaderFrame::OnAbout(wxCommandEvent& event __attribute__((unused))) {
    wxAboutDialogInfo info;
    info.SetName(wxTheApp->GetAppName());
    info.SetVersion(wxT(INFO_VERSION) + wxString(wxT(" ")) + wxString(wxT(INFO_ARCH)));
    wxString text = wxString(wxT(INFO_DESCR)) + wxString(wxT("\n")) + wxString(wxT(INFO_YEAR)) + wxString(wxT(" "))
            + wxString(wxT(INFO_DEV)) + wxString(wxT(" ")) + wxString(wxT(INFO_LICENCE));
    info.SetDescription(text);
    info.SetWebSite(wxT(INFO_WEB));

    wxAboutBox(info);
}

class MyProcess: public wxProcess {
public:
    MyProcess(loaderFrame *parent, const wxString& cmd) :
            wxProcess(parent), m_cmd(cmd) {
        m_parent = parent;
    }

    void OnTerminate(int pid, int status);

protected:
    loaderFrame *m_parent;
    wxString m_cmd;
};

void MyProcess::OnTerminate(int pid __attribute__((unused)), int status) {
    m_parent->OnProcessTerminated(this, status);
}

void loaderFrame::OnButtonLoadClick(wxCommandEvent& event __attribute__((unused))) {

    ButtonLoad->Enable(false);

    int answer = wxMessageBox(_("This tool is only compatible with GIMX adapters made with a USB to UART adapter and a Pro Micro board. Proceed?"), _(""), wxICON_INFORMATION | wxOK | wxCANCEL);
    if (answer != wxOK) {
        ButtonLoad->Enable(true);
        return;
    }

    answer = wxMessageBox(_("Plug both sides of the adapter to the computer."), _(""), wxICON_INFORMATION | wxOK | wxCANCEL);
    if (answer != wxOK) {
        ButtonLoad->Enable(true);
        return;
    }

    int ports[MAX_PORT_NB] = {};

    list_ports(ports);

    int i = -1;
    int count;

    {
        wxWindowDisabler disableAll;
        wxBusyInfo wait(_("Unplug/replug the USB cable (Pro Micro side of the adapter)."));

        for (count = 0; count < 40; ++count) {
            for (i = 0; i < MAX_PORT_NB; ++i) {
                if (ports[i] == 0) {
                    if (check_port(i) != -1) {
                        break;
                    }
                }
            }
            if (i < MAX_PORT_NB) {
                break;
            }
            usleep(250000);

            wxTheApp->Yield();
        }
    }

    if (count == 40) {
        wxMessageBox(_("No new device found within 10 seconds."), _("Error"), wxICON_ERROR);
        ButtonLoad->Enable(true);
        return;
    }

    wxString command;

    command.Append(wxT("avrdude -p atmega32u4 -c avr109 -P "));
#ifndef WIN32
    command.Append(wxT("/dev/ttyACM"));
#else
    command.Append(wxT("COM"));
#endif
    command.Append(wxString::Format(wxT("%i"), i));
    command.Append(wxT(" -D -U flash:w:"));
    command.Append(wxT(FIRMWARE_DIR));
    command.Append(wxT("/"));
    command.Append(ChoiceFirmware->GetStringSelection());
    command.Append(wxT(":i"));

    MyProcess *process = new MyProcess(this, command);

    if (!wxExecute(command, wxEXEC_ASYNC | wxEXEC_NOHIDE, process)) {
        wxMessageBox(_("failed to load firmware"), _("Error"), wxICON_ERROR);
        ButtonLoad->Enable(true);
    }
}

void loaderFrame::OnProcessTerminated(wxProcess *process __attribute__((unused)), int status) {

    ButtonLoad->Enable(true);

    if (status) {
        wxMessageBox(_("Failed to load firmware."), _("Error"), wxICON_ERROR);
    } else {
        wxMessageBox(_("Firmware loaded successfully!"), _("Info"), wxICON_INFORMATION);
    }

    SetFocus();
}

