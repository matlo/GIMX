/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef LAUNCHERMAIN_H
#define LAUNCHERMAIN_H

//(*Headers(launcherFrame)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

#include <wx/process.h>
#include <wx/progdlg.h>

#include <vector>

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>

using namespace std;

class DongleInfo
{
public:
    wxString hci;
    wxString address;
    wxString manufacturer;
    wxString chip;
};

class BluetoothPairing
{
public:
    wxString local;
    wxString remote;
    wxString linkkey;
};

class launcherFrame: public wxFrame
{
    public:

        launcherFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~launcherFrame();

        void OnProcessTerminated(wxProcess *process, int status);

        int OnUpdateProgress(Updater::UpdaterStatus status, double progress, double total);
        int OnUpdateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total);

    private:

        //(*Handlers(launcherFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonCheckClick(wxCommandEvent& event);
        void OnButtonStartClick(wxCommandEvent& event);
        void OnCheckBoxGrabClick(wxCommandEvent& event);
        void OnCheckBoxGuiClick(wxCommandEvent& event);
        void OnCheckBoxTerminalClick(wxCommandEvent& event);
        void OnChoice8Select(wxCommandEvent& event);
        void OnButtonCheckClick1(wxCommandEvent& event);
        void OnMenuEditConfig(wxCommandEvent& event);
        void OnMenuEditFpsConfig(wxCommandEvent& event);
        void OnMenuRefresh(wxCommandEvent& event);
        void OnOutputSelect(wxCommandEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuStartupUpdates(wxCommandEvent& event);
        void OnMenuGetConfigs(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        void OnMenuOpenConfigDirectory(wxCommandEvent& event);
        void OnInputSelect(wxCommandEvent& event);
        void OnMenuSave(wxCommandEvent& event);
        void OnOutputNewButtonClick(wxCommandEvent& event);
        void OnInputNewButtonClick(wxCommandEvent& event);
        void OnMenuUpdateFirmware(wxCommandEvent& event);
        void OnMenuOpenMacroDirectory(wxCommandEvent& event);
        //*)

        void refresh();
        void refreshGui();
        void autoBindControls(wxArrayString configs);

        void readHidPorts();
        void readSerialPorts();
        void readConfigs();
        
        void autoConfig();
        bool getConfig(const std::string& config);

        int readChoices(const char* file, wxChoice* choices, const char* default_file);
        int saveChoices(const char* file, wxChoice* choices);
        int saveLinkKeys(wxString dongleBdaddr, wxString ds4Bdaddr, wxString ds4LinkKey, wxString ps4Bdaddr, wxString ps4LinkKey);
        int saveParam(const char* file, wxString option);
        
        void readIp(wxChoice* choices);
        
        void readStartUpdates();
        void readParam(const char* file, wxChoice* choices);

        void readPairings(vector<BluetoothPairing>& bluetoothPairings, wxString tool);
        void readDongles(vector<DongleInfo>& dongleInfos);

        int setDongleAddress(vector<DongleInfo>& dongleInfos, int dongleIndex, wxString address);
        
        int choosePairing(BluetoothPairing& pairing);
        int chooseDongle(wxString address, DongleInfo& dongleInfo);
        
        wxString generateLinkKey();

        int ps3Setup();
        int ps4Setup();
        int ps4Repair();
        int readDonglePairings(vector<BluetoothPairing>& donglePairings);
        BluetoothPairing selectBrokenPairing(vector<BluetoothPairing>& brokenDonglePairings);

        void readDebugStrings(wxArrayString & values);

        void initDownload(wxProgressDialog * dlg);
        void cleanDownload();

        //(*Identifiers(launcherFrame)
        static const long ID_STATICTEXT4;
        static const long ID_CHOICE1;
        static const long ID_STATICTEXT3;
        static const long ID_CHOICE3;
        static const long ID_BUTTON2;
        static const long ID_STATICTEXT1;
        static const long ID_CHOICE2;
        static const long ID_STATICTEXT2;
        static const long ID_CHOICE5;
        static const long ID_BUTTON4;
        static const long ID_STATICTEXT5;
        static const long ID_CHOICE4;
        static const long ID_STATICTEXT6;
        static const long ID_CHOICE6;
        static const long ID_BUTTON1;
        static const long ID_BUTTON3;
        static const long ID_PANEL1;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM8;
        static const long ID_MENUITEM11;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM3;
        static const long ID_MENUITEM9;
        static const long idMenuQuit;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM10;
        static const long ID_MENUITEM5;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(launcherFrame)
        wxChoice* Output;
        wxChoice* ProcessOutputChoice;
        wxFlexGridSizer* FlexGridSizer1;
        wxPanel* Panel1;
        wxStatusBar* StatusBar1;
        wxMenuItem* MenuAutoBindControls;
        wxButton* ButtonCheck;
        wxMenuItem* MenuEditFpsConfig;
        wxChoice* InputChoice;
        wxButton* OutputNewButton;
        wxButton* Button1;
        wxMenuItem* MenuGetConfigs;
        wxMenuItem* MenuUpdate;
        wxFlexGridSizer* OutputSizer;
        wxStaticText* StaticText1;
        wxStaticText* StaticText3;
        wxFlexGridSizer* IOSizer;
        wxChoice* OutputChoice;
        wxMenuItem* MenuRefresh;
        wxMenuItem* MenuStartupUpdates;
        wxMenuItem* MenuItem3;
        wxChoice* Input;
        wxMenuItem* MenuItem5;
        wxStaticText* StaticText4;
        wxFlexGridSizer* SourceIpSizer;
        wxStaticText* StaticText5;
        wxStaticText* StaticText2;
        wxChoice* MouseGrabChoice;
        wxMenuItem* MenuItem4;
        wxMenuItem* MenuItem6;
        wxButton* ButtonStart;
        wxMenuItem* MenuEditConfig;
        wxStaticText* OutputText;
        wxFlexGridSizer* MouseSizer;
        //*)

        wxLocale* locale;

        bool started;
        long startTime;

        wxString userDir;
        wxString gimxConfigDir;
        wxString gimxLogDir;
        wxString gimxMacrosDir;
        wxString launcherDir;
        wxString gimxDir;

        wxArrayString hids;

        bool openLog;

        wxProgressDialog * progressDialog;
        int progressValue;

        DECLARE_EVENT_TABLE()
};

#endif // LAUNCHERMAIN_H
