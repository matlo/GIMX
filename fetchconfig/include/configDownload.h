/*
 * configDownload.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#ifndef  CONFIGDOWNLOAD_H
#define  CONFIGDOWNLOAD_H

#include <string>
#include <list>
#include <vector>
#include <fstream>
#include <iterator> //std::next & std::prev

#include <memory> //for smart pointers

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>
#include <gimxusb/include/gusb.h>
#include <gimxinput/include/ginput.h>
#include <gimxfile/include/gfile.h>
#include <gimxfetchconfig/include/commonOps.h>

#include "../directories.h"
#include "easyCurses.h"


using namespace EasyCurses;


#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif


int process_cb(GE_Event* event __attribute__((unused)));


/*Configuration file downloaders*/
//Common
typedef configupdater::ConfigUpdaterStatus CUStat;
constexpr configupdater::ConfigUpdaterStatus Ok        = configupdater::ConfigUpdaterStatusOk;
constexpr configupdater::ConfigUpdaterStatus Pending   = configupdater::ConfigUpdaterStatusConnectionPending;
constexpr configupdater::ConfigUpdaterStatus Running   = configupdater::ConfigUpdaterStatusDownloadInProgress;
constexpr configupdater::ConfigUpdaterStatus Cancelled = configupdater::ConfigUpdaterStatusCancelled;
constexpr configupdater::ConfigUpdaterStatus InitFail  = configupdater::ConfigUpdaterStatusInitFailed;
constexpr configupdater::ConfigUpdaterStatus DlFail    = configupdater::ConfigUpdaterStatusDownloadFailed;

bool printStat(CUStat status, WINDOW* s);

int updateProgress_common(ttyProgressDialog* progressDialog, CUStat status, double progress, double total);


class ConfigDownload
{
protected:
    std::string gimxConfigDir;
    std::unique_ptr<ttyProgressDialog> progressDialog;
    std::unique_ptr<WinData> dlWinData;
    WINDOW* dlScreen;
    void* clientp;

public:
    ConfigDownload();
    virtual ~ConfigDownload() { delwin(dlScreen); };

    virtual int chooseConfigs()  = 0;
    virtual CUStat grabConfigs(std::list<std::string>& configs, WINDOW* screen);

    virtual void initDownload();
    virtual void cleanDownload();
    virtual int  updateProgress(CUStat status, double progress, double total);
};

class ManualConfigDownload : public ConfigDownload
{
public:
    ManualConfigDownload();

    bool help();

    int chooseConfigs() override;

private:
    std::unique_ptr<WinData> winData;

    std::string helpText;

    //Information needed to allow user to choose config(s)
    std::list<std::string> configList;
    std::list<std::string> selectedConfigs;
};

class AutoConfigDownload : public ConfigDownload
{
public:
    AutoConfigDownload() { }

    int chooseConfigs() override;
};


#endif //CONFIGDOWNLOAD_H
