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

    do
    {
        optChar = getopt_long(argc, argv, "ach", longOptions, &optIndex);

        optChar = optionsCaller(longOptions, optChar, optIndex);
    } while(optChar > 1);

    return (optChar == -2 ? -2 : 0);
}
