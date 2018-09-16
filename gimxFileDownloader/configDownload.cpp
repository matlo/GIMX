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
        //TODO change pointer to something that updates terminal instead
        if (progressDialog->Update(progress, message) == false) {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}


ManualConfigDownload::ManualConfigDownload()
{
    getmaxyx(stdscr, height, width);
    downloadWin = newwin(height, width, startY, startX);
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
    werase(downloadWin);
    wrefresh(downloadWin);
    progressDialog = NULL;
}
//Return codes => 0 ok, 1 cancelled, 2 something wrong with getting configs
int ManualConfigDownload::chooseConfig()
{
    /*Choose configs*/
        /*Create the selection menu window*/
    int startY, startX;
    startY = 0;
    startX = 0;

    std::vector<std::string> options;
    for(std::string configName : configList)
        options.push_back(configName);

    /*Download config list*/
    configupdater::ConfigUpdaterStatus status;
    {
        printw("Downloading config list\nConnecting\n");
        refresh();

        ttyProgressDialog pDialog(downloadWin);
        initDownload(&pDialog); //Also opens dialog window
        status = configupdater().getconfiglist(configList, progress_callback_configupdater_terminal, this);
        wgetch(downloadWin);//for debugging
        cleanDownload();
    }

    if(status == configupdater::ConfigUpdaterStatusCancelled)
        return 1;

    /*Ensure the config list is not empty*/
    if(configList.empty())
    {
        printw("Can't retrieve configs list!\n");
        refresh();
        return 2;
    }

    /*Add config names to options list*/
    for(auto name : configList)
        options.push_back(name);

    selectionMenuWin = newwin(height, width, startY, startX);
    SelectionMenu selectionMenu(selectionMenuWin, options, std::string("Select the files to download"));

    /*Stylise the menu borders*/
    //				borders => (bool, we, ns)
    selectionMenu.setDrawBorder(true, 0, 0);

    selectionMenu.menuLoop();
    std::vector<int> chosen;
    selectionMenu.getResult(chosen);

    werase(selectionMenuWin);
    /*Check if any chosen configs exist already*/
    for(int cIndex : chosen)
    {
        std::string sel = options[chosen[cIndex]];
        std::string file = configDir + sel;
        std::ifstream check(file);
        if(check.good())
        {
            wprintw(selectionMenuWin, "Overwrite local file: %s?(y or n)\n", file);
            wrefresh(selectionMenuWin);
            int c = getch();

            //No
            if(not (c == 121 || c == 89) )
                continue; 
        }
        selectedConfigs.push_back(sel);
    }
    erase();
    delwin(selectionMenuWin);

    return grabConfig();
}

int ManualConfigDownload::grabConfig()
{
    configupdater::ConfigUpdaterStatus status = configupdater::ConfigUpdaterStatusOk;
    for (std::list<std::string>::iterator it = selectedConfigs.begin(); it != selectedConfigs.end(); ++it)
    {
        printw("Downloading\nConnecting");
        
        status = configupdater().getconfig(configDir, *it, progress_callback_configupdater_terminal, this);
        if (status == configupdater::ConfigUpdaterStatusCancelled)
            break;
    }
    
    //readConfigs();
    if(status == configupdater::ConfigUpdaterStatusOk)
        printw("Completed\n");
    else if (status != configupdater::ConfigUpdaterStatusCancelled)
    {
        printw("Can't retrieve configs!");
        return 2;
    }

    return 0;
}
