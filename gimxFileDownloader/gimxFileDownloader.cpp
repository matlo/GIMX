/*
 * gimxFileDownloader.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */

#include "include/gFD.h"

using namespace std::placeholders;

int main(int argc, char* argv[])
{
    /*Start curses*/
    initscr();
    cbreak();
    keypad(stdscr, true);

	/*Handle command line options*/
	struct option longOpts [] =
	{
		//These options don’t set a flag. We distinguish them by their indices.
		{"config", no_argument, 0, 'c'},
		{"autoconfig", no_argument, 0, 'a'},
		{0, 0, 0, 0}
	};

	FileDownloader fDownloader;
	auto callback = [&fDownloader](struct option* opts, int optChar, int optI) -> int {
		return fDownloader.callOptions(opts, optChar, optI);
	};
	parseArgs(argc, argv, longOpts, callback);

	/*End curses*/
	endwin();
	return 0;
}
