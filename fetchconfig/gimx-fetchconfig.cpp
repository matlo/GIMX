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
    void exec()
    {
        if(doConfig)
            manualConfig();
        else if(doAutoConfig)
            autoConfig();
    }

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

    int callOptions(int optionCharacter)
    {
        int result = -1;
        switch(optionCharacter)
        {
            case -1:
                //End of list
                break;
            //Manual configuration download
            case 'c':
                //manualConfig();
                doConfig = true;
                break;
            //Automatic configuration download
            case 'a':
                //autoConfig();
                doAutoConfig = true;
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
    bool doConfig     = false;
    bool doAutoConfig = false;

    std::unique_ptr<ConfigDownload> configDl;
};


int main(int argc, char* argv[])
{
    if(argc == 1)
        help();
    else
    {
        /*Handle command line options*/
        struct option longOpts [] =
        {
                //These options don’t set a flag. We distinguish them by their indices.
                { "config",     no_argument, 0, 'c' },
                { "autoconfig", no_argument, 0, 'a' },
                { "help",       no_argument, 0, 'h' },
                { 0, 0, 0, 0 }
        };

        FileDownloader fDownloader;
        auto callback = [&fDownloader](int optChar) -> int {
                return fDownloader.callOptions(optChar);
        };

        if(parseArgs(argc, argv, longOpts, callback) == -2)
        {
            help();
            return 0;
        }
        else if(optind < argc)
        {
            std::cout << "Invalid input: " << argv[optind++] << std::endl;
            help();
            return 0;
        }
        else
        {
            initscr();
            cbreak();
            curs_set(0);
            noecho();
            keypad(stdscr, true);

            fDownloader.exec();

            endwin();
        }
    }

    return 0;
}
