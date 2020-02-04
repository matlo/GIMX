/*
 * parseArgs.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 8 Aug. 2018
 */

#ifndef PARSEARGS_H
#define PARSEARGS_H

#include <getopt.h>


template <typename Call>
int parseArgs(int argc, char* argv[], struct option* longOptions, Call optionsCaller);

#include "../parseArgs.tpp"

#endif //PARSEARGS_H
