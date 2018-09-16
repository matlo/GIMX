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

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>

#include "cursesIO.h"
using namespace EasyCurses;

/*Configuration file downloaders*/
//Common
int updateProgress_common(ttyProgressDialog* progressDialog, configupdater::ConfigUpdaterStatus status, double progress, double total);

//Abstraction
class ConfigDownload
{
public:
    virtual int chooseConfig() = 0;
    virtual int grabConfig() = 0;

    virtual void initDownload(ttyProgressDialog* dialog) = 0;
    virtual void cleanDownload() = 0;
    virtual int updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total) = 0;
};

class ManualConfigDownload : public ConfigDownload
{
public:
    ManualConfigDownload();
    virtual ~ManualConfigDownload() { delwin(downloadWin); delwin(selectionMenuWin); }

    int chooseConfig();
    int grabConfig();

    void initDownload(ttyProgressDialog* dialog);
    void cleanDownload();
    int updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total);

private:
    void* clientp;

    int height, width, startY, startX;
    WINDOW* selectionMenuWin;
    WINDOW* downloadWin;

    ttyProgressDialog* progressDialog;

    //Information needed to allow user to choose config(s)
    std::list<std::string> configList;
    std::list<std::string> selectedConfigs;
    std::string configDir;
};

class AutoConfigDownload : public ConfigDownload
{
public:
    int chooseConfig();
    int grabConfig();

    void initDownload(ttyProgressDialog* dialog);
    void cleanDownload();
    int updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total);

private:
    void* clientp;

    int height, width, startY, startX;
    WINDOW* downloadWin;
    
    ttyProgressDialog* progressDialog;
};


#endif //CONFIGDOWNLOAD_H