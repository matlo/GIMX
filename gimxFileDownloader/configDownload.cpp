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
        if(progressDialog->update(progress, message) == false) {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}


ConfigDownload::ConfigDownload(WINDOW* win)
{
    setUpDirectories(win);
}

bool ConfigDownload::setUpDirectories(WINDOW* screen)
{
    /*Setup directory strings*/
    int res = getUserConfigDir(userDir);
    if(res)
    {
        wprintw(screen, "Cannot access config directory. Press any key to continue");
        wrefresh(screen);
        wgetch(screen);
        return false;
    }
    gimxDir = userDir + GIMX_DIR;
    gimxConfigDir = gimxDir + CONFIG_DIR;
    return true;
}


ManualConfigDownload::ManualConfigDownload() : ConfigDownload(stdscr), winData(newWinData(stdscr)),
  dlWinData(newWinData(stdscr))
{
    //Selection menu
    winData->height -= 1;
    screen       = newwin(winData->height, winData->width, winData->startY, winData->startX);
    winData->win = screen;
    
    //Progress dialog
    dlWinData->height = 7;
    dlWinData->width  = 30;
    dlWinData->startX = (winData->width /2) - (dlWinData->width /2);
    dlWinData->startY = (winData->height /2) - (dlWinData->height /2);
    dlScreen = newwin(dlWinData->height, dlWinData->width, dlWinData->startY, dlWinData->startX);
    dlWinData->win = dlScreen;
    progressDialog.reset(new ttyProgressDialog(dlWinData.get(), "Downloading"));
    
    //Help dialog
    helpText = "Press:\n\nESC to exit\nENTER to select\nArrow keys to change selection\n"\
    "Page up and down keys to change page";
    SelectionMenu::keyBindings[104] = EasyCurses::NavContent::custom; //104 => 'h'
}

bool ManualConfigDownload::help()
{
    BasicMenu helpMenu(helpText, winData.get(), "BasicMenu demo");
    /*Stylise the menu borders*/
    //				borders => (bool, we, ns)
    helpMenu.setDrawBorder(true, 0, 0);
    flushinp();
    
    mvwprintw(stdscr, winData->height, 0, "Press ESC to exit menu");
    wrefresh(stdscr);
    
    helpMenu.menuLoop();
    delwin(screen);
    
    wmove(stdscr, winData->height, 0);
    wclrtoeol(stdscr);
    mvwprintw(stdscr, winData->height, 0, "Press h for help");
    wrefresh(stdscr);
    
    return true;
}

int ManualConfigDownload::updateProgress(configupdater::ConfigUpdaterStatus status, double progress, double total)
{
    return updateProgress_common(progressDialog.get(), status, progress, total);
}

void ManualConfigDownload::initDownload()
{
    progressDialog->dialog();
}
void ManualConfigDownload::cleanDownload()
{
    progressDialog->resetPBar();
    werase(dlScreen);
    wrefresh(dlScreen);
}
//Return codes => 0 ok, 1 cancelled, 2 something wrong with getting configs
int ManualConfigDownload::chooseConfigs()
{
    configupdater::ConfigUpdaterStatus status;

    /*Download config list*/
    initDownload();
    status = configupdater().getconfiglist(configList, progress_callback_configupdater_terminal, this);
    cleanDownload();

    if(status == configupdater::ConfigUpdaterStatusCancelled)
        return status;

    /*Ensure the config list is not empty*/
    if(configList.empty())
    {
        wprintw(screen, "Can't retrieve configs list!\nPress any key to continue");
        wgetch(screen);
        wrefresh(screen);
        return status;
    }

    /*Add config names to options list*/
    std::vector<int> chosen;
    {
        std::string options;
        for(std::string configName : configList)
            options += configName + "\n";

        SelectionMenu selectionMenu(options, winData.get(), "Select the files to download");

        /*Stylise the menu borders*/
        //				borders => (bool, we, ns)
        selectionMenu.setDrawBorder(true, 0, 0);

        const char* help = "Press h for help";
        selectionMenu.setCustomAction(std::bind(&ManualConfigDownload::help, this));
        mvwprintw(stdscr, winData->height, 0, help);
        wrefresh(stdscr);
        
        selectionMenu.menuLoop();
        selectionMenu.getResult(chosen);
    }
    
    werase(screen);

    std::string sel;
    std::string file;
    /*Check if any chosen configs exist already*/
    int c;
    for(int cIndex : chosen)
    {
        sel = *(std::next(configList.begin(), cIndex));
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
    if(!selectedConfigs.empty())
    {
        initDownload();
        for (std::list<std::string>::iterator it = selectedConfigs.begin(); it != selectedConfigs.end(); ++it)
        {
            status = configupdater().getconfig(gimxConfigDir, *it, progress_callback_configupdater_terminal, this);
            if (status == configupdater::ConfigUpdaterStatusCancelled)
                break;
        }
        cleanDownload();
    }

    if(status == configupdater::ConfigUpdaterStatusOk)
        wprintw(screen, "Completed\n");
    else if (status != configupdater::ConfigUpdaterStatusCancelled)
        wprintw(screen, "Can't retrieve configs!\n");
    wprintw(screen, "Press any key to continue");

    wrefresh(screen);
    return status;
}
