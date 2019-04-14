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

#include "fileOps.h"
#include "easyCurses.h"


using namespace EasyCurses;

/*Configuration file downloaders*/
//Common
int updateProgress_common(ttyProgressDialog* progressDialog, configupdater::ConfigUpdaterStatus status, double progress, double total);

class ConfigDownload
{
protected:
    std::string userDir;
    std::string gimxDir;
    std::string gimxConfigDir;

public:
    ConfigDownload(WINDOW* win);
    virtual ~ConfigDownload() { };

    virtual bool setUpDirectories(WINDOW* win);
    
    virtual int chooseConfigs() = 0;
    virtual int grabConfigs() = 0;

    virtual void initDownload() = 0;
    virtual void cleanDownload() = 0;
    virtual int updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total) = 0;
};

class ManualConfigDownload : public ConfigDownload
{
public:
    ManualConfigDownload();
    ~ManualConfigDownload() { delwin(screen); delwin(dlScreen); }
    
    bool help();
    
    int chooseConfigs();
    int grabConfigs();

    void initDownload();
    void cleanDownload();
    int  updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total);

private:
    WINDOW* screen;
    WINDOW* dlScreen;
    std::unique_ptr<WinData> winData;
    std::unique_ptr<WinData> dlWinData;
    std::string helpText;

    std::unique_ptr<ttyProgressDialog> progressDialog;
    
    void* clientp;

    //Information needed to allow user to choose config(s)
    std::list<std::string> configList;
    std::list<std::string> selectedConfigs;
};

class AutoConfigDownload : public ConfigDownload
{
public:
    int chooseConfigs();
    int grabConfigs();

    void initDownload();
    void cleanDownload();
    int updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total);

private:
    void* clientp;

    /*int height, width, startY, startX;
    WINDOW* downloadWin;*/
};


#endif //CONFIGDOWNLOAD_H
