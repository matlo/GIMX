/*
 * parseArgs.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */

#include "include/parseArgs.h"

int parseArgs(int argc, char* argv[], struct option* longOptions, std::function<int (struct option*, int, int)> optionsCaller)
{
	int optIndex = 0;
	int optChar;
    
    auto getOpts = [&] () -> int {
        return getopt_long(argc, argv, "ach", longOptions, &optIndex);
    };

    optChar = getOpts();
    if(optChar == -1)
        return -1;
    
    while(true)
	{
        if(optChar == -1) //End of list
            break;

		if(optionsCaller(longOptions, optChar, optIndex) == -1) //Help or invalid args
            return -1;
        
        optChar = getOpts();
	}
    return 0;
}
