/*
 * gFD.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 12 Aug. 2018
 */

#include "include/gFD.h"

int FileDownloader::callOptions(struct option* longOptions, int optionCharacter, int optionIndex)
{
	int result = 0;
	switch(optionCharacter)
	{
		case 0:
			//If this option set a flag, do nothing else now.
			if(longOptions[optionIndex].flag != 0)
				break;
			printw("option %s", longOptions[optionIndex].name);
			if(optarg)
				printw(" with arg %s", optarg);
			printw("\n");
			break;
		//Manual configuration download
		case 'c':
			printw("option -c with value `%s'\n", optarg);
			refresh();
            manualConfig();
			break;
		//Automatic configuration download
		case 'a':
			printw("option -a with value `%s'\n", optarg);
            autoConfig();
			break;
		case '?':
			break;
		default:
			printw("?? getopt returned character code 0%o ??\n", optionCharacter);
	}

	return result;
}

void FileDownloader::manualConfig()
{
	configDl = std::unique_ptr<ConfigDownload>(new ManualConfigDownload);
	static_cast<ManualConfigDownload*>(configDl.get());

	configDl->chooseConfig();
}

void FileDownloader::autoConfig()
{
	//Commented until class complete
//	configDl = std::unique_ptr<AutoConfigDownload>(new AutoConfigDownload);
//	dynamic_cast<AutoConfigDownload*>(configDl.get());
}