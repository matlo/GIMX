/*
 * gimx-fetchconfig
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */
#include <memory> //for smart pointers
#include <string>
#include <iostream>

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>

#include "parseArgs.h"
#include "easyCurses.h"
#include "configDownload.h"

//Callback functionality for download progress
int progress_callback_configupdater_terminal(void *clientp, configupdater::ConfigUpdaterStatus status, double progress, double total);


void help()
{
    std::cout << "Usage: gimxFileDownloader <OPTION>" << std::endl;
    std::cout << "Options: " << std::endl;
    std::cout << "    -a, --autoconfig    Download configuration "\
      "files based on peripherals attached" << std::endl;
    std::cout << "    -c, --config        Manually choose and download "\
      "configuration files" << std::endl;
    std::cout << "    -h, --help          Display this help text" << std::endl;
}


class FileDownloader
{
public:
    void manualConfig()
    {
        configDl = std::unique_ptr<ConfigDownload>(new ManualConfigDownload);
        static_cast<ManualConfigDownload*>(configDl.get());

        configDl->chooseConfigs();
    }
    void autoConfig()
    {
        configDl = std::unique_ptr<AutoConfigDownload>(new AutoConfigDownload);
        static_cast<AutoConfigDownload*>(configDl.get());

        configDl->chooseConfigs();
    }

    int callOptions(struct option* longOptions, int optionCharacter, int optionIndex)
    {
        int result = -1;
        switch(optionCharacter)
        {
            case -1:
                //End of list
                break;
            case 0:
                //If this option set a flag, do nothing else now.
                if(longOptions[optionIndex].flag != 0)
                    result = 1;
                break;
            //Manual configuration download
            case 'c':
                manualConfig();
                break;
            //Automatic configuration download
            case 'a':
                autoConfig();
                break;
            case 'h':
            case '?':
            default:
                result = -2;
                break;
        }

        return result;
    }

private:
    std::unique_ptr<ConfigDownload> configDl;
};


int main(int argc, char* argv[])
{
    /*Start curses*/
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    keypad(stdscr, true);

    /*Handle command line options*/
    struct option longOpts [] =
    {
            //These options don’t set a flag. We distinguish them by their indices.
            { "config", no_argument, 0, 'c' },
            { "autoconfig", no_argument, 0, 'a' },
            { "help", no_argument, 0, 'h' },
            { 0, 0, 0, 0 }
    };

    FileDownloader fDownloader;
    auto callback = [&fDownloader](struct option* opts, int optChar, int optI) -> int {
            return fDownloader.callOptions(opts, optChar, optI);
    };

    if(argc > 2)
    {
        endwin();
        help();
    }
    else
    {
        if(parseArgs(argc, argv, longOpts, callback) == -2)
        {
            endwin();
            help();
        }
    }

    /*End curses*/
    endwin();
    return 0;
}
