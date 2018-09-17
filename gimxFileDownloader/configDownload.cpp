/*
 * configDownload.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/configDownload.h"

int progress_callback_configupdater_terminal(void *clientp, configupdater::ConfigUpdaterStatus status, double progress, double total)
{
    return ((ConfigDownload *) clientp)->updateProgress(status, progress, total);
}

int updateProgress_common(ttyProgressDialog* progressDialog, configupdater::ConfigUpdaterStatus status, double progress, double total)
{
    std::string message;
    switch(status)
    {
        case configupdater::ConfigUpdaterStatusConnectionPending:
            message = "Connecting";
            break;
        case configupdater::ConfigUpdaterStatusDownloadInProgress:
            message = "Progress: ";
            message += Updater::getProgress(progress, total);
            break;
        default:
            break;
    }

    if (status >= 0) {
        if (progressDialog->Update(progress, message) == false) {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}


ConfigDownload::ConfigDownload() : winData(newWinData(stdscr))
{
    screen = newwin(winData->height, winData->width, winData->startY, winData->startX);
    winData->menuWin = screen;

    /*Setup directory strings*/
    getConfigDir(userDir);
    /*if(getConfigDir(userDir))
    {
        //TODO make it known that the config directory couldn't be found
    }*/
    gimxDir = userDir + GIMX_DIR;
    gimxConfigDir = gimxDir + CONFIG_DIR;
}


int ManualConfigDownload::updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total)
{
    return updateProgress_common(progressDialog, status, progress, total);
}

void ManualConfigDownload::initDownload(ttyProgressDialog* dialog)
{
    progressDialog = dialog;
    progressDialog->dialog();
}
void ManualConfigDownload::cleanDownload()
{
    werase(screen);
    wrefresh(screen);
    progressDialog = NULL;
}
//Return codes => 0 ok, 1 cancelled, 2 something wrong with getting configs
int ManualConfigDownload::chooseConfigs()
{
    /*Choose configs*/
        /*Create the selection menu window*/
    /*int startY, startX;
    startY = 0;
    startX = 0;*/

    /*Download config list*/
    configupdater::ConfigUpdaterStatus status;
    {
        printw("Downloading config list\nConnecting\n");
        refresh();

        ttyProgressDialog pDialog(winData.get());
        initDownload(&pDialog); //Also opens dialog window
        status = configupdater().getconfiglist(configList, progress_callback_configupdater_terminal, this);
        wgetch(screen);//for debugging
        cleanDownload();
    }

    if(status == configupdater::ConfigUpdaterStatusCancelled)
        return status;

    /*Ensure the config list is not empty*/
    if(configList.empty())
    {
        printw("Can't retrieve configs list!\n");
        refresh();
        return status;
    }

    /*Add config names to options list*/
    std::vector<std::string> options;
    for(std::string configName : configList)
        options.push_back(configName);

    SelectionMenu selectionMenu(winData.get(), options);
    winData->title = "Select the files to download";

    /*Stylise the menu borders*/
    //				borders => (bool, we, ns)
    selectionMenu.setDrawBorder(true, 0, 0);

    selectionMenu.menuLoop();
    std::vector<int> chosen;
    selectionMenu.getResult(chosen);

    werase(screen);

    std::string sel;
    std::string file;
    /*Check if any chosen configs exist already*/
    int c;
    for(int cIndex : chosen)
    {
        sel = options[cIndex];
        file = gimxConfigDir + sel;
        if(fileExists(file))
        {
            wprintw(screen, "Overwrite local file: %s?(y or n)\n", file.c_str());
            wrefresh(screen);
            c = wgetch(screen);

            //No
            if(not (c == 121 || c == 89) )
                continue; 
        }
        selectedConfigs.push_back(sel);
    }
    werase(screen);

    return grabConfigs();
}

int ManualConfigDownload::grabConfigs()
{
    configupdater::ConfigUpdaterStatus status = configupdater::ConfigUpdaterStatusOk;
    wprintw(screen, "Downloading\nConnecting\n");
    wrefresh(screen);
    for (std::list<std::string>::iterator it = selectedConfigs.begin(); it != selectedConfigs.end(); ++it)
    {   
        status = configupdater().getconfig(gimxConfigDir, *it, progress_callback_configupdater_terminal, this);
        if (status == configupdater::ConfigUpdaterStatusCancelled)
            break;
    }
    
    //readConfigs();
    if(status == configupdater::ConfigUpdaterStatusOk)
        wprintw(screen, "Completed\n");
    else if (status != configupdater::ConfigUpdaterStatusCancelled)
        wprintw(screen, "Can't retrieve configs!\n");

    wrefresh(screen);
    return status;
}
