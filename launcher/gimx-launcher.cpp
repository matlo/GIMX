/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "gimx-launcher.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(launcherFrame)
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
#ifndef WIN32
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
#include "../shared/gpp/pcprog.h"
#include <ConfigurationFile.h>

#include <wx/arrstr.h>
#include <wx/stdpaths.h>
#include <wx/busyinfo.h>
#include "wx/numdlg.h"

#include <time.h>
#include "../shared/async/include/hidasync.h"
#include "../shared/async/include/serialasync.h"

using namespace std;

#ifdef WIN32
#define MAX_PORT_ID 32
#endif

#define BLUETOOTH_DIR "/.sixemugui"

#define OUTPUT_FILE "/output"
#define OUTPUT_CHOICE_FILE "/outputChoice"

#define INPUT_FILE "/input"
#define INPUT_CHOICE_FILE "/inputChoice"

#define PS3_PAIRINGS "/PS3Pairings"
#define PS4_PAIRINGS "/PS4Pairings"

#define IP_DESTS "/IPDests"
#define IP_SOURCES "/IPSources"

#define START_UPDATES "/startUpdates"

#define BLUETOOTH_LK_DIR "/bluetooth"
#define BLUETOOTH_LK_FILE "/linkkeys"

#define LOG_FILE "log.txt"

#define GPP_NAME "GPP/Cronus/Titan"

#define CONSOLETUNER_VID 0x2508

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
const long launcherFrame::ID_STATICTEXT5 = wxNewId();
const long launcherFrame::ID_CHOICE4 = wxNewId();
const long launcherFrame::ID_STATICTEXT6 = wxNewId();
const long launcherFrame::ID_CHECKBOX1 = wxNewId();
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
      if (res != -1 && !results2[0].IsEmpty() && results2[0][0] != wxT("Unknown"))
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
  char portname[16];
  int i;
  wxString previous = OutputChoice->GetStringSelection();

  OutputChoice->Clear();

  for(i=0; i<MAX_PORT_ID; ++i)
  {
    snprintf(portname, sizeof(portname), "COM%d", i);
    int device = serialasync_open(portname, 500000);
    if(device >= 0) {
      OutputChoice->SetSelection(OutputChoice->Append(wxString(portname, wxConvUTF8)));
      serialasync_close(device);
    }
  }

  if(previous != wxEmptyString)
  {
    OutputChoice->SetSelection(OutputChoice->FindString(previous));
  }
  if(OutputChoice->GetSelection() < 0)
  {
    OutputChoice->SetSelection(0);
  }
}
#else
void launcherFrame::readSerialPorts()
{
  string filename;
  string line = "";
  wxString previous = OutputChoice->GetStringSelection();

  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append(OUTPUT_CHOICE_FILE);
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
      if( infile.good() )
      {
          getline (infile,line);
      }
      infile.close();
  }

  OutputChoice->Clear();

  string ds = "/dev";

  wxDir dir(wxString(ds.c_str(), wxConvUTF8));

  if(!dir.IsOpened())
  {
    wxMessageBox( _("Cannot open directory: ") + wxString(ds.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    return;
  }

  wxString file;
  wxString filespec = wxT("*");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    if(file.StartsWith(wxT("ttyUSB")) || file.StartsWith(wxT("ttyACM")) || file.StartsWith(wxT("ttyAMA")))
    {
      if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
      {
        OutputChoice->SetSelection(OutputChoice->Append(file));
      }
      else
      {
        OutputChoice->Append(file);
      }
    }
  }

  if(previous != wxEmptyString)
  {
    OutputChoice->SetSelection(OutputChoice->FindString(previous));
  }
  if(OutputChoice->GetSelection() < 0)
  {
    OutputChoice->SetSelection(0);
  }
}
#endif

void launcherFrame::readHidPorts()
{
  s_hid_dev *devs, *cur_dev;

  wxString previous = OutputChoice->GetStringSelection();

  OutputChoice->Clear();
  hids.Clear();

  unsigned int nb_usb_ids;
  const GCAPI_USB_IDS * usb_ids = gpppcprog_get_ids(&nb_usb_ids);

  devs = hidasync_enumerate(0x0000, 0x0000);
  for(cur_dev = devs; cur_dev != NULL; ++cur_dev)
  {
    wxString device;
    for(unsigned int i = 0; i < nb_usb_ids; ++i)
    {
      if(usb_ids[i].vid == cur_dev->vendor_id && usb_ids[i].pid == cur_dev->product_id)
      {
        device.append(wxString(usb_ids[i].name, wxConvUTF8));
        device.append(wxT(" ("));
        break;
      }
    }
    if(!device.IsEmpty())
    {
      device.append(wxString::Format(wxT("%i"),hids.GetCount()));
      hids.Add(wxString(cur_dev->path, wxConvUTF8));
      device.append(wxT(")"));
      OutputChoice->SetSelection(OutputChoice->Append(device));
    }
    if(cur_dev->next == 0) {
        break;
    }
  }
  hidasync_free_enumeration(devs);

  if(previous != wxEmptyString)
  {
    OutputChoice->SetSelection(OutputChoice->FindString(previous));
  }
  if(OutputChoice->GetSelection() < 0)
  {
    OutputChoice->SetSelection(0);
  }
}

void launcherFrame::readConfigs()
{
  string filename;
  string line = "";
  wxString previous = InputChoice->GetStringSelection();

  /* Read the last config used so as to auto-select it. */
  filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append(INPUT_CHOICE_FILE);
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
    }
    infile.close();
  }

  InputChoice->Clear();

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
    InputChoice->Append(file);
  }

  if(previous != wxEmptyString)
  {
    InputChoice->SetSelection(InputChoice->FindString(previous));
  }
  if(InputChoice->GetSelection() < 0)
  {
    InputChoice->SetSelection(0);
  }
}

int launcherFrame::readChoices(const char* file, wxChoice* choices, const char* default_file)
{
    string filename;
    string line;
    string device;
    string master;
    int ret = -1;

    string defaultChoice = "";
    filename = string(launcherDir.mb_str(wxConvUTF8));
    filename.append(default_file);
    ifstream infile (filename.c_str());
    if ( infile.is_open() )
    {
      if( infile.good() )
      {
        getline (infile,defaultChoice);
      }
      infile.close();
    }

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
            if(!defaultChoice.empty() && defaultChoice == line)
            {
                choices->SetSelection(choices->Append(wxString(line.c_str(), wxConvUTF8)));
                ret = 0;
            }
            else if(!line.empty())
            {
                choices->Append(wxString(line.c_str(), wxConvUTF8));
                ret = 0;
            }
        }
        if(choices->GetSelection() < 0)
        {
          choices->SetSelection(0);
        }
        myfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }

    return ret;
}

int launcherFrame::saveParam(const char* file, wxString option)
{
  int ret = 0;
  string filename = string(launcherDir.mb_str(wxConvUTF8));
  filename.append(file);
  ofstream outfile (filename.c_str(), ios_base::trunc);
  if(outfile.is_open())
  {
      outfile << option.mb_str(wxConvUTF8) << endl;
      outfile.close();
  }
  else
  {
      wxMessageBox( _("Cannot open file for writing: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
      ret = -1;
  }
  return ret;
}

int launcherFrame::saveChoices(const char* file, wxChoice* choices)
{
    string filename;
    string line;
    string device;
    string master;
    int ret = 0;

    filename = string(launcherDir.mb_str(wxConvUTF8));
    filename.append(file);

    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        for(int i=0; i<(int)choices->GetCount(); i++)
        {
            outfile << choices->GetString(i).mb_str(wxConvUTF8) << endl;
        }
        outfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file for writing: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
        ret = -1;
    }
    return ret;
}

int launcherFrame::saveLinkKeys(wxString dongleBdaddr, wxString ds4Bdaddr, wxString ds4LinkKey, wxString ps4Bdaddr, wxString ps4LinkKey)
{
  string filename;
  string line;
  string device;
  string master;
  int ret = 0;

  filename = string(gimxDir.mb_str(wxConvUTF8));
  filename.append(BLUETOOTH_LK_DIR);

  wxString btDir = wxString(filename.c_str(), wxConvUTF8);

  if(!wxDir::Exists(btDir))
  {
    if(!wxMkdir(btDir))
    {
      wxMessageBox( _("Can't init directory: ") + btDir, _("Error"), wxICON_ERROR);
      return -1;
    }
  }

  filename.append("/");
  filename.append(dongleBdaddr.mb_str(wxConvUTF8));

  wxString lkDir = wxString(filename.c_str(), wxConvUTF8);

  if(!wxDir::Exists(lkDir))
  {
    if(!wxMkdir(lkDir))
    {
      wxMessageBox( _("Can't init directory: ") + lkDir, _("Error"), wxICON_ERROR);
      return -1;
    }
  }

  filename.append(BLUETOOTH_LK_FILE);

  ofstream outfile (filename.c_str(), ios_base::trunc);
  if(outfile.is_open())
  {
      outfile << ds4Bdaddr.mb_str(wxConvUTF8) << " " << ds4LinkKey.mb_str(wxConvUTF8) << " 4 0" << endl;
      outfile << ps4Bdaddr.mb_str(wxConvUTF8) << " " << ps4LinkKey.mb_str(wxConvUTF8) << " 4 0" << endl;
      outfile.close();
  }
  else
  {
      wxMessageBox( _("Cannot open file for writing: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
      ret = -1;
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
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;
    wxFlexGridSizer* FlexGridSizer5;

    Create(parent, wxID_ANY, _("Gimx-launcher"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
    IOSizer = new wxFlexGridSizer(4, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Output = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(Output, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OutputSizer = new wxFlexGridSizer(1, 3, 0, 0);
    OutputText = new wxStaticText(Panel1, ID_STATICTEXT3, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    OutputSizer->Add(OutputText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OutputChoice = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    OutputSizer->Add(OutputChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OutputNewButton = new wxButton(Panel1, ID_BUTTON2, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    OutputSizer->Add(OutputNewButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(OutputSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Input"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Input = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Input->SetSelection( Input->Append(_("Physical devices")) );
    Input->Append(_("Window events"));
    Input->Append(_("Network"));
    FlexGridSizer5->Add(Input, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SourceIpSizer = new wxFlexGridSizer(1, 3, 0, 0);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("IP:port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    SourceIpSizer->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    InputChoice = new wxChoice(Panel1, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE5"));
    SourceIpSizer->Add(InputChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(Panel1, ID_BUTTON4, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    SourceIpSizer->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    IOSizer->Add(SourceIpSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(IOSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT5, _("Messages"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ProcessOutputChoice = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    ProcessOutputChoice->SetSelection( ProcessOutputChoice->Append(_("none")) );
    ProcessOutputChoice->Append(_("curses"));
    ProcessOutputChoice->Append(_("text"));
    ProcessOutputChoice->Append(_("log file"));
    FlexGridSizer3->Add(ProcessOutputChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MouseSizer = new wxFlexGridSizer(1, 2, 0, 0);
    StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT6, _("Grab mouse"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    MouseSizer->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxGrab = new wxCheckBox(Panel1, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxGrab->SetValue(true);
    MouseSizer->Add(CheckBoxGrab, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(MouseSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(1, 2, 0, 0);
    ButtonCheck = new wxButton(Panel1, ID_BUTTON1, _("Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer7->Add(ButtonCheck, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStart = new wxButton(Panel1, ID_BUTTON3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(ButtonStart, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&launcherFrame::OnOutputSelect);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnOutputNewButtonClick);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&launcherFrame::OnInputSelect);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&launcherFrame::OnInputNewButtonClick);
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

    /*if(SingleInstanceChecker1.IsAnotherRunning())
    {
        wxMessageBox( _("gimx-launcher is already running!"), _("Error"), wxICON_ERROR);
        exit(-1);
    }*/

    launcherDir = wxStandardPaths::Get().GetUserDataDir();
    if(!wxDir::Exists(launcherDir))
    {
      if(!wxMkdir(launcherDir))
      {
        wxMessageBox( _("Can't init directory: ") + launcherDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    userDir = wxStandardPaths::Get().GetUserConfigDir();

    //migrate old config file from gimx-bluetooth
    wxString oldConfig = userDir + wxT(BLUETOOTH_DIR) + wxT("/config");
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
    wxString oldGimxDir = userDir + wxT(OLD_GIMX_DIR);
    gimxDir = userDir + wxT(GIMX_DIR);
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
    gimxLogDir = gimxDir + wxT(LOG_DIR);
    if(!wxDir::Exists(gimxLogDir))
    {
      if(!wxMkdir(gimxLogDir))
      {
        wxMessageBox( _("Can't init directory: ") + gimxLogDir, _("Error"), wxICON_ERROR);
        exit(-1);
      }
    }

    gpppcprog_read_user_ids(userDir.mb_str(wxConvUTF8), GIMX_DIR);

    started = false;

#ifndef WIN32
    Output->SetSelection( Output->Append(_("Bluetooth / PS3")) );
    Output->Append(_("Bluetooth / PS4"));
    Output->Append(_("DIY USB"));
    Output->Append(_(GPP_NAME));
    Output->Append(_("Remote GIMX"));
#else
    Output->SetSelection( Output->Append(_("DIY USB")) );
    Output->Append(_(GPP_NAME));
    Output->Append(_("Remote GIMX"));
#endif

    readParam(OUTPUT_FILE, Output);
    readParam(INPUT_FILE, Input);

    wxCommandEvent event;
    OnOutputSelect(event);
    OnInputSelect(event);

    readStartUpdates();
    if(MenuStartupUpdates->IsChecked())
    {
      OnMenuUpdate(event);
    }

    started = true;

    if(InputChoice->IsEmpty())
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
  info.SetVersion(wxT(INFO_VERSION) + wxString(wxT(" ")) + wxString(wxT(INFO_ARCH)));
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

    wxString outputSelection = OutputChoice->GetStringSelection();

    if(InputChoice->GetStringSelection().IsEmpty())
    {
      wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
      return;
    }

    if(Output->GetStringSelection() == _(GPP_NAME))
    {
      if(outputSelection.IsEmpty())
      {
        wxMessageBox( _("No device selected!"), _("Error"), wxICON_ERROR);
        return;
      }
    }
    else if(Output->GetStringSelection() == _("DIY USB"))
    {
      if(outputSelection.IsEmpty())
      {
        wxMessageBox( _("No USB to serial device selected!"), _("Error"), wxICON_ERROR);
        return;
      }
    }
    else if(Output->GetStringSelection() == _("Remote GIMX"))
    {
      if(outputSelection.IsEmpty())
      {
        wxMessageBox( _("No destination IP:port specified!"), _("Error"), wxICON_ERROR);
        return;
      }
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS3")
         || Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
      if(outputSelection.IsEmpty())
      {
        wxMessageBox( _("No pairing selected!"), _("Error"), wxICON_ERROR);
        return;
      }

      int pos = outputSelection.Find(wxT(" "));

      if(pos == wxNOT_FOUND)
      {
        wxMessageBox( _("Selected pairing is incorrect!"), _("Error"), wxICON_ERROR);
        return;
      }

      wxString bdaddrSrc = outputSelection.BeforeFirst(wxChar(' '));
      bdaddrDst = outputSelection.AfterFirst(wxChar(' '));

      DongleInfo dongleInfo;

      if(chooseDongle(bdaddrSrc, dongleInfo) < 0)
      {
        wxMessageBox( _("Dongle not found!"), _("Error"), wxICON_ERROR);
        return;
      }

      hciIndex = dongleInfo.hci.Mid(3);
    }

    if(Input->GetStringSelection() == _("Network"))
    {
      if(InputChoice->GetStringSelection().IsEmpty())
      {
        wxMessageBox( _("No source IP:port specified!"), _("Error"), wxICON_ERROR);
        return;
      }
    }

    if(Output->GetStringSelection() == _("Remote GIMX")
    && Input->GetStringSelection() == _("Network"))
    {
      if(InputChoice->GetStringSelection() == outputSelection)
      {
        wxMessageBox( _("IP:port is the same for input and output!"), _("Error"), wxICON_ERROR);
        return;
      }
    }

    if(Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
      wxString checkCommand = wxT("pgrep bluetoothd");

      if(!wxExecute(checkCommand, wxEXEC_SYNC))
      {
        int answer = wxMessageBox(_("Bluetooth service has to be stopped.\nProceed?"), _("Confirm"), wxYES_NO | wxCANCEL);

        if (answer != wxYES)
        {
          return;
        }

        wxString stopService = wxT("gksudo --message \"stop bluetooth service\" -- bash -c \"service bluetooth stop\"");

        if(wxExecute(stopService, wxEXEC_SYNC))
        {
          wxMessageBox( _("Can't stop bluetoothd!"), _("Error"), wxICON_ERROR);
          return;
        }
      }
    }

#ifndef WIN32
    command.Append(wxT("xterm -e "));
#endif
    command.Append(wxT("gimx"));

    if(ProcessOutputChoice->GetStringSelection() == _("curses"))
    {
      command.Append(wxT(" --curses"));
    }
    else if(ProcessOutputChoice->GetStringSelection() == _("text"))
    {
      command.Append(wxT(" --status"));
    }
    else if(ProcessOutputChoice->GetStringSelection() == _("log file"))
    {
      command.Append(wxT(" --status --log "));
      command.Append(wxT(LOG_FILE));
    }

    if(Input->GetStringSelection() == _("Network"))
    {
      command.Append(wxT(" --src "));
      command.Append(InputChoice->GetStringSelection());
      command.Append(wxT(" --nograb"));
    }
    else
    {
      if(Input->GetStringSelection() == _("Window events"))
      {
        command.Append(wxT(" --window-events"));
      }

      if(!CheckBoxGrab->IsChecked())
      {
          command.Append(wxT(" --nograb"));
      }
      command.Append(wxT(" --config \""));
      command.Append(InputChoice->GetStringSelection());
      command.Append(wxT("\""));

      if(Output->GetStringSelection() != _("Remote GIMX"))
      {
        command.Append(wxT(" --force-updates"));
      }
      command.Append(wxT(" --subpos"));
    }

    if(Output->GetStringSelection() == _("Remote GIMX"))
    {
      command.Append(wxT(" --dst "));
      command.Append(outputSelection);
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS3"))
    {
      command.Append(wxT(" --type Sixaxis"));

      command.Append(wxT(" --hci "));
      command.Append(hciIndex);

      command.Append(wxT(" --bdaddr "));
      command.Append(bdaddrDst);
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
      command.Append(wxT(" --type DS4"));

      command.Append(wxT(" --hci "));
      command.Append(hciIndex);

      command.Append(wxT(" --bdaddr "));
      command.Append(bdaddrDst);
    }
    else if(Output->GetStringSelection() == _("DIY USB"))
    {
      command.Append(wxT(" --port "));
#ifndef WIN32
      command.Append(wxT("/dev/"));
#endif
      command.Append(outputSelection);
    }
    else if(Output->GetStringSelection() == _(GPP_NAME))
    {
      command.Append(wxT(" --port "));
      wxString path = wxT("\"") + hids[OutputChoice->GetSelection()] + wxT("\"");
      command.Append(path);
    }

    StatusBar1->SetStatusText(_("Press Shift+Esc to exit."));

    ButtonStart->Enable(false);

    //wxMessageBox( command, _("Error"), wxICON_ERROR);

    MyProcess *process = new MyProcess(this, command);

    if(!wxExecute(command, wxEXEC_ASYNC | wxEXEC_NOHIDE, process))
    {
      wxMessageBox( _("can't start gimx!"), _("Error"), wxICON_ERROR);
    }
}

void launcherFrame::OnProcessTerminated(wxProcess *process, int status)
{
    ButtonStart->Enable(true);
    StatusBar1->SetStatusText(wxEmptyString);

    if(status)
    {
      wxMessageBox( _("gimx error"), _("Error"), wxICON_ERROR);
    }
    else
    {
      //Save options when gimx successfully terminates.

      wxCommandEvent event;
      OnMenuSave(event);
    }

    if(ProcessOutputChoice->GetStringSelection() == _("log file"))
    {
#ifdef WIN32
      gimxLogDir.Replace(wxT("/"), wxT("\\"));
      wxExecute(wxT("explorer ") + gimxLogDir + wxT(LOG_FILE), wxEXEC_ASYNC, NULL);
#else
      wxExecute(wxT("xdg-open ") + gimxLogDir + wxT(LOG_FILE), wxEXEC_ASYNC, NULL);
#endif
    }

    SetFocus();
}

void launcherFrame::OnButtonCheckClick1(wxCommandEvent& event)
{
    if(InputChoice->GetStringSelection().IsEmpty())
    {
      wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
      return;
    }

    string file = string(gimxConfigDir.mb_str(wxConvUTF8));
    file.append(InputChoice->GetStringSelection().mb_str(wxConvUTF8));

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
  if(InputChoice->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-config -f \"");
  command.Append(InputChoice->GetStringSelection());
  command.Append(wxT("\""));

  if (!wxExecute(command, wxEXEC_ASYNC))
  {
    wxMessageBox(_("Error editing the config file!"), _("Error"),
        wxICON_ERROR);
  }
}

void launcherFrame::OnMenuEditFpsConfig(wxCommandEvent& event)
{
  if(InputChoice->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-fpsconfig -f \"");
  command.Append(InputChoice->GetStringSelection());
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
    if(Output->GetStringSelection() == _("DIY USB"))
    {
      readSerialPorts();
      if(OutputChoice->IsEmpty())
      {
          wxMessageBox( _("No Serial Port Detected!\n"), _("Error"), wxICON_ERROR);
      }
    }
    else if(Output->GetStringSelection() == _(GPP_NAME))
    {
      readHidPorts();
      if(started && OutputChoice->IsEmpty())
      {
          wxMessageBox( _("No device detected!\n"), _("Error"), wxICON_ERROR);
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

void launcherFrame::OnOutputSelect(wxCommandEvent& event)
{
    if(Output->GetStringSelection() == _(GPP_NAME))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(false);
      OutputText->SetLabel(_("Device"));

      readHidPorts();

      refreshGui();

      if(started && OutputChoice->IsEmpty())
      {
          wxMessageBox( _("No device detected!\n"), _("Error"), wxICON_ERROR);
      }
    }
    else if(Output->GetStringSelection() == _("DIY USB"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(false);
      OutputText->SetLabel(_("Port"));

      readSerialPorts();

      refreshGui();

      if(started && OutputChoice->IsEmpty())
      {
          wxMessageBox( _("No Serial Port Detected!\n"), _("Error"), wxICON_ERROR);
      }
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS3"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("Pairing"));

      readChoices(PS3_PAIRINGS, OutputChoice, OUTPUT_CHOICE_FILE);

      refreshGui();

      if(started && OutputChoice->IsEmpty())
      {
        int answer = wxMessageBox(_("No pairing found.\nPerform a new pairing?"), _("Confirm"), wxYES_NO);
        if (answer == wxYES)
        {
          ps3Setup();
        }
      }
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("Pairing"));

      readChoices(PS4_PAIRINGS, OutputChoice, OUTPUT_CHOICE_FILE);

      refreshGui();

      if(started && OutputChoice->IsEmpty())
      {
        int answer = wxMessageBox(_("No pairing found.\nPerform a new pairing?"), _("Confirm"), wxYES_NO);
        if (answer == wxYES)
        {
          ps4Setup();
        }
      }
    }
    else if(Output->GetStringSelection() == _("Remote GIMX"))
    {
      OutputSizer->Show(true);
      OutputNewButton->Show(true);
      OutputText->SetLabel(_("IP:port"));

      readChoices(IP_DESTS, OutputChoice, OUTPUT_CHOICE_FILE);

      refreshGui();

      if(started && OutputChoice->IsEmpty())
      {
        readIp(OutputChoice);
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
  filename.append(START_UPDATES);
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
  u->setconfigdirectory(dir);

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
        if(!InputChoice->IsEmpty())
        {
          int answer = wxMessageBox(_("Auto-bind and convert?"), _("Confirm"), wxYES_NO);
          if (answer == wxYES)
          {
            autoBindControls(configs);
          }
        }
        readConfigs();
        InputChoice->SetSelection(InputChoice->FindString(wxString(cl_sel.front().c_str(), wxConvUTF8)));
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
  for(unsigned int i=0; i<InputChoice->GetCount(); i++)
  {
    ref_configs.Add(InputChoice->GetString(i));
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
  if(InputChoice->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxArrayString configs;
  configs.Add(InputChoice->GetStringSelection());

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

void launcherFrame::OnInputSelect(wxCommandEvent& event)
{
  if(Input->GetStringSelection() == _("Network"))
  {
    StaticText2->SetLabel(_("IP:port"));
    readChoices(IP_SOURCES, InputChoice, INPUT_CHOICE_FILE);
    Button1->Show(true);
    ButtonCheck->Show(false);
    MouseSizer->Show(false);

    refreshGui();

    if(InputChoice->IsEmpty())
    {
      readIp(InputChoice);
    }
  }
  else
  {
    StaticText2->SetLabel(_("Config"));
    readConfigs();
    Button1->Show(false);
    ButtonCheck->Show(true);
    MouseSizer->Show(true);
  }

  refreshGui();
}

int launcherFrame::choosePairing(BluetoothPairing& pairing)
{
  vector<BluetoothPairing> bluetoothPairings;
  if(Output->GetStringSelection() == _("Bluetooth / PS3"))
  {
    readPairings(bluetoothPairings, wxT("sixaddr"));
  }
  else if(Output->GetStringSelection() == _("Bluetooth / PS4"))
  {
    readPairings(bluetoothPairings, wxT("ds4tool"));
  }

  if(bluetoothPairings.empty())
  {
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

    if(Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
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

  //choose a ds4

  while(choosePairing(btPairing) < 0)
  {
    int answer = wxMessageBox( _("Plug a sixaxis/ds3"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }
  }

  DongleInfo dongleInfo;

  //choose a bluetooth device

  while(chooseDongle(btPairing.controller, dongleInfo) < 0)
  {
    int answer = wxMessageBox( _("Plug a bluetooth dongle"), _("PS Tool"), wxICON_INFORMATION | wxYES | wxCANCEL);
    if (answer != wxYES)
    {
      return -1;
    }
  }

  wxString pairing = btPairing.controller + wxT(" ") + btPairing.console;
  int pos = OutputChoice->FindString(pairing);
  if(pos == wxNOT_FOUND)
  {
    OutputChoice->SetSelection(OutputChoice->Append(pairing));
  }
  else
  {
    OutputChoice->SetSelection(pos);
  }

  if(saveChoices(PS3_PAIRINGS, OutputChoice) < 0)
  {
    return -1;
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
  wxString ds4Bdaddr;
  wxString ps4LinkKey;
  wxString ps4Bdaddr;

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

  ds4Bdaddr = btPairing.controller;

  ds4LinkKey = generateLinkKey();

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

  //set teensy slave address

  command.Clear();
  command.Append(wxT("ds4tool -t -s "));
  command.Append(dongleInfo.address);
  if(wxExecute(command, wxEXEC_SYNC))
  {
    wxMessageBox( _("Cannot execute: ") + command, _("Error"), wxICON_ERROR);
    return -1;
  }

  //blank master address & link key

  command.Clear();
  command.Append(wxT("ds4tool -t -m 00:00:00:00:00:00"));
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

  ps4Bdaddr = pairings[0].console;
  ps4LinkKey = pairings[0].linkKey;

  if(saveLinkKeys(dongleInfo.address, ds4Bdaddr, ds4LinkKey, ps4Bdaddr, ps4LinkKey) < 0)
  {
    return -1;
  }

  wxString pairing = dongleInfo.address + wxT(" ") + ps4Bdaddr;
  int pos = OutputChoice->FindString(pairing);
  if(pos == wxNOT_FOUND)
  {
    OutputChoice->SetSelection(OutputChoice->Append(pairing));
  }
  else
  {
    OutputChoice->SetSelection(pos);
  }

  if(saveChoices(PS4_PAIRINGS, OutputChoice) < 0)
  {
    return -1;
  }

  return 0;
}

void launcherFrame::OnMenuSave(wxCommandEvent& event)
{
    saveParam(OUTPUT_FILE, Output->GetStringSelection());
    saveParam(OUTPUT_CHOICE_FILE, OutputChoice->GetStringSelection());

    if(Output->GetStringSelection() == _("Bluetooth / PS3"))
    {
      saveChoices(PS3_PAIRINGS, OutputChoice);
    }
    else if(Output->GetStringSelection() == _("Bluetooth / PS4"))
    {
      saveChoices(PS4_PAIRINGS, OutputChoice);
    }
    else if(Output->GetStringSelection() == _("Remote GIMX"))
    {
      saveChoices(IP_DESTS, OutputChoice);
    }

    saveParam(INPUT_FILE, Input->GetStringSelection());
    saveParam(INPUT_CHOICE_FILE, InputChoice->GetStringSelection());

    if(Input->GetStringSelection() == _("Network"))
    {
      saveChoices(IP_SOURCES, InputChoice);
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
  if(Output->GetStringSelection() == _("Bluetooth / PS3"))
  {
    ps3Setup();
  }
  else if(Output->GetStringSelection() == _("Bluetooth / PS4"))
  {
    ps4Setup();
  }
  else if(Output->GetStringSelection() == _("Remote GIMX"))
  {
    readIp(OutputChoice);
  }

  refreshGui();
}

void launcherFrame::OnInputNewButtonClick(wxCommandEvent& event)
{
  readIp(InputChoice);

  refreshGui();
}
