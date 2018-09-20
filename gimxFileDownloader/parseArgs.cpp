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

	int result = 0;

	while (true)
	{
		//'getopt_long()' stores the option index here.
		optChar = getopt_long(argc, argv, "ca", longOptions, &optIndex);

		//End of the options
		if(optChar == -1)
			break;

		optionsCaller(longOptions, optChar, optIndex);
	}

	return result;
}
