/*
 * gFD.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#ifndef  GFD_H
#define  GFD_H

#include <memory>
#include <functional>

#include <gimxconfigupdater/configupdater.h>
#include <gimxupdater/Updater.h>

#include "parseArgs.h"
#include "configDownload.h"
#include "cursesIO.h"


//Callback functionality for download progress
int progress_callback_configupdater_terminal(void *clientp, configupdater::ConfigUpdaterStatus status, double progress, double total);

class FileDownloader
{
public:
    void manualConfig();
    void autoConfig();

    int callOptions(struct option* longOptions, int optionCharacter, int optionIndex);

private:
    std::unique_ptr<ConfigDownload> configDl;
};


#endif //GFD_H