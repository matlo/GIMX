/*
 * misc.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 17 Sep. 2018
 */


#ifndef   MISC_H
#define   MISC_H

#include <sys/types.h> //for stat()
#include <sys/stat.h> //for stat()
#ifdef WIN32
#include <shlobj.h> //SHGetFolderPath => to get the userDir
#endif

#include <string>

#include "../../directories.h"


int getConfigDir(std::string& dir);

bool fileExists(std::string fName);

#endif //MISC_H