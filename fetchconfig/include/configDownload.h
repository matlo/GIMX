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

#include "fileOps.h"
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
int updateProgress_common(ttyProgressDialog* progressDialog, configupdater::ConfigUpdaterStatus status, double progress, double total);


class ConfigDownload
{
protected:
    std::string userDir;
    std::string gimxDir;
    std::string gimxConfigDir;
    std::unique_ptr<ttyProgressDialog> progressDialog;
    std::unique_ptr<WinData> dlWinData;
    WINDOW* dlScreen;
    void* clientp;

public:
    ConfigDownload(WINDOW* win, WinData* win1);
    virtual ~ConfigDownload() { };

    virtual bool setUpDirectories(WINDOW* win);

    virtual int chooseConfigs()  = 0;
    virtual int getConfig(std::string& configName);
    virtual int grabConfigs(std::list<std::string>& configs, WINDOW* screen);

    virtual void initDownload();
    virtual void cleanDownload();
    virtual int  updateProgress(configupdater::ConfigUpdaterStatus status,
      double progress, double total);
};

class ManualConfigDownload : public ConfigDownload
{
public:
    ManualConfigDownload();
    ~ManualConfigDownload() { delwin(screen); delwin(dlScreen); }

    bool help();

    virtual int chooseConfigs() override;

private:
    WINDOW* screen;
    std::unique_ptr<WinData> winData;

    std::string helpText;

    //Information needed to allow user to choose config(s)
    std::list<std::string> configList;
    std::list<std::string> selectedConfigs;
};

class AutoConfigDownload : public ConfigDownload
{
public:
    AutoConfigDownload();

    int chooseConfigs();
};


#endif //CONFIGDOWNLOAD_H
