/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "launcherMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(launcherFrame)
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
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include <wx/file.h>
#include "launcher.h"

#include "../directories.h"
#include "../shared/updater/updater.h"
#include "../shared/configupdater/configupdater.h"
#include <ConfigurationFile.h>

#include <wx/arrstr.h>
#include <wx/stdpaths.h>
#include <wx/busyinfo.h>
#include "wx/numdlg.h"

#include <time.h>

using namespace std;

#ifdef WIN32
#define MAX_PORT_ID 32
#endif

#define BLUETOOTH_DIR "/.sixemugui"
#define OUTPUT_FILE "/controller"
#define INPUT_FILE "/input"
#define PS3_PAIRINGS "/PS3Pairings"
#define PS4_PAIRINGS "/PS4Pairings"
#define IP_DESTS "/IPDests"
#define IP_SOURCES "/IPSources"
#define START_UPDATES "/startUpdates"
#define PORT_FILE "/port"

wxString gimxConfigDir;
wxString launcherDir;

//(*IdInit(launcherFrame)
const long launcherFrame::ID_STATICTEXT4 = wxNewId();
const long launcherFrame::ID_CHOICE1 = wxNewId();
const long launcherFrame::ID_STATICTEXT3 = wxNewId();
const long launcherFrame::ID_CHOICE3 = wxNewId();
const long launcherFrame::ID_BUTTON2 = wxNewId();
const long launcherFrame::ID_STATICTEXT1 = wxNewId();
const long launcherFrame::ID_CHOICE2 = wxNewId();
const long launcherFrame::ID_STATICTEXT2 = wxNewId();
const long launcherFrame::ID_CHOICE5 = wxNewId();
const long launcherFrame::ID_BUTTON4 = wxNewId();
const long launcherFrame::ID_CHECKBOX1 = wxNewId();
const long launcherFrame::ID_CHECKBOX2 = wxNewId();
const long launcherFrame::ID_CHECKBOX3 = wxNewId();
const long launcherFrame::ID_CHOICE4 = wxNewId();
const long launcherFrame::ID_BUTTON1 = wxNewId();
const long launcherFrame::ID_BUTTON3 = wxNewId();
const long launcherFrame::ID_PANEL1 = wxNewId();
const long launcherFrame::ID_MENUITEM1 = wxNewId();
const long launcherFrame::ID_MENUITEM2 = wxNewId();
const long launcherFrame::ID_MENUITEM8 = wxNewId();
const long launcherFrame::ID_MENUITEM7 = wxNewId();
const long launcherFrame::ID_MENUITEM3 = wxNewId();
const long launcherFrame::ID_MENUITEM9 = wxNewId();
const long launcherFrame::idMenuQuit = wxNewId();
const long launcherFrame::ID_MENUITEM6 = wxNewId();
const long launcherFrame::ID_MENUITEM4 = wxNewId();
const long launcherFrame::ID_MENUITEM5 = wxNewId();
const long launcherFrame::idMenuAbout = wxNewId();
const long launcherFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(launcherFrame,wxFrame)
    //(*EventTable(launcherFrame)
    //*)
END_EVENT_TABLE()

/*
 * \brief This function performs a synchronous execution of a given command, and parses the command output.
 * 
 * \param command    the command to execute
 * \param firstLine  in case the output is a list of line blocks, this indicates an entry delimiter
 * \param params     the list of strings to look for in the command output
 * \param results    the values parsed from the command output
 * \param nbResults  specifies the size of the results array
 * 
 * \return 0 if command exited with 0, -1 otherwise
 */
static int readCommandResults(wxString command, wxString firstLine, wxArrayString params, wxArrayString results[], int nbResults)
{
    int ret = 0;
    wxArrayString output, errors;
    int index = -1;

    if(!wxExecute(command, output, errors, wxEXEC_SYNC))
    {
      for(unsigned int j=0; j<output.GetCount(); ++j)
      {
        //find a line that matches firstLine
        if(output[j].Find(firstLine) != wxNOT_FOUND)
        {
          //increase the result index
          ++index;
          if(index == nbResults)
          {
            break;
          }
          //init the result
          for(unsigned int i=0; i<params.GetCount(); ++i)
          {
            results[index].Add(wxEmptyString);
          }
        }
        
        //firstLine has not been found yet
        if(index < 0)
        {
          continue;
        }
        
        for(unsigned int i=0; i<params.GetCount(); ++i)
        {
          //check if line contains something interesting
          int pos = output[j].Find(params[i]);
          if(pos != wxNOT_FOUND)
          {
            //store the remaining chars of the line
            results[index][i] = output[j].Mid(pos+params[i].Length());
            break;
          }
        }
      }
    }
    else
    {
      //command execution has failed
      ret = -1;
    }

    return ret;
}

/*
 * \brief Read the bluetooth pairings with a given tool.
 * 
 * \param bluetoothPairings  the vector to store the pairings
 * \param tool               the tool, e.g. sixaddr or ds4tool
 */
void launcherFrame::readPairings(vector<BluetoothPairing>& bluetoothPairings, wxString tool)
{
    wxString firstLine = wxT("Current Bluetooth master: ");
    wxArrayString params;
    params.Add(wxT("Current Bluetooth master: "));
    params.Add(wxT("Current Bluetooth Device Address: "));
    params.Add(wxT("Current link key: "));
    wxArrayString results[7];
    wxString command = tool;

    int res = readCommandResults(command, firstLine, params, results, sizeof(results)/sizeof(*results));

    if(res != -1)
    {
        for(unsigned int i=0; i<sizeof(results)/sizeof(*results); ++i)
        {
            if(results[i].IsEmpty())
            {
              break;
            }

            BluetoothPairing pairing;
            pairing.console = results[i][0].MakeUpper();
            pairing.controller = results[i][1].MakeUpper();
            pairing.linkKey = results[i][2].MakeUpper();
            bluetoothPairings.push_back(pairing);
        }
    }
}

/*
 * \brief Read bluetooth device properties.
 * 
 * \param dongleInfos  the vector to store the properties
 */
void launcherFrame::readDongles(vector<DongleInfo>& dongleInfos)
{
  //retrieve the hci indexes
  
  wxArrayString hciDevices;
  
  wxString firstLine = wxT("hci");
  wxArrayString params;
  params.Add(wxT("hci"));
  wxArrayString results[7];
  
  int res = readCommandResults(wxT("hciconfig"), firstLine, params, results, sizeof(results)/sizeof(*results));

  if(res != -1)
  {
    for(unsigned int i=0; i<sizeof(results)/sizeof(*results); ++i)
    {
      if(results[i].IsEmpty())
      {
        break;
      }

      hciDevices.push_back(results[i][0].BeforeFirst(wxChar(':')));
    }
  }
  else
  {
    return;
  }
  
  wxString firstLine1 = wxT("BD Address: ");
  wxArrayString params1;
  params1.Add(wxT("BD Address: "));
  params1.Add(wxT("Manufacturer: "));
  wxArrayString results1[1];
  
  wxString firstLine2 = wxT("Chip version: ");
  wxArrayString params2;
  params2.Add(wxT("Chip version: "));
  wxArrayString results2[1];
  
  for(unsigned int i=0; i<hciDevices.GetCount(); ++i)
  {
    wxString device = wxT("hci");
    device.Append(hciDevices[i]);

    wxString command = wxT("hciconfig -a ") + device;

    res = readCommandResults(command, firstLine1, params1, results1, sizeof(results1)/sizeof(*results1));

    if (res != -1)
    {
      DongleInfo di;
      di.hci = device;
      di.address = results1[0][0].Left(17);
      di.manufacturer = results1[0][1];

      command = wxT("hcirevision ") + device;
      res = readCommandResults(command, firstLine2, params2, results2, sizeof(results2)/sizeof(*results2));

      wxString chip = wxEmptyString;
      if (res != -1 && results2[0][0] != wxT("Unknown"))
      {
        chip = results2[0][0];
      }
      di.chip = chip;

      dongleInfos.push_back(di);
    }
  }
}

int launcherFrame::setDongleAddress(vector<DongleInfo>& dongleInfos, int dongleIndex, wxString address)
{
    int j = 0;
    wxString firstLine1 = wxT("Address changed - ");
    wxArrayString params1;
    params1.Add(wxT("Address changed - "));
    wxArrayString results1[1];
    wxString firstLine2 = wxT("Device address: ");
    wxArrayString params2;
    params2.Add(wxT("Device address: "));
    wxArrayString results2[1];
    int res;

    int pos = -1;
    
    for(unsigned int i=0; i<dongleInfos.size(); ++i)
    {
      if(dongleInfos[i].address == address)
      {
        pos = i;
        break;
      }
    }

    if(pos != -1)
    {
      if(pos == dongleIndex)
      {
        return 0;
      }
      else
      {
        wxMessageBox( _("Address already used!"), _("Error"), wxICON_ERROR);
        return -1;
      }
    }

    int answer = wxMessageBox(_("Did you saved your dongle address?"), _("Confirm"), wxYES_NO | wxCANCEL);

    if (answer != wxYES)
    {
      wxMessageBox(_("Please save it!"), _("Info"));
      return -1;
    }

    wxString command = wxT("bdaddr -r -i ") + dongleInfos[dongleIndex].hci + wxT(" ") + address;
    res = readCommandResults(command, firstLine1, params1, results1, sizeof(results1)/sizeof(*results1));

    if(res != -1)
    {
      wxMessageBox( results1[0][0], _("Success"), wxICON_INFORMATION);
    }

    //wait up to 5s for the device to come back
    command = wxT("bdaddr -i ") + dongleInfos[dongleIndex].hci;
    while(readCommandResults(command, firstLine2, params2, results2, sizeof(results2)/sizeof(*results2)) == -1 && j<50)
    {
        usleep(100000);
        j++;
    }

    if(results2[0][0] !=  address)
    {
        wxMessageBox( _("Read address after set: ko!"), _("Error"), wxICON_ERROR);
        return -1;
    }
    else
    {
        wxMessageBox( _("Read address after set: seems ok!"), _("Success"), wxICON_INFORMATION);
    }

    return 0;
}

#ifdef WIN32
void launcherFrame::readSerialPorts()
{
  HANDLE hSerial;
  DWORD accessdirection = /*GENERIC_READ |*/GENERIC_WRITE;
  char portname[16];
  wchar_t szCOM[16];
  int i;
  wxString previous = ChoiceOutput->GetStringSelection();

  ChoiceOutput->Clear();

  for(i=0; i<MAX_PORT_ID; ++i)
  {
    wsprintf(szCOM, L"\\\\.\\COM%d", i);
    hSerial = CreateFile(szCOM, accessdirection, 0, 0, OPEN_EXISTING, 0, 0);
    if (hSerial != INVALID_HANDLE_VALUE)
    {
      DCB oldDcbSerialParams = { 0 };
      oldDcbSerialParams.DCBlength = sizeof(oldDcbSerialParams);
      if (GetCommState(hSerial, &oldDcbSerialParams))
      {
        DCB newDcbSerialParams;
        memcpy(&newDcbSerialParams, &oldDcbSerialParams, sizeof(newDcbSerialParams));
        newDcbSerialParams.BaudRate = 500000;
        newDcbSerialParams.ByteSize = 8;
        newDcbSerialParams.StopBits = ONESTOPBIT;
        newDcbSerialParams.Parity = NOPARITY;
        if (SetCommState(hSerial, &newDcbSerialParams))//test port parameters
        {
          snprintf(portname, sizeof(portname), "COM%d", i);
          ChoiceOutput->SetSelection(ChoiceOutput->Append(wxString(portname, wxConvUTF8)));
        }
        SetCommState(hSerial, &oldDcbSerialParams);//restore port parameters, do not care about any error
      }
    }
    CloseHandle(hSerial);
  }

  if(previous != wxEmptyString)
  {
    ChoiceOutput->SetSelection(ChoiceOutput->FindString(previous));
  }
  if(ChoiceOutput->GetSelection() < 0)
  {
    ChoiceOutput->SetSelection(0);
  }
}
#else
void launcherFrame::readSerialPorts()
{
  string filename;
  string line = "";
  wxString previous = ChoiceOutput->GetStringSelection();

  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append("/config");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
      if( infile.good() )
      {
          getline (infile,line);
      }
      infile.close();
  }

  ChoiceOutput->Clear();

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
    if(file.StartsWith(wxT("ttyUSB")) || file.StartsWith(wxT("ttyACM")))
    {
      if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
      {
        ChoiceOutput->SetSelection(ChoiceOutput->Append(file));
      }
      else
      {
        ChoiceOutput->Append(file);
      }
    }
  }

  if(previous != wxEmptyString)
  {
    ChoiceOutput->SetSelection(ChoiceOutput->FindString(previous));
  }
  if(ChoiceOutput->GetSelection() < 0)
  {
    ChoiceOutput->SetSelection(0);
  }
}
#endif

void launcherFrame::readConfigs()
{
  string filename;
  string line = "";
  wxString previous = ChoiceConfig->GetStringSelection();

  /* Read the last config used so as to auto-select it. */
  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append("/default");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
    }
    infile.close();
  }

  ChoiceConfig->Clear();

  /* Read all config file names. */
  wxDir dir(gimxConfigDir);

  if(!dir.IsOpened())
  {
    return;
  }

  wxString file;
  wxString filespec = wxT("*.xml");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
    {
      previous = file;
    }
    ChoiceConfig->Append(file);
  }

  if(previous != wxEmptyString)
  {
    ChoiceConfig->SetSelection(ChoiceConfig->FindString(previous));
  }
  if(ChoiceConfig->GetSelection() < 0)
  {
    ChoiceConfig->SetSelection(0);
  }
}

int launcherFrame::readChoices(const char* file, wxChoice* choices)
{
    string filename;
    string line;
    string device;
    string master;
    int ret = -1;

    choices->Clear();

    filename = string(launcherDir.mb_str(wxConvUTF8));
    filename.append(file);

    if(!::wxFileExists(wxString(filename.c_str(), wxConvUTF8)))
    {
      return 0;
    }

    ifstream myfile(filename.c_str());
    if(myfile.is_open())
    {
        while ( myfile.good() )
        {
            getline (myfile,line);
            if(!line.empty())
            {
                choices->Append(wxString(line.c_str(), wxConvUTF8));
                ret = 0;
            }
        }
        choices->SetSelection(0);
        myfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }

    return ret;
}

int launcherFrame::saveChoices(const char* file, wxChoice* choices)
{
    string filename;
    string line;
    string device;
    string master;
    int ret = -1;

    filename = string(launcherDir.mb_str(wxConvUTF8));
    filename.append(file);

    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        if(!choices->GetStringSelection().IsEmpty())
        {
          outfile << choices->GetStringSelection().mb_str(wxConvUTF8) << endl;
        }
        for(int i=0; i<(int)choices->GetCount(); i++)
        {
            if(i != choices->GetSelection())
            {
                outfile << choices->GetString(i).mb_str(wxConvUTF8) << endl;
            }
        }
        outfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file for writing: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
    return ret;
}

void launcherFrame::readParam(const char* file, wxChoice* choice)
{
  string filename;
  string line = "";

  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append(file);
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

void launcherFrame::readStartUpdates()
{
  string filename;
  string line = "";

  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append(START_UPDATES);
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
      if(line == "yes")
      {
        MenuStartupUpdates->Check(true);
      }
    }
    infile.close();
  }
}

launcherFrame::launcherFrame(wxWindow* parent,wxWindowID id)
{
    locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

    //(*Initialize(launcherFrame)
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItem2;
    wxFlexGridSizer* FlexGridSizer10;
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
    wxFlexGridSizer* FlexGridSizer11;
    wxMenu* Menu2;

    Create(parent, wxID_ANY, _("Gimx-launcher"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
    IOSizer = new wxFlexGridSizer(4, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ControllerType = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ControllerType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OutputSizer = new wxFlexGridSizer(1, 3, 0, 0);
    OutputText = new wxStaticText(Panel1, ID_STATICTEXT3, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    OutputSizer->Add(OutputText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceOutput = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    OutputSizer->Add(ChoiceOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OutputNewButton = new wxButton(Panel1, ID_BUTTON2, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    OutputSizer->Add(OutputNewButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(OutputSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Input"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    sourceChoice = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    sourceChoice->SetSelection( sourceChoice->Append(_("Physical inputs")) );
    sourceChoice->Append(_("Network"));
    FlexGridSizer5->Add(sourceChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SourceIpSizer = new wxFlexGridSizer(1, 3, 0, 0);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("IP:port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    SourceIpSizer->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceInput = new wxChoice(Panel1, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    SourceIpSizer->Add(ChoiceInput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(Panel1, ID_BUTTON4, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    SourceIpSizer->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(SourceIpSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(IOSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    MouseSizer = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("Mouse"));
    FlexGridSizer10 = new wxFlexGridSizer(1, 1, 0, 0);
    CheckBoxGrab = new wxCheckBox(Panel1, ID_CHECKBOX1, _("grab"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxGrab->SetValue(true);
    FlexGridSizer10->Add(CheckBoxGrab, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MouseSizer->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(MouseSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Config"));
    FlexGridSizer4 = new wxFlexGridSizer(1, 1, 0, 0);
    ChoiceConfig = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE4"));
    FlexGridSizer4->Add(ChoiceConfig, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer8->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(StaticBoxSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(2, 1, 0, 0);
    ButtonCheck = new wxButton(Panel1, ID_BUTTON1, _("Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer7->Add(ButtonCheck, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStart = new wxButton(Panel1, ID_BUTTON3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(ButtonStart, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(Panel1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuEditConfig = new wxMenuItem(Menu1, ID_MENUITEM1, _("Edit config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuEditConfig);
    MenuEditFpsConfig = new wxMenuItem(Menu1, ID_MENUITEM2, _("Edit fps config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuEditFpsConfig);
    MenuItem3 = new wxMenuItem(Menu1, ID_MENUITEM8, _("Open config directory"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuAutoBindControls = new wxMenuItem(Menu1, ID_MENUITEM7, _("Auto-bind and convert"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuAutoBindControls);
    MenuRefresh = new wxMenuItem(Menu1, ID_MENUITEM3, _("Refresh\tF5"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuRefresh);
    MenuItem4 = new wxMenuItem(Menu1, ID_MENUITEM9, _("Save\tCtrl-S"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem4);
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
    SingleInstanceChecker1.Create(_T("gimx-launcher_") + wxGetUserId() + _T("_Guard"));

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&launcherFrame::OnControllerTypeSelect);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnOutputNewButtonClick);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&launcherFrame::OnsourceChoiceSelect);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnInputNewButtonClick);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&launcherFrame::OnCheckBoxGuiClick);
    Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&launcherFrame::OnCheckBoxTerminalClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnButtonCheckClick1);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnButtonStartClick);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuEditConfig);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuEditFpsConfig);
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuOpenConfigDirectory);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuAutoBindControls);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuRefresh);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuSave);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnQuit);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuGetConfigs);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuUpdate);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnMenuStartupUpdates);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&launcherFrame::OnAbout);
    //*)

    if(SingleInstanceChecker1.IsAnotherRunning())
    {
        wxMessageBox( _("gimx-launcher is already running!"), _("Error"), wxICON_ERROR);
        exit(-1);
    }

    launcherDir = wxStandardPaths::Get().GetUserDataDir();
    if(!wxDir::Exists(launcherDir))
    {
      if(!wxMkdir(launcherDir))
      {
        wxMessageBox( _("Can't init directory: ") + launcherDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    //migrate old config file from gimx-bluetooth
    wxString oldConfig = wxStandardPaths::Get().GetUserConfigDir() + wxT(BLUETOOTH_DIR) + wxT("/config");
    wxString config = launcherDir + wxT(PS3_PAIRINGS);
    if(wxFile::Exists(oldConfig) && !wxFile::Exists(config))
    {
      if(!wxRenameFile(oldConfig, config))
      {
        wxMessageBox( _("Can't migrate config: ") + oldConfig, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    //migrate old config/ directory if present
    wxString oldGimxDir = wxStandardPaths::Get().GetUserConfigDir() + wxT(OLD_GIMX_DIR);
    wxString gimxDir = wxStandardPaths::Get().GetUserConfigDir() + wxT(GIMX_DIR);
    if(wxDir::Exists(oldGimxDir) && !wxDir::Exists(gimxDir))
    {
      if(!wxRenameFile(oldGimxDir, gimxDir))
      {
        wxMessageBox( _("Can't migrate directory: ") + oldGimxDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    //create config/ directory if absent
    if(!wxDir::Exists(gimxDir))
    {
      if(!wxMkdir(gimxDir))
      {
        wxMessageBox( _("Can't init directory: ") + gimxDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }
    gimxConfigDir = gimxDir + wxT(CONFIG_DIR);
    if(!wxDir::Exists(gimxConfigDir))
    {
      if(!wxMkdir(gimxConfigDir))
      {
        wxMessageBox( _("Can't init directory: ") + gimxConfigDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    started = false;

#ifndef WIN32
    ControllerType->SetSelection( ControllerType->Append(_("Bluetooth / PS3")) );
    ControllerType->Append(_("Bluetooth / PS4"));
    ControllerType->Append(_("DIY USB"));
    ControllerType->Append(_("GPP/Cronus"));
    ControllerType->Append(_("Remote GIMX"));
#else
    ControllerType->SetSelection( ControllerType->Append(_("DIY USB")) );
    ControllerType->Append(_("GPP/Cronus"));
    ControllerType->Append(_("Remote GIMX"));
#endif

    readParam(OUTPUT_FILE, ControllerType);
    readParam(INPUT_FILE, sourceChoice);
    readConfigs();

    wxCommandEvent event;
    OnControllerTypeSelect(event);
    OnsourceChoiceSelect(event);

    readStartUpdates();
    if(MenuStartupUpdates->IsChecked())
    {
      OnMenuUpdate(event);
    }

    started = true;

    if(ChoiceConfig->IsEmpty())
    {
      int answer = wxMessageBox(_("No config found! Download configs?"), _("Confirm"), wxYES_NO);
      if (answer == wxYES)
      {
        wxCommandEvent event;
        OnMenuGetConfigs(event);
      }
    }

    refreshGui();
}

launcherFrame::~launcherFrame()
{
    //(*Destroy(launcherFrame)
    //*)
}

void launcherFrame::OnQuit(wxCommandEvent& event)
{
  Close();
}

void launcherFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(wxT(INFO_DESCR)) + wxString(wxT("\n")) + wxString(wxT(INFO_YEAR)) + wxString(wxT(" ")) + wxString(wxT(INFO_DEV)) + wxString(wxT(" ")) + wxString(wxT(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

class MyProcess : public wxProcess
{
public:
    MyProcess(launcherFrame *parent, const wxString& cmd)
        : wxProcess(parent), m_cmd(cmd)
    {
        m_parent = parent;
    }

    void OnTerminate(int pid, int status);

protected:
    launcherFrame *m_parent;
    wxString m_cmd;
};

void MyProcess::OnTerminate(int pid, int status)
{
    m_parent->OnProcessTerminated(this, status);
}

void launcherFrame::OnButtonStartClick(wxCommandEvent& event)
{
    wxString command;
    string filename;
    wxString hciIndex = wxEmptyString;
    wxString bdaddrDst = wxEmptyString;

    wxString output = ChoiceOutput->GetStringSelection();

    if(ChoiceConfig->GetStringSelection().IsEmpty())
    {
      wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
      return;
    }

    if(ControllerType->GetStringSelection() == _("DIY USB"))
    {
      if(output.IsEmpty())
      {
        wxMessageBox( _("No USB to serial device selected!"), _("Error"), wxICON_ERROR);
        return;
      }
    }
    else if(ControllerType->GetStringSelection() == _("Remote GIMX"))
    {
      if(output.IsEmpty())
      {
        wxMessageBox( _("No destination IP:port specified!"), _("Error"), wxICON_ERROR);
        return;
      }
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS3")
         || ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
    {
      if(output.IsEmpty())
      {
        wxMessageBox( _("No pairing selected!"), _("Error"), wxICON_ERROR);
        return;
      }

      int pos = output.Find(wxT(" "));

      if(pos == wxNOT_FOUND)
      {
        wxMessageBox( _("Selected pairing is incorrect!"), _("Error"), wxICON_ERROR);
        return;
      }

      wxString bdaddrSrc = output.BeforeFirst(wxChar(' '));
      bdaddrDst = output.AfterFirst(wxChar(' '));

      DongleInfo dongleInfo;

      if(chooseDongle(bdaddrSrc, dongleInfo) < 0)
      {
        return;
      }
      
      hciIndex = dongleInfo.hci.Mid(3);
    }

    if(sourceChoice->GetStringSelection() == _("Network"))
    {
      if(ChoiceInput->GetStringSelection().IsEmpty())
      {
        wxMessageBox( _("No source IP:port specified!"), _("Error"), wxICON_ERROR);
        return;
      }
    }

    if(ControllerType->GetStringSelection() == _("Remote GIMX")
    && sourceChoice->GetStringSelection() == _("Network"))
    {
      if(ChoiceInput->GetStringSelection() == output)
      {
        wxMessageBox( _("IP:port is the same for input and output!"), _("Error"), wxICON_ERROR);
        return;
      }
    }

#ifndef WIN32
    command.Append(wxT("xterm -e "));
#endif
    command.Append(wxT("gimx"));

    if(!CheckBoxGrab->IsChecked())
    {
        command.Append(wxT(" --nograb"));
    }
    command.Append(wxT(" --config \""));
    command.Append(ChoiceConfig->GetStringSelection());
    command.Append(wxT("\""));
    command.Append(wxT(" --force-updates"));
    command.Append(wxT(" --subpos"));

    if(ControllerType->GetStringSelection() == _("GPP/Cronus"))
    {
      command.Append(wxT(" --type GPP"));
    }
    else if(ControllerType->GetStringSelection() == _("Remote GIMX"))
    {
      command.Append(wxT(" --dst "));
      command.Append(output);
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS3"))
    {
      command.Append(wxT(" --type Sixaxis"));

      command.Append(wxT(" --hci "));
      command.Append(hciIndex);

      command.Append(wxT(" --bdaddr "));
      command.Append(bdaddrDst);
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
    {
      command.Append(wxT(" --type DS4"));

      command.Append(wxT(" --hci "));
      command.Append(hciIndex);

      command.Append(wxT(" --bdaddr "));
      command.Append(bdaddrDst);
    }
    else if(ControllerType->GetStringSelection() == _("DIY USB"))
    {
      command.Append(wxT(" --port "));
#ifndef WIN32
      command.Append(wxT("/dev/"));
#endif
      command.Append(output);
    }
    
    if(sourceChoice->GetStringSelection() == _("Network"))
    {
      command.Append(wxT(" --src "));
      command.Append(ChoiceInput->GetStringSelection());
    }

    if(CheckBoxTerminal->IsChecked())
    {
        command.Append(wxT(" --status"));
    }
    else if(CheckBoxGui->IsChecked())
    {
      command.Append(wxT(" --curses"));
    }

    StatusBar1->SetStatusText(_("Press Shift+Esc to exit."));

    ButtonStart->Enable(false);

    //wxMessageBox( command, _("Error"), wxICON_ERROR);

    MyProcess *process = new MyProcess(this, command);

    if(!wxExecute(command, wxEXEC_ASYNC | wxEXEC_NOHIDE, process))
    {
      wxMessageBox( _("can't start emuclient!"), _("Error"), wxICON_ERROR);
    }

    //TODO: sauver les options lorsque le programme se termine

    filename = launcherDir.mb_str(wxConvUTF8);
    filename.append("/default");
    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        outfile << ChoiceConfig->GetStringSelection().mb_str(wxConvUTF8) << endl;
        outfile.close();
    }
    filename = launcherDir.mb_str(wxConvUTF8);
    filename.append("/config");
    ofstream outfile3 (filename.c_str(), ios_base::trunc);
    if(outfile3.is_open())
    {
        outfile3 << output.mb_str(wxConvUTF8) << endl;
        outfile3.close();
    }
    filename = launcherDir.mb_str(wxConvUTF8);
    filename.append("/controller");
    ofstream outfile4 (filename.c_str(), ios_base::trunc);
    if(outfile4.is_open())
    {
        outfile4 << ControllerType->GetStringSelection().mb_str(wxConvUTF8) << endl;
        outfile4.close();
    }
}

void launcherFrame::OnProcessTerminated(wxProcess *process, int status)
{
    ButtonStart->Enable(true);
    StatusBar1->SetStatusText(wxEmptyString);

    if(status)
    {
      wxMessageBox( _("emuclient error"), _("Error"), wxICON_ERROR);
    }

    SetFocus();
}

void launcherFrame::OnCheckBoxGuiClick(wxCommandEvent& event)
{
    CheckBoxTerminal->SetValue(false);
}

void launcherFrame::OnCheckBoxTerminalClick(wxCommandEvent& event)
{
    CheckBoxGui->SetValue(false);
}

void launcherFrame::OnButtonCheckClick1(wxCommandEvent& event)
{
    if(ChoiceConfig->GetStringSelection().IsEmpty())
    {
      wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
      return;
    }

    string file = string(gimxConfigDir.mb_str(wxConvUTF8));
    file.append(ChoiceConfig->GetStringSelection().mb_str(wxConvUTF8));

    ConfigurationFile configFile;
    int ret = configFile.ReadConfigFile(file);

    if(ret < 0)
    {
      wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
    else if(ret > 0)
    {
      wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), _("Info"), wxICON_INFORMATION);
    }
    else
    {
      wxMessageBox( _("This config seems OK!\n"), _("Info"), wxICON_INFORMATION);
    }
}

void launcherFrame::OnMenuEditConfig(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-config -f \"");
  command.Append(ChoiceConfig->GetStringSelection());
  command.Append(wxT("\""));

  if (!wxExecute(command, wxEXEC_ASYNC))
  {
    wxMessageBox(_("Error editing the config file!"), _("Error"),
        wxICON_ERROR);
  }
}

void launcherFrame::OnMenuEditFpsConfig(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-fpsconfig -f \"");
  command.Append(ChoiceConfig->GetStringSelection());
  command.Append(wxT("\""));

  if (!wxExecute(command, wxEXEC_ASYNC))
  {
    wxMessageBox(_("Error editing the config file!"), _("Error"),
        wxICON_ERROR);
  }
}

void launcherFrame::refresh()
{
    readConfigs();
    if(ControllerType->GetStringSelection() == _("DIY USB"))
    {
      readSerialPorts();
      if(ChoiceOutput->IsEmpty())
      {
          wxMessageBox( _("No Serial Port Detected!\n"), _("Error"), wxICON_ERROR);
      }
    }
}

void launcherFrame::refreshGui()
{
  IOSizer->Layout();
  Panel1->Fit();
  Fit();
  Refresh();
}

void launcherFrame::OnMenuRefresh(wxCommandEvent& event)
{
    refresh();
}

void launcherFrame::OnControllerTypeSelect(wxCommandEvent& event)
{
    if(ControllerType->GetStringSelection() == _("GPP/Cronus"))
    {
      OutputSizer->Show(false);
    }
    else if(ControllerType->GetStringSelection() == _("DIY USB"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(false);
      OutputText->SetLabel(_("Port"));

      readSerialPorts();

      refreshGui();

      if(ChoiceOutput->IsEmpty())
      {
          wxMessageBox( _("No Serial Port Detected!\n"), _("Error"), wxICON_ERROR);
      }
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS3"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("Pairing"));

      readChoices(PS3_PAIRINGS, ChoiceOutput);

      refreshGui();

      if(ChoiceOutput->IsEmpty())
      {
        int answer = wxMessageBox(_("No pairing found.\nPerform a new pairing?"), _("Confirm"), wxYES_NO);
        if (answer == wxYES)
        {
          ps3Setup();
        }
      }
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("Pairing"));

      readChoices(PS4_PAIRINGS, ChoiceOutput);

      refreshGui();

      if(ChoiceOutput->IsEmpty())
      {
        int answer = wxMessageBox(_("No pairing found.\nPerform a new pairing?"), _("Confirm"), wxYES_NO);
        if (answer == wxYES)
        {
          ps4Setup();
        }
      }
    }
    else if(ControllerType->GetStringSelection() == _("Remote GIMX"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("IP:port"));

      readChoices(IP_DESTS, ChoiceOutput);

      refreshGui();

      if(ChoiceOutput->IsEmpty())
      {
        readIp(ChoiceOutput);
      }
    }

    refreshGui();
}

void launcherFrame::OnMenuUpdate(wxCommandEvent& event)
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
  else if(started)
  {
    wxMessageBox(_("GIMX is up-to-date!"), _("Info"), wxICON_INFORMATION);
  }
}

void launcherFrame::OnMenuStartupUpdates(wxCommandEvent& event)
{
  string filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append("/startUpdates");
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

void launcherFrame::OnMenuGetConfigs(wxCommandEvent& event)
{
  string dir = string(gimxConfigDir.mb_str(wxConvUTF8));

  configupdater* u = configupdater::getInstance();
  u->SetParams(CONFIGS_URL, CONFIGS_FILE, dir);

  list<string>* cl;
  list<string> cl_sel;

  {
    wxBusyInfo wait(_("Downloading config list..."));
    cl = u->getconfiglist();
  }

  if(cl && !cl->empty())
  {
    wxArrayString choices;

    for(list<string>::iterator it = cl->begin(); it != cl->end(); ++it)
    {
      choices.Add(wxString(it->c_str(), wxConvUTF8));
    }

    wxMultiChoiceDialog dialog(this, _("Select the files to download."), _("Config download"), choices);

    if (dialog.ShowModal() == wxID_OK)
    {
      wxArrayInt selections = dialog.GetSelections();
      wxArrayString configs;

      for ( size_t n = 0; n < selections.GetCount(); n++ )
      {
        string sel = string(choices[selections[n]].mb_str(wxConvUTF8));
        wxString wxfile = wxString(dir.c_str(), wxConvUTF8) + choices[selections[n]];
        if (::wxFileExists(wxfile))
        {
          int answer = wxMessageBox(_("Overwrite local file: ") + choices[selections[n]] + _("?"), _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            continue;
          }
        }
        cl_sel.push_back(sel);
        configs.Add(choices[selections[n]]);
      }

      {
        wxBusyInfo wait(_("Downloading configs..."));
        if(u->getconfigs(&cl_sel) < 0)
        {
          wxMessageBox(_("Can't retrieve configs!"), _("Error"), wxICON_ERROR);
          return;
        }
      }

      if(!cl_sel.empty())
	    {
	      wxMessageBox(_("Download is complete!"), _("Info"), wxICON_INFORMATION);
	      if(!ChoiceConfig->IsEmpty())
	      {
	        int answer = wxMessageBox(_("Auto-bind and convert?"), _("Confirm"), wxYES_NO);
          if (answer == wxYES)
          {
            autoBindControls(configs);
          }
	      }
        readConfigs();
        ChoiceConfig->SetSelection(ChoiceConfig->FindString(wxString(cl_sel.front().c_str(), wxConvUTF8)));
      }
    }
  }
  else
  {
    wxMessageBox(_("Can't retrieve config list!"), _("Error"), wxICON_ERROR);
    return;
  }
}

void launcherFrame::autoBindControls(wxArrayString configs)
{
  string dir = string(gimxConfigDir.mb_str(wxConvUTF8));

  wxString mod_config;

  wxArrayString ref_configs;
  for(unsigned int i=0; i<ChoiceConfig->GetCount(); i++)
  {
    ref_configs.Add(ChoiceConfig->GetString(i));
  }

  wxSingleChoiceDialog dialog(this, _("Select the reference config."), _("Auto-bind and convert"), ref_configs);

  if (dialog.ShowModal() == wxID_OK)
  {
    for(unsigned int j=0; j<configs.GetCount(); ++j)
    {
      ConfigurationFile configFile;
      mod_config = configs[j];

      int ret = configFile.ReadConfigFile(dir + string(mod_config.mb_str(wxConvUTF8)));

      if(ret < 0)
      {
        wxMessageBox(_("Can't read config: ") + mod_config + wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
        return;
      }

      if(configFile.AutoBind(dir + string(dialog.GetStringSelection().mb_str(wxConvUTF8))) < 0)
      {
        wxMessageBox(_("Can't auto-bind controls for config: ") + mod_config, _("Error"), wxICON_ERROR);
      }
      else
      {
        configFile.ConvertSensitivity(dir + string(dialog.GetStringSelection().mb_str(wxConvUTF8)));
        if(configFile.WriteConfigFile() < 0)
        {
          wxMessageBox(_("Can't write config: ") + mod_config, _("Error"), wxICON_ERROR);
        }
        else
        {
          wxMessageBox(_("Auto bind done for ") + mod_config, _("Info"), wxICON_INFORMATION);
        }
      }
    }
  }
}

void launcherFrame::OnMenuAutoBindControls(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxArrayString configs;
  configs.Add(ChoiceConfig->GetStringSelection());

  autoBindControls(configs);
}

void launcherFrame::OnMenuOpenConfigDirectory(wxCommandEvent& event)
{
#ifdef WIN32
  gimxConfigDir.Replace(wxT("/"), wxT("\\"));
  wxExecute(wxT("explorer ") + gimxConfigDir, wxEXEC_ASYNC, NULL);
#else
  wxExecute(wxT("xdg-open ") + gimxConfigDir, wxEXEC_ASYNC, NULL);
#endif
}

void launcherFrame::OnsourceChoiceSelect(wxCommandEvent& event)
{
  if(sourceChoice->GetStringSelection() == _("Network"))
  {
    MouseSizer->Show(false);
    SourceIpSizer->Show(true);

    readChoices(IP_SOURCES, ChoiceInput);

    refreshGui();

    if(ChoiceInput->IsEmpty())
    {
      readIp(ChoiceInput);
    }
  }
  else
  {
    MouseSizer->Show(true);
    SourceIpSizer->Show(false);
  }

  refreshGui();
}

int launcherFrame::choosePairing(BluetoothPairing& pairing)
{
  vector<BluetoothPairing> bluetoothPairings;
  if(ControllerType->GetStringSelection() == _("Bluetooth / PS3"))
  {
    readPairings(bluetoothPairings, wxT("sixaddr"));
  }
  else if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
  {
    readPairings(bluetoothPairings, wxT("ds4tool"));
  }

  if(bluetoothPairings.empty())
  {
      wxMessageBox( _("No controller Detected!\nPlug a controller with a USB cable."), _("Error"), wxICON_ERROR);
      return -1;
  }

  wxArrayString addresses;

  for(unsigned int i=0; i<bluetoothPairings.size(); ++i)
  {
    wxString address = wxEmptyString;
    address.Append(_("Controller: ")).Append(bluetoothPairings[i].controller).Append(wxT("\n"));
    address.Append(_("Console: ")).Append(bluetoothPairings[i].console);
    addresses.Add(address);
  }

  wxSingleChoiceDialog dialogSixaxis(this, _("Select the pairing."), _("PS Tool"), addresses);
  dialogSixaxis.SetSelection(0);
  if (dialogSixaxis.ShowModal() != wxID_OK)
  {
    return -1;
  }
    
  pairing = bluetoothPairings[dialogSixaxis.GetSelection()];
  
  return 0;
}

/*
 * Choose a bt dongle:
 * -if address is not empty
 *   -if a dongle address matches
 *   -else ask the user to choose a dongle and set its address
 * -else
 *   -ask the user to choose a dongle
 * In any case, if a dongle is successfully selected, dongleInfo is set.
 */
int launcherFrame::chooseDongle(wxString address, DongleInfo& dongleInfo)
{
  vector<DongleInfo> dongleInfos;
  readDongles(dongleInfos);

  if(dongleInfos.empty())
  {
      wxMessageBox( _("No Bluetooth Dongle Detected!"), _("Error"), wxICON_ERROR);
      return -1;
  }
  
  if(!address.IsEmpty())
  {
    for(unsigned int i=0; i<dongleInfos.size(); ++i)
    {
      if(dongleInfos[i].address == address)
      {
        dongleInfo = dongleInfos[i];
        return 0;
      }
    }
    
    if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
    {
      wxMessageBox( _("Dongle not found!"), _("Error"), wxICON_ERROR);
      return -1;
    }
  }

  wxArrayString dongles;

  for(unsigned int i=0; i<dongleInfos.size(); ++i)
  {
    wxString dongle = wxEmptyString;
    dongle.Append(_("Device: ")).Append(dongleInfos[i].hci).Append(wxT("\n"));
    dongle.Append(_("Address: ")).Append(dongleInfos[i].address).Append(wxT("\n"));
    dongle.Append(_("Manufacturer: ")).Append(dongleInfos[i].manufacturer);

    if(!dongleInfos[i].chip.IsEmpty())
    {
      dongle.Append(wxT("\n")).Append(_("Chip: ")).Append(dongleInfos[i].chip);
    }

    dongles.Add(dongle);
  }

  wxSingleChoiceDialog dialogDongles(this, _("Select the bluetooth adapter."), _("PS Tool"), dongles);
  dialogDongles.SetSelection(0);
  if (dialogDongles.ShowModal() != wxID_OK)
  {
    return -1;
  }
  
  int dongleIndex = dialogDongles.GetSelection();
  dongleInfo = dongleInfos[dongleIndex];

  if(!address.IsEmpty())
  {
    if(setDongleAddress(dongleInfos, dongleIndex, address) != -1)
    {
      dongleInfo.address = address;
    }
    else
    {
      return -1;
    }
  }
  
  return 0;
}

int launcherFrame::ps3Setup()
{
  BluetoothPairing btPairing;

  if(choosePairing(btPairing) < 0)
  {
    return -1;
  }
  
  DongleInfo dongleInfo;

  if(chooseDongle(btPairing.controller, dongleInfo) < 0)
  {
    return -1;
  }
  
  wxString pairing = btPairing.controller + wxT(" ") + btPairing.console;
  int pos = ChoiceOutput->FindString(pairing);
  if(pos == wxNOT_FOUND)
  {
    ChoiceOutput->SetSelection(ChoiceOutput->Append(pairing));
  }
  else
  {
    ChoiceOutput->SetSelection(pos);
  }

  return 0;
}

wxString launcherFrame::generateLinkKey()
{
  wxString ds4LinkKey;

  srand(time(NULL));
  for(unsigned int i=0; i<16; i++)
  {
    unsigned char byte = rand();
    ds4LinkKey.Append(wxString::Format(wxT("%02x"), byte));
  }

  return ds4LinkKey;
}

int launcherFrame::ps4Setup()
{
  wxString command;
  DongleInfo dongleInfo;
  BluetoothPairing btPairing;
  vector<BluetoothPairing> pairings;
  wxString ds4LinkKey;
  wxString ps4LinkKey;
  
  //choose a bluetooth device

  while(chooseDongle(wxEmptyString, dongleInfo) < 0)
  {
    int answer = wxMessageBox( _("Plug a bluetooth dongle"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }
  }
  
  //choose a ds4

  while(choosePairing(btPairing) < 0)
  {
    int answer = wxMessageBox( _("Plug a ds4"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }
  }
  
  //loop until the teensy is plugged

  do
  {
    pairings.clear();
    readPairings(pairings, wxT("ds4tool -t"));

    if(!pairings.empty())
    {
      break;
    }

    int answer = wxMessageBox( _("Plug the teensy"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }
  } while(1);
  
  //set teensy slave & master address (& reset link key)
  
  command.Clear();
  command.Append(wxT("ds4tool -t -s "));
  command.Append(dongleInfo.address);
  command.Append(wxT(" -m 00:00:00:00:00:00"));
  if(wxExecute(command, wxEXEC_SYNC))
  {
    wxMessageBox( _("Cannot execute: ") + command, _("Error"), wxICON_ERROR);
    return -1;
  }
  
  //loop until the teensy is paired with the PS4

  do
  {
    int answer = wxMessageBox( _("Plug the Teensy to the PS4,\nwait a few seconds,\nand plug it back to the PC"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }

    pairings.clear();
    readPairings(pairings, wxT("ds4tool -t"));

  } while(pairings.empty() || pairings[0].linkKey == wxT("00000000000000000000000000000000"));
  
  ps4LinkKey = pairings[0].linkKey;

  ds4LinkKey = generateLinkKey();

  command.Clear();
  command.Append(wxT("gksudo -- bash -c \"gimx-ps4setup.sh "));
  command.Append(dongleInfo.hci);
  command.Append(wxT(" "));
  command.Append(dongleInfo.address);
  command.Append(wxT(" "));
  command.Append(btPairing.controller);
  command.Append(wxT(" "));
  command.Append(ds4LinkKey);
  command.Append(wxT(" "));
  command.Append(btPairing.console);
  command.Append(wxT(" "));
  command.Append(ps4LinkKey);
  command.Append(wxT("\""));
  if(wxExecute(command, wxEXEC_SYNC ))
  {
    wxMessageBox( _("Cannot execute: ") + command, _("Error"), wxICON_ERROR);
    return -1;
  }

  //set the master and the link key of the ds4

  command.Clear();
  command.Append(wxT("ds4tool -m "));
  command.Append(dongleInfo.address);
  command.Append(wxT(" -l "));
  command.Append(ds4LinkKey);
  if(wxExecute(command, wxEXEC_SYNC))
  {
    wxMessageBox( _("Cannot execute: ") + command, _("Error"), wxICON_ERROR);
    return -1;
  }

  wxString pairing = dongleInfo.address + wxT(" ") + btPairing.console;
  int pos = ChoiceOutput->FindString(pairing);
  if(pos == wxNOT_FOUND)
  {
    ChoiceOutput->SetSelection(ChoiceOutput->Append(pairing));
  }
  else
  {
    ChoiceOutput->SetSelection(pos);
  }

  return -1;
}

void launcherFrame::OnMenuSave(wxCommandEvent& event)
{
    saveChoices(OUTPUT_FILE, ControllerType);

    if(ControllerType->GetStringSelection() == _("DIY USB"))
    {
      saveChoices(PORT_FILE, ChoiceOutput);
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS3"))
    {
      saveChoices(PS3_PAIRINGS, ChoiceOutput);
    }
    else if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
    {
      saveChoices(PS4_PAIRINGS, ChoiceOutput);
    }
    else if(ControllerType->GetStringSelection() == _("Remote GIMX"))
    {
      saveChoices(IP_DESTS, ChoiceOutput);
    }

    saveChoices(INPUT_FILE, sourceChoice);
    if(sourceChoice->GetStringSelection() == _("Network"))
    {
      saveChoices(IP_SOURCES, ChoiceInput);
    }
}

void launcherFrame::readIp(wxChoice* choices)
{
  wxTextEntryDialog dialogIp(this, _("Enter an IP address"), _("Remote GIMX"), wxT("127.0.0.1"));

  if(dialogIp.ShowModal() == wxID_OK)
  {
    wxString text = dialogIp.GetValue();

    unsigned int ip = inet_addr(text.mb_str(wxConvUTF8));

    if (ip != INADDR_NONE)
    {
      wxNumberEntryDialog dialogPort(this, wxT(""), _("Enter a port"), _("Remote GIMX"), 51914, 0, 65535);

      if(dialogPort.ShowModal() == wxID_OK)
      {
          int port = dialogPort.GetValue();

          text.Append(wxT(":"));
          text.Append(wxString::Format(wxT("%i"), port));

          int pos = choices->FindString(text);

          if(pos != wxNOT_FOUND)
          {
            choices->SetSelection(pos);
          }
          else
          {
            choices->SetSelection(choices->Append(text));
          }
      }
    }
    else
    {
      wxMessageBox( _("Bad IP format!"), _("Error"), wxICON_ERROR);
    }
  }
}

void launcherFrame::OnOutputNewButtonClick(wxCommandEvent& event)
{
  if(ControllerType->GetStringSelection() == _("Bluetooth / PS3"))
  {
    ps3Setup();
  }
  else if(ControllerType->GetStringSelection() == _("Bluetooth / PS4"))
  {
    ps4Setup();
  }
  else if(ControllerType->GetStringSelection() == _("Remote GIMX"))
  {
    readIp(ChoiceOutput);
  }

  refreshGui();
}

void launcherFrame::OnInputNewButtonClick(wxCommandEvent& event)
{
  readIp(ChoiceInput);

  refreshGui();
}
