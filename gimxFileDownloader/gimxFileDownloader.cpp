/*
 * gimxFileDownloader.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */
#include <memory> //for smart pointers

#include <string>

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>

#include "parseArgs.h"
#include "cursesIO.h"
#include "configDownload.h"

//Callback functionality for download progress
int progress_callback_configupdater_terminal(void *clientp, configupdater::ConfigUpdaterStatus status, double progress, double total);

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
        //Commented until this feature begins development
    //  configDl = std::unique_ptr<AutoConfigDownload>(new AutoConfigDownload);
    //  dynamic_cast<AutoConfigDownload*>(configDl.get());
    }

    int callOptions(struct option* longOptions, int optionCharacter, int optionIndex)
    {
        int result = 0;
        switch(optionCharacter)
        {
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
                break;
            case '?':
                break;
            default:
                printw("?? getopt returned character code 0%o ??\n", optionCharacter);
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
    keypad(stdscr, true);

        /*Handle command line options*/
        struct option longOpts [] =
        {
                //These options don’t set a flag. We distinguish them by their indices.
                {"config", no_argument, 0, 'c'},
                {"autoconfig", no_argument, 0, 'a'},
                {0, 0, 0, 0}
        };

        FileDownloader fDownloader;
        auto callback = [&fDownloader](struct option* opts, int optChar, int optI) -> int {
                return fDownloader.callOptions(opts, optChar, optI);
        };
        parseArgs(argc, argv, longOpts, callback);

        /*End curses*/
        endwin();
        return 0;
}