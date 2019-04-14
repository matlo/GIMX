/*
 * fileOps.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 17 Sep. 2018
 */


#ifndef   FILEOPS_H
#define   FILEOPS_H

#include <sys/types.h> //for stat()
#include <sys/stat.h> //for stat()
#include <string>

#ifndef  WIN32

#include <stdlib.h> //getenv => to get the userDir
#else

#include <shlobj.h> //SHGetFolderPath => to get the userDir
#endif //WIN32

#include "../../directories.h"


int getUserConfigDir(std::string& dir);

bool fileExists(std::string fName);

#endif //FILEOPS_H
