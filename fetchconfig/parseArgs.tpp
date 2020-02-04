/*
 * parseArgs.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Oct. 2019
 */

#include "include/parseArgs.h"

template <typename Call>
int parseArgs(int argc, char* argv[], struct option* longOptions, Call optionsCaller)
{
    int optIndex = 0;
    int optChar;

    do
    {
        optChar = getopt_long(argc, argv, "cah", longOptions, &optIndex);

        optChar = optionsCaller(optChar);
    } while(optIndex > argc);
    return optChar;
}
