/*
 * fileOps.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 17 Sep. 2018
 */


#include "fileOps.h"


int getUserConfigDir(std::string& dir)
{
#ifndef  WIN32
    char* path = getenv("HOME");
    if(path == NULL)
        return 1;
    
#else
    static char path[MAX_PATH];
    if(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))
        return 1;
#endif //WIN32

    //TODO change forward slashes to back slashes for native windows path
    dir = path;
    return 0;
}

bool fileExists(std::string fName)
{
    struct stat buffer;
    return (stat (fName.c_str(), &buffer) == 0);
}
