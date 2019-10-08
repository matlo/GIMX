/*
 * configDownload.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/configDownload.h"


int process_cb(GE_Event* event __attribute__((unused)))
{
    return 0;
}


int progress_callback_configupdater_terminal(void *clientp, CUStat status, double progress, double total)
{
    return ((ConfigDownload *) clientp)->updateProgress(status, progress, total);
}

int updateProgress_common(ttyProgressDialog* progressDialog, CUStat status, double progress, double total)
{
    std::string message;
    switch(status)
    {
        case Pending:
            message = "Connecting";
            break;
        case Running:
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


/*
 * Return codes for both manual and auto config download classes
 * -3 download failed, -2 ConfigUpdater module failed to init,
 * -1 cancelled, 0 okay, 1 connection pending, 2 download in progress
 *
 */

ConfigDownload::ConfigDownload() : dlWinData(newWinData(stdscr))
{
    {
        char* res = gfile_homedir();
        if(res == NULL)
        {
            wprintw(stdscr, "Cannot access config directory. Press any key to continue");
            wrefresh(stdscr);
            wgetch(stdscr);
        }

        gimxConfigDir = std::string(res) + (GIMX_DIR CONFIG_DIR);
        free(res);
    }

    //Progress dialog
    int height = dlWinData->height;
    int width  = dlWinData->width;
    dlWinData->height = 7;
    dlWinData->width  = 30;
    dlWinData->startX = (width /2) - (dlWinData->width /2);
    dlWinData->startY = (height /2) - (dlWinData->height /2);
    dlScreen = newwin(dlWinData->height, dlWinData->width, dlWinData->startY, dlWinData->startX);
    dlWinData->win = dlScreen;
    progressDialog = std::make_unique<ttyProgressDialog>(dlWinData.get(), "Downloading");
}

void ConfigDownload::initDownload()
{
    progressDialog->dialog();
}

void ConfigDownload::cleanDownload()
{
    progressDialog->resetPBar();
    werase(dlScreen);
    wrefresh(dlScreen);
}

int ConfigDownload::grabConfigs(std::list<std::string>& configs, WINDOW* screen)
{
    CUStat status = Ok;
    if(!configs.empty())
    {
        initDownload();
        for(std::list<std::string>::iterator it = configs.begin(); it != configs.end(); ++it)
        {
            status = configupdater().getconfig(gimxConfigDir, *it,
              progress_callback_configupdater_terminal, this);
            if (status == Cancelled)
                break;
        }
        cleanDownload();
    }
    else
        wprintw(screen, "No configs to download\n");

    if(status == Ok)
        wprintw(screen, "Completed\n");
    else if(status != Cancelled)
        wprintw(screen, "Can't retrieve configs!\n");
    wprintw(screen, "Press any key to continue");

    wrefresh(screen);
    wgetch(screen);

    return status;
}

int ConfigDownload::updateProgress(CUStat status, double progress, double total)
{
    return updateProgress_common(progressDialog.get(), status, progress, total);
}


ManualConfigDownload::ManualConfigDownload() : winData(newWinData(stdscr))
{
    //Selection menu
    winData->height -= 1;
    winData->win = newwin(winData->height, winData->width, winData->startY, winData->startX);

    //Help dialog
    helpText = "Press:\n\nESC to exit\nENTER to select\nArrow keys to change selection\n"\
      "Page up and down keys to change page";
}

bool ManualConfigDownload::help()
{
    BasicMenu helpMenu(helpText, winData.get(), "Help menu");
    /*Stylise the menu borders*/
    //         borders => (bool, we, ns)
    helpMenu.setDrawBorder(true, 0, 0);
    flushinp();

    mvwprintw(stdscr, winData->height, 0, "Press ESC to exit menu");
    wrefresh(stdscr);

    helpMenu.menuLoop();

    wmove(stdscr, winData->height, 0);
    wclrtoeol(stdscr);
    mvwprintw(stdscr, winData->height, 0, "Press h for help");
    wrefresh(stdscr);

    return true;
}

int ManualConfigDownload::chooseConfigs()
{
    CUStat status;

    /*Download config list*/
    initDownload();
    status = configupdater().getconfiglist(configList,
      progress_callback_configupdater_terminal, this);
    cleanDownload();

    if(status != Ok)
    {
        wprintw(winData->win, "ConfigUpdater module failed. Return value %i.\n" \
          "Reason: ", status);

        switch (status) {
            case DlFail:
                wprintw(winData->win, "download failed.", status);
                break;
            case InitFail:
                wprintw(winData->win, "initialisation failed.", status);
                break;
            case Cancelled:
                wprintw(winData->win, "download cancelled.", status);
                break;
            default:
                break;
        }

        wprintw(winData->win, "\nPress any key to continue", status);
        wrefresh(winData->win);
        wgetch(winData->win);
        return status;
    }

    /*Ensure the config list is not empty*/
    if(configList.empty())
    {
        wprintw(winData->win, "Can't retrieve configs list!\nPress any key to continue");
        wrefresh(winData->win);
        wgetch(winData->win);
        return status;
    }

    /*Add config names to options list*/
    std::vector<unsigned> chosen;
    {
        std::string options;
        for(std::string configName : configList)
            options += configName + "\n";

        SelectionMenu selectionMenu(options, winData.get(),
          "Select the files to download");

        /*Stylise the menu borders*/
        //				borders => (bool, we, ns)
        selectionMenu.setDrawBorder(true, 0, 0);

        const char* help = "Press h for help";
        selectionMenu.setCustomAction(std::bind(&ManualConfigDownload::help, this));
        {
            std::map<unsigned, NavContent>& kB = selectionMenu.getKeyBindings();
            kB[104] = NavContent::custom; //104 => 'h'
            selectionMenu.setKeyBindings(kB);
        }
        mvwprintw(stdscr, winData->height, 0, help);
        wrefresh(stdscr);

        selectionMenu.menuLoop();
        selectionMenu.getResult(chosen);
    }

    werase(winData->win);

    std::string sel;
    std::string file;
    /*Check if any chosen configs exist already*/
    int c;
    for(int cIndex : chosen)
    {
        sel = *(std::next(configList.begin(), cIndex));
        file = gimxConfigDir + sel;

        if(gfile_isfile(gimxConfigDir.c_str(), sel.c_str()) != 0 )
        {
            wprintw(winData->win, "Overwrite local file: %s?(y or n)\n", file.c_str());
            wrefresh(winData->win);
            c = wgetch(winData->win);

            if( not (c == 121 || c == 89) ) //don't overwrite
                continue;
        }
        selectedConfigs.push_back(sel);
    }
    werase(winData->win);
    wrefresh(winData->win);

    status = static_cast<CUStat>(grabConfigs(selectedConfigs, winData->win));

    wrefresh(winData->win);
    return status;
}


AutoConfigDownload::AutoConfigDownload()
{

}

int AutoConfigDownload::chooseConfigs()
{
    std::list<std::string> joysticks;

    GPOLL_INTERFACE poll_interace =
    {
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if(ginput_init(&poll_interace, GE_MKB_SOURCE_NONE, process_cb) < 0)
    {
        ginput_quit();
        return -4;
    }

    for (int i = 0; ginput_joystick_name(i) != NULL; ++i)
    {
        joysticks.push_back(ginput_joystick_name(i));
    }

    ginput_quit();

    // TODO MLA: have an online index with device -> config, and be able to merge multiple configs

    struct
    {
        std::string name;
        std::string config;
    } configs [] =
#ifndef WIN32
    {
        { "Logitech Inc. WingMan Formula", "LogitechWingManFormula_G29.xml" },
        { "Logitech Inc. WingMan Formula GP", "LogitechWingManFormulaGP_G29.xml" },
        { "Logitech Inc. WingMan Formula Force", "LogitechWingManFormulaForce_G29.xml" },
        { "Logitech Inc. WingMan Formula Force GP", "LogitechWingManFormulaForceGP_G29.xml" },
        { "Logitech Logitech Driving Force", "LogitechDrivingForce_G29.xml" },
        { "Logitech Logitech Driving Force EX", "LogitechDrivingForceEx_G29.xml" },
        { "Logitech Logitech Driving Force Rx", "LogitechDrivingForceRx_G29.xml" },
        { "Logitech Logitech Formula Force EX", "LogitechFormulaForceEx_G29.xml" },
        { "PS3/USB Cordless Wheel", "LogitechDrivingForceWireless_DS4.xml" },
        // TODO MLA { "Logitech MOMO Force USB", "LogitechMomoForce_G29.xml" },
        { "Logitech Logitech Driving Force Pro", "LogitechDrivingForcePro_G29.xml" },
        { "G25 Racing Wheel", "LogitechG25_G29.xml" },
        { "Driving Force GT", "LogitechDrivingForceGT_G29.xml" },
        { "G27 Racing Wheel", "LogitechG27_G29.xml" },
        { "Logitech  Logitech MOMO Racing ", "LogitechMomoRacing_G29.xml" },
        { "Logitech G920 Driving Force Racing Wheel", "LogitechG920_G29.xml" },
    };
#else
    {
        { "Logitech WingMan Formula (Yellow) (USB)", "LogitechWingManFormula_G29.xml" },
        { "Logitech WingMan Formula GP", "LogitechWingManFormulaGP_G29.xml" },
        { "Logitech WingMan Formula Force USB", "LogitechWingManFormulaForce_G29.xml" },
        { "Logitech WingMan Formula Force GP USB", "LogitechWingManFormulaForceGP_G29.xml" },
        { "Logitech Driving Force USB", "LogitechDrivingForce_G29.xml" },
        { "Logitech MOMO Force USB", "LogitechMomoForce_G29.xml" },
        { "Logitech Driving Force Pro USB", "LogitechDrivingForcePro_G29.xml" },
        { "Logitech G25 Racing Wheel USB", "LogitechG25_G29.xml" },
        { "Logitech Driving Force GT USB", "LogitechDrivingForceGT_G29.xml" },
        { "Logitech G27 Racing Wheel USB", "LogitechG27_G29.xml" },
        { "Logitech MOMO Racing USB", "LogitechMomoRacing_G29.xml" },
        { "Logitech G920 Driving Force Racing Wheel USB", "LogitechG920_G29.xml" },
    };
#endif

    std::list<std::string> download;

    for (std::list<std::string>::iterator it = joysticks.begin(); it != joysticks.end(); ++it)
    {
        for (unsigned int i = 0; i < sizeof(configs) / sizeof(*configs); ++i)
        {
            if(*it == configs[i].name)
            {
                download.push_back(configs[i].config);
            }
        }
    }

    grabConfigs(download, dlScreen);

    return Ok;
}
