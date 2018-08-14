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
    downloadWin = newwin(height, width, starty, startx);
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
    wclear(downloadWin);
    wrefresh(downloadWin);
    progressDialog = NULL;
}
//Return codes => 0 ok, 1 cancelled, 2 something wrong with getting configs
int ManualConfigDownload::chooseConfig()
{
    /*Choose configs*/
        /*Create the selection menu window*/
    int starty, startx;
    starty = 0;
    startx = 0;

    std::vector<std::string> options = {"Cancel", "Done"};
    for(std::string configName : configList)
        options.push_back(configName);

    selectionMenuWin = newwin(height, width, starty, startx);

    Menu selectionMenu(selectionMenuWin, height, width, starty, startx, options);
    selectionMenu.setKeypad();

    /*Stylise the menu borders*/
    //					borders =>    we  ns
    selectionMenu.setDrawBorder(true, 36, 0);

    /*Download config list*/
    configupdater::ConfigUpdaterStatus status;
    {
        printw("Downloading config list\nConnecting\n");
        refresh();

        ttyProgressDialog pDialog(downloadWin, "Downloading", height, width, 0, 0, "Progress");
        initDownload(&pDialog); //Also opens dialog window
        status = configupdater().getconfiglist(configList, progress_callback_configupdater_terminal, this);
        wprintw(downloadWin, "status = %i\n", status); wgetch(downloadWin);//for debugging
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

    int menuChoice;
    std::vector<int> chosen;
    while(true)
    {
        clear();

        std::string prompt = { "Select the files to download.\n" };
        wprintw(selectionMenuWin, prompt.c_str());
        flushinp();
        wrefresh(selectionMenuWin);

        menuChoice = selectionMenu.menuLoop();
        if(menuChoice == 1)
            return 1; //User chose to cancel
        else if(menuChoice == 2)
        {
            /*Ensure the selected config list is not empty*/
            if(!selectedConfigs.empty())
                break; //Finished choosing
            else
            {
                mvwprintw(stdscr, 0, width - prompt.length(), "You must chose at least one config file, or cancel.");
                continue;
            }
                
        }
        else
        {
            /*Check if already chosen and render check marks*/
            for(int choice : chosen)
            {
                //Already toggled
                if(choice == menuChoice)
                {
                    chosen.erase(chosen.begin() +menuChoice);
                    mvwprintw(selectionMenuWin, choice, width -1, " ");
                }
                else
                {
                    chosen.push_back(menuChoice);
                    mvwprintw(selectionMenuWin, choice, width -1, "X");
                }
            }
        }
    }
    clear();

    /*Check if any chosen configs exist already*/
    for(int cIndex : chosen)
    {
        std::string sel = options[chosen[cIndex]];
        std::string file = configDir + sel;
        std::ifstream check(file);
        if(check.good())
        {
            wprintw(selectionMenuWin, "Overwrite local file: %s?(y or n)\n", file);
            int c = getch();

            //No
            if(not (c == 121 || c == 89) )
                continue; 
        }
        selectedConfigs.push_back(sel);
    }
    clear();
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
