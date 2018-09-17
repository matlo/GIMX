/*
 * misc.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 17 Sep. 2018
 */


#include "misc.h"


int getConfigDir(std::string& dir)
{
#ifndef WIN32
    dir = "/etc"
#else
    static char path[MAX_PATH];
    if(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))
        return 1;

    //TODO change forward slashes to back slashes for native windows path
    dir = path;
    return 0;
#endif
}

bool fileExists(std::string fName)
{
    struct stat buffer;
    return (stat (fName.c_str(), &buffer) == 0);
}