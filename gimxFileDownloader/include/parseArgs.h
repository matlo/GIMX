/*
 * parseArgs.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */

#ifndef PARSEARGS_H
#define PARSEARGS_H

//#include <unistd.h>
#include <getopt.h>

#include <functional>


int parseArgs(int argc, char* argv[], struct option* longOptions, std::function<int (struct option*, int, int)> optionsCaller);


#endif //PARSEARGS_H
